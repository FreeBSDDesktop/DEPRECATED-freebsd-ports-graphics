--- src/loader/loader.c.orig	2016-11-10 17:05:17.000000000 -0500
+++ src/loader/loader.c	2016-12-17 20:09:34.946729000 -0500
@@ -33,6 +33,16 @@
 #include <stdio.h>
 #include <stdbool.h>
 #include <string.h>
+
+
+#include <assert.h>
+#include <dlfcn.h>
+#include <unistd.h>
+#include <stdlib.h>
+
+#undef HAVE_LIBDRM
+#define HAVE_LIBUDEV 1
+
 #ifdef MAJOR_IN_MKDEV
 #include <sys/mkdev.h>
 #endif
@@ -82,6 +92,231 @@
    return fd;
 }
 
+#ifdef HAVE_LIBUDEV
+#include <libudev.h>
+
+static void *udev_handle = NULL;
+
+static void *
+udev_dlopen_handle(void)
+{
+   char name[80];
+   unsigned flags = RTLD_NOLOAD | RTLD_LOCAL | RTLD_LAZY;
+   int version;
+
+   /* libudev.so.1 changed the return types of the two unref functions
+    * from voids to pointers.  We don't use those return values, and the
+    * only ABI I've heard that cares about this kind of change (calling
+    * a function with a void * return that actually only returns void)
+    * might be ia64.
+    */
+
+   /* First try opening an already linked libudev, then try loading one */
+   do {
+      for (version = 1; version >= 0; version--) {
+         snprintf(name, sizeof(name), "libudev.so.%d", version);
+         udev_handle = dlopen(name, flags);
+         if (udev_handle)
+            return udev_handle;
+      }
+
+      if ((flags & RTLD_NOLOAD) == 0)
+         break;
+
+      flags &= ~RTLD_NOLOAD;
+   } while (1);
+
+   log_(_LOADER_WARNING,
+        "Couldn't dlopen libudev.so.1 or "
+        "libudev.so.0, driver detection may be broken.\n");
+   return NULL;
+}
+
+static int dlsym_failed = 0;
+
+static void *
+checked_dlsym(void *dlopen_handle, const char *name)
+{
+   void *result = dlsym(dlopen_handle, name);
+   if (!result)
+      dlsym_failed = 1;
+   return result;
+}
+
+#define UDEV_SYMBOL(ret, name, args) \
+   ret (*name) args = checked_dlsym(udev_dlopen_handle(), #name);
+
+
+static inline struct udev_device *
+udev_device_new_from_fd(struct udev *udev, int fd)
+{
+   struct udev_device *device;
+   struct stat buf;
+   UDEV_SYMBOL(struct udev_device *, udev_device_new_from_devnum,
+               (struct udev *udev, char type, dev_t devnum));
+
+   if (dlsym_failed)
+      return NULL;
+
+   if (fstat(fd, &buf) < 0) {
+      log_(_LOADER_WARNING, "MESA-LOADER: failed to stat fd %d\n", fd);
+      return NULL;
+   }
+
+   device = udev_device_new_from_devnum(udev, 'c', buf.st_rdev);
+   if (device == NULL) {
+      log_(_LOADER_WARNING,
+              "MESA-LOADER: could not create udev device for fd %d\n", fd);
+      return NULL;
+   }
+
+   return device;
+}
+
+static int
+libudev_get_pci_id_for_fd(int fd, int *vendor_id, int *chip_id)
+{
+   struct udev *udev = NULL;
+   struct udev_device *device = NULL, *parent;
+   const char *pci_id;
+   UDEV_SYMBOL(struct udev *, udev_new, (void));
+   UDEV_SYMBOL(struct udev_device *, udev_device_get_parent,
+               (struct udev_device *));
+   UDEV_SYMBOL(const char *, udev_device_get_property_value,
+               (struct udev_device *, const char *));
+   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
+               (struct udev_device *));
+   UDEV_SYMBOL(struct udev *, udev_unref, (struct udev *));
+
+   *chip_id = -1;
+
+   if (dlsym_failed)
+      return 0;
+
+   udev = udev_new();
+   device = udev_device_new_from_fd(udev, fd);
+   if (!device)
+      goto out;
+
+   parent = udev_device_get_parent(device);
+   if (parent == NULL) {
+      log_(_LOADER_WARNING, "MESA-LOADER: could not get parent device\n");
+      goto out;
+   }
+
+   pci_id = udev_device_get_property_value(parent, "PCI_ID");
+   if (pci_id == NULL) {
+      log_(_LOADER_INFO, "MESA-LOADER: no PCI ID\n");
+      *chip_id = -1;
+      goto out;
+   } else if (sscanf(pci_id, "%x:%x", vendor_id, chip_id) != 2) {
+      log_(_LOADER_WARNING, "MESA-LOADER: malformed PCI ID\n");
+      *chip_id = -1;
+      goto out;
+   }
+
+out:
+   if (device)
+      udev_device_unref(device);
+   if (udev)
+      udev_unref(udev);
+
+   return (*chip_id >= 0);
+}
+
+static char *
+get_render_node_from_id_path_tag(struct udev *udev,
+                                 char *id_path_tag,
+                                 char another_tag)
+{
+   struct udev_device *device;
+   struct udev_enumerate *e;
+   struct udev_list_entry *entry;
+   const char *path, *id_path_tag_tmp;
+   char *path_res;
+   char found = 0;
+   UDEV_SYMBOL(struct udev_enumerate *, udev_enumerate_new,
+               (struct udev *));
+   UDEV_SYMBOL(int, udev_enumerate_add_match_subsystem,
+               (struct udev_enumerate *, const char *));
+   UDEV_SYMBOL(int, udev_enumerate_add_match_sysname,
+               (struct udev_enumerate *, const char *));
+   UDEV_SYMBOL(int, udev_enumerate_scan_devices,
+               (struct udev_enumerate *));
+   UDEV_SYMBOL(struct udev_list_entry *, udev_enumerate_get_list_entry,
+               (struct udev_enumerate *));
+   UDEV_SYMBOL(void, udev_enumerate_unref,
+               (struct udev_enumerate *));
+   UDEV_SYMBOL(struct udev_list_entry *, udev_list_entry_get_next,
+               (struct udev_list_entry *));
+   UDEV_SYMBOL(const char *, udev_list_entry_get_name,
+               (struct udev_list_entry *));
+   UDEV_SYMBOL(struct udev_device *, udev_device_new_from_syspath,
+               (struct udev *, const char *));
+   UDEV_SYMBOL(const char *, udev_device_get_property_value,
+               (struct udev_device *, const char *));
+   UDEV_SYMBOL(const char *, udev_device_get_devnode,
+               (struct udev_device *));
+   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
+               (struct udev_device *));
+
+   e = udev_enumerate_new(udev);
+   udev_enumerate_add_match_subsystem(e, "drm");
+   udev_enumerate_add_match_sysname(e, "render*");
+
+   udev_enumerate_scan_devices(e);
+   udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(e)) {
+      path = udev_list_entry_get_name(entry);
+      device = udev_device_new_from_syspath(udev, path);
+      if (!device)
+         continue;
+      id_path_tag_tmp = udev_device_get_property_value(device, "ID_PATH_TAG");
+      if (id_path_tag_tmp) {
+         if ((!another_tag && !strcmp(id_path_tag, id_path_tag_tmp)) ||
+             (another_tag && strcmp(id_path_tag, id_path_tag_tmp))) {
+            found = 1;
+            break;
+         }
+      }
+      udev_device_unref(device);
+   }
+
+   udev_enumerate_unref(e);
+
+   if (found) {
+      path_res = strdup(udev_device_get_devnode(device));
+      udev_device_unref(device);
+      return path_res;
+   }
+   return NULL;
+}
+
+static char *
+get_id_path_tag_from_fd(struct udev *udev, int fd)
+{
+   struct udev_device *device;
+   const char *id_path_tag_tmp;
+   char *id_path_tag;
+   UDEV_SYMBOL(const char *, udev_device_get_property_value,
+               (struct udev_device *, const char *));
+   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
+               (struct udev_device *));
+
+   device = udev_device_new_from_fd(udev, fd);
+   if (!device)
+      return NULL;
+
+   id_path_tag_tmp = udev_device_get_property_value(device, "ID_PATH_TAG");
+   if (!id_path_tag_tmp)
+      return NULL;
+
+   id_path_tag = strdup(id_path_tag_tmp);
+
+   udev_device_unref(device);
+   return id_path_tag;
+}
+#endif
+
 #if defined(HAVE_LIBDRM)
 #ifdef USE_DRICONF
 static const char __driConfigOptionsLoader[] =
@@ -304,9 +539,48 @@
    if (drm_get_pci_id_for_fd(fd, vendor_id, chip_id))
       return 1;
 #endif
+#if HAVE_LIBUDEV
+   if (libudev_get_pci_id_for_fd(fd, vendor_id, chip_id))
+      return 1;
+#endif   
    return 0;
 }
 
+#ifdef HAVE_LIBUDEV
+static char *
+libudev_get_device_name_for_fd(int fd)
+{
+   char *device_name = NULL;
+   struct udev *udev;
+   struct udev_device *device;
+   const char *const_device_name;
+   UDEV_SYMBOL(struct udev *, udev_new, (void));
+   UDEV_SYMBOL(const char *, udev_device_get_devnode,
+               (struct udev_device *));
+   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
+               (struct udev_device *));
+   UDEV_SYMBOL(struct udev *, udev_unref, (struct udev *));
+
+   if (dlsym_failed)
+      return NULL;
+
+   udev = udev_new();
+   device = udev_device_new_from_fd(udev, fd);
+   if (device == NULL)
+      return NULL;
+
+   const_device_name = udev_device_get_devnode(device);
+   if (!const_device_name)
+      goto out;
+   device_name = strdup(const_device_name);
+
+out:
+   udev_device_unref(device);
+   udev_unref(udev);
+   return device_name;
+}
+#endif
+
 
 #if defined(HAVE_LIBDRM)
 static char *
@@ -332,6 +606,10 @@
 {
    char *result = NULL;
 
+#if HAVE_LIBUDEV
+   if ((result = libudev_get_device_name_for_fd(fd)))
+      return result;
+#endif
 #if HAVE_LIBDRM
    if ((result = drm_get_device_name_for_fd(fd)))
       return result;
@@ -386,7 +664,7 @@
 
 out:
    log_(driver ? _LOADER_DEBUG : _LOADER_WARNING,
-         "pci id for fd %d: %04x:%04x, driver %s\n",
+	"UGH....pci id for fd %d: %04x:%04x, driver %s\n",
          fd, vendor_id, chip_id, driver);
    return driver;
 }
