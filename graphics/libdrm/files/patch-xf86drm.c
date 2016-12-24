--- xf86drm.c.orig	2016-09-08 21:04:36.283417000 +0200
+++ xf86drm.c	2016-09-08 21:10:16.495306000 +0200
@@ -62,6 +62,10 @@
 #endif
 #include <math.h>
 
+#ifdef HAVE_LIBDEVQ
+#include "libdevq.h"
+#endif
+
 /* Not all systems have MAP_FAILED defined */
 #ifndef MAP_FAILED
 #define MAP_FAILED ((void *)-1)
@@ -82,8 +86,12 @@
 #define DRM_RENDER_MINOR_NAME   "renderD"
 #endif
 
-#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__DragonFly__)
-#define DRM_MAJOR 145
+#if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
+#define DRM_MAJOR 0
+#endif
+
+#if defined(__DragonFly__)
+#define DRM_MAJOR 65 /* was 154 XXX needs checking */
 #endif
 
 #ifdef __NetBSD__
@@ -532,6 +540,7 @@ static int drmGetMinorType(int minor)
     }
 }
 
+#if !defined(__FreeBSD__) && !defined(__DragonFly__)
 static const char *drmGetMinorName(int type)
 {
     switch (type) {
@@ -545,6 +554,7 @@ static const char *drmGetMinorName(int t
         return NULL;
     }
 }
+#endif
 
 /**
  * Open the device by bus ID.
@@ -2817,6 +2827,15 @@ static char *drmGetMinorNameForFD(int fd
 
 out_close_dir:
     closedir(sysdir);
+#elif defined(__FreeBSD__) || defined(__DragonFly__)
+    struct stat buf;
+    char name[64];
+
+    fstat(fd, &buf);
+    snprintf(name, sizeof(name), "/dev/%s",
+             devname(buf.st_rdev, S_IFCHR));
+
+    return strdup(name);
 #else
 #warning "Missing implementation of drmGetMinorNameForFD"
 #endif
@@ -2854,12 +2873,19 @@ static int drmParseSubsystemType(int maj
         return DRM_BUS_PCI;
 
     return -EINVAL;
+#elif defined(__FreeBSD__) || defined(__DragonFly__)
+    /* XXX: Don't know how to get the subsystem type, hardcode for now.
+     * The code following the call to this function needs depends on
+     * information provided by the /pci subsystem on linux. No replacement
+     * found yet for FreeBSD. */
+    return DRM_BUS_PCI;
 #else
 #warning "Missing implementation of drmParseSubsystemType"
     return -EINVAL;
 #endif
 }
 
+#if !defined(__FreeBSD__) && !defined(__DragonFly__)
 static int drmParsePciBusInfo(int maj, int min, drmPciBusInfoPtr info)
 {
 #ifdef __linux__
@@ -2901,6 +2927,75 @@ static int drmParsePciBusInfo(int maj, i
     return -EINVAL;
 #endif
 }
+#else
+
+/*
+ * XXX temporary workaround, because FreeBSD doesn't provide 
+ * pcibus device sysctl trees for renderD and controlD nodes (yet)
+ */
+static char *
+drmBSDDeviceNameHack(const char *path)
+{
+    int i, size;
+    long val;
+    char *hacked_path;
+
+  for (i = 0; i < strlen(path); i++)
+  {
+    val = strtol(&path[i], NULL, 10);
+   
+    if (val != 0)
+      break;
+  }
+
+  if (val >= 64 && val < 128) // controlD node
+  {
+    val = val - 64;
+  }else if (val >= 128 && val < 256) // renderD node
+  {
+    val = val - 128;
+  }
+
+  size = sizeof(DRM_DIR_NAME) + sizeof("/card") + sizeof(val);
+
+  if ((hacked_path = malloc(size)) == NULL)
+    return NULL;
+
+  snprintf(hacked_path, size, DRM_DIR_NAME "/card%li", val);
+
+  return hacked_path;
+}
+
+static int
+drmParsePciBusInfoBSD(const char *path, drmPciBusInfoPtr info)
+{
+    int fd, ret;
+    int domain = 0, bus = 0, slot = 0, function = 0;
+
+    char hacked_path[PATH_MAX + 1];
+
+    snprintf(hacked_path, PATH_MAX, "%s", drmBSDDeviceNameHack(path));
+
+    fd = open(hacked_path, O_RDONLY);
+
+    if (fd < 0)
+      return -errno;
+
+    ret = devq_device_get_pcibusaddr(fd, &domain, &bus, &slot, &function);
+
+    if (ret < 0)
+      return -1;
+
+    info->domain = (uint16_t) domain;
+    info->bus = (uint8_t) bus;
+    info->dev = (uint8_t) slot;
+    info->func = (uint8_t) function;
+
+    close(fd);
+
+    return 0;
+}
+#endif
 
 static int drmCompareBusInfo(drmDevicePtr a, drmDevicePtr b)
 {
@@ -2971,6 +3066,32 @@ static int drmParsePciDeviceInfo(const c
     device->subdevice_id = config[46] | (config[47] << 8);
 
     return 0;
+#elif defined(__FreeBSD__) || defined(__DragonFly__)
+
+    int fd, vendor_id = 0, device_id = 0, subvendor_id = 0, 
+	subdevice_id = 0, revision_id = 0;
+    char path[PATH_MAX + 1];
+    char hacked_path[PATH_MAX + 1];
+
+    snprintf(path, PATH_MAX, DRM_DIR_NAME "/%s", d_name);
+
+    snprintf(hacked_path, PATH_MAX, "%s", drmBSDDeviceNameHack(path));
+
+    fd = open(hacked_path, O_RDONLY);
+
+    if (fd < 0)
+        return -errno;
+
+    devq_device_get_pciid_full_from_fd(fd, &vendor_id, &device_id,
+	&subvendor_id, &subdevice_id, &revision_id);
+
+    device->vendor_id = (uint16_t) vendor_id;
+    device->device_id = (uint16_t) device_id;
+    device->subvendor_id = (uint16_t) subvendor_id;
+    device->subdevice_id = (uint16_t) subdevice_id;
+    device->revision_id = (uint8_t) revision_id;
+
+    return 0;
 #else
 #warning "Missing implementation of drmParsePciDeviceInfo"
     return -EINVAL;
@@ -3030,7 +3151,12 @@ static int drmProcessPciDevice(drmDevice
 
     (*device)->businfo.pci = (drmPciBusInfoPtr)addr;
 
+#if defined(__FreeBSD__) || defined(__DragonFly__)
+    ret = drmParsePciBusInfoBSD(node, (*device)->businfo.pci);
+#else
     ret = drmParsePciBusInfo(maj, min, (*device)->businfo.pci);
+#endif
+
     if (ret)
         goto free_device;
 
