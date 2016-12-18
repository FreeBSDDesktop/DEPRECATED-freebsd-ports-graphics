/*
 * Copyright (C) 2013 Rob Clark <robclark@freedesktop.org>
 * Copyright (C) 2014-2016 Emil Velikov <emil.l.velikov@gmail.com>
 * Copyright (C) 2016 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Authors:
 *    Rob Clark <robclark@freedesktop.org>
 */

#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#include <assert.h>
#include <dlfcn.h>
#include <unistd.h>
#include <stdlib.h>

#undef HAVE_LIBDRM
#define HAVE_LIBUDEV 1

#ifdef MAJOR_IN_MKDEV
#include <sys/mkdev.h>
#endif
#ifdef MAJOR_IN_SYSMACROS
#include <sys/sysmacros.h>
#endif
#include "loader.h"

#ifdef HAVE_LIBDRM
#include <stdlib.h>
#include <unistd.h>
#include <xf86drm.h>
#ifdef USE_DRICONF
#include "xmlconfig.h"
#include "xmlpool.h"
#endif
#endif

#define __IS_LOADER
#include "pci_id_driver_map.h"

static void default_logger(int level, const char *fmt, ...)
{
   if (level <= _LOADER_WARNING) {
      va_list args;
      va_start(args, fmt);
      vfprintf(stderr, fmt, args);
      va_end(args);
   }
}

static void (*log_)(int level, const char *fmt, ...) = default_logger;

int
loader_open_device(const char *device_name)
{
   int fd;
#ifdef O_CLOEXEC
   fd = open(device_name, O_RDWR | O_CLOEXEC);
   if (fd == -1 && errno == EINVAL)
#endif
   {
      fd = open(device_name, O_RDWR);
      if (fd != -1)
         fcntl(fd, F_SETFD, fcntl(fd, F_GETFD) | FD_CLOEXEC);
   }
   return fd;
}

#ifdef HAVE_LIBUDEV
#include <libudev.h>

static void *udev_handle = NULL;

static void *
udev_dlopen_handle(void)
{
   char name[80];
   unsigned flags = RTLD_NOLOAD | RTLD_LOCAL | RTLD_LAZY;
   int version;

   /* libudev.so.1 changed the return types of the two unref functions
    * from voids to pointers.  We don't use those return values, and the
    * only ABI I've heard that cares about this kind of change (calling
    * a function with a void * return that actually only returns void)
    * might be ia64.
    */

   /* First try opening an already linked libudev, then try loading one */
   do {
      for (version = 1; version >= 0; version--) {
         snprintf(name, sizeof(name), "libudev.so.%d", version);
         udev_handle = dlopen(name, flags);
         if (udev_handle)
            return udev_handle;
      }

      if ((flags & RTLD_NOLOAD) == 0)
         break;

      flags &= ~RTLD_NOLOAD;
   } while (1);

   log_(_LOADER_WARNING,
        "Couldn't dlopen libudev.so.1 or "
        "libudev.so.0, driver detection may be broken.\n");
   return NULL;
}

static int dlsym_failed = 0;

static void *
checked_dlsym(void *dlopen_handle, const char *name)
{
   void *result = dlsym(dlopen_handle, name);
   if (!result)
      dlsym_failed = 1;
   return result;
}

#define UDEV_SYMBOL(ret, name, args) \
   ret (*name) args = checked_dlsym(udev_dlopen_handle(), #name);


static inline struct udev_device *
udev_device_new_from_fd(struct udev *udev, int fd)
{
   struct udev_device *device;
   struct stat buf;
   UDEV_SYMBOL(struct udev_device *, udev_device_new_from_devnum,
               (struct udev *udev, char type, dev_t devnum));

   if (dlsym_failed)
      return NULL;

   if (fstat(fd, &buf) < 0) {
      log_(_LOADER_WARNING, "MESA-LOADER: failed to stat fd %d\n", fd);
      return NULL;
   }

   device = udev_device_new_from_devnum(udev, 'c', buf.st_rdev);
   if (device == NULL) {
      log_(_LOADER_WARNING,
              "MESA-LOADER: could not create udev device for fd %d\n", fd);
      return NULL;
   }

   return device;
}

static int
libudev_get_pci_id_for_fd(int fd, int *vendor_id, int *chip_id)
{
   struct udev *udev = NULL;
   struct udev_device *device = NULL, *parent;
   const char *pci_id;
   UDEV_SYMBOL(struct udev *, udev_new, (void));
   UDEV_SYMBOL(struct udev_device *, udev_device_get_parent,
               (struct udev_device *));
   UDEV_SYMBOL(const char *, udev_device_get_property_value,
               (struct udev_device *, const char *));
   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
               (struct udev_device *));
   UDEV_SYMBOL(struct udev *, udev_unref, (struct udev *));

   *chip_id = -1;

   if (dlsym_failed)
      return 0;

   udev = udev_new();
   device = udev_device_new_from_fd(udev, fd);
   if (!device)
      goto out;

   parent = udev_device_get_parent(device);
   if (parent == NULL) {
      log_(_LOADER_WARNING, "MESA-LOADER: could not get parent device\n");
      goto out;
   }

   pci_id = udev_device_get_property_value(parent, "PCI_ID");
   if (pci_id == NULL) {
      log_(_LOADER_INFO, "MESA-LOADER: no PCI ID\n");
      *chip_id = -1;
      goto out;
   } else if (sscanf(pci_id, "%x:%x", vendor_id, chip_id) != 2) {
      log_(_LOADER_WARNING, "MESA-LOADER: malformed PCI ID\n");
      *chip_id = -1;
      goto out;
   }

out:
   if (device)
      udev_device_unref(device);
   if (udev)
      udev_unref(udev);

   return (*chip_id >= 0);
}

static char *
get_render_node_from_id_path_tag(struct udev *udev,
                                 char *id_path_tag,
                                 char another_tag)
{
   struct udev_device *device;
   struct udev_enumerate *e;
   struct udev_list_entry *entry;
   const char *path, *id_path_tag_tmp;
   char *path_res;
   char found = 0;
   UDEV_SYMBOL(struct udev_enumerate *, udev_enumerate_new,
               (struct udev *));
   UDEV_SYMBOL(int, udev_enumerate_add_match_subsystem,
               (struct udev_enumerate *, const char *));
   UDEV_SYMBOL(int, udev_enumerate_add_match_sysname,
               (struct udev_enumerate *, const char *));
   UDEV_SYMBOL(int, udev_enumerate_scan_devices,
               (struct udev_enumerate *));
   UDEV_SYMBOL(struct udev_list_entry *, udev_enumerate_get_list_entry,
               (struct udev_enumerate *));
   UDEV_SYMBOL(void, udev_enumerate_unref,
               (struct udev_enumerate *));
   UDEV_SYMBOL(struct udev_list_entry *, udev_list_entry_get_next,
               (struct udev_list_entry *));
   UDEV_SYMBOL(const char *, udev_list_entry_get_name,
               (struct udev_list_entry *));
   UDEV_SYMBOL(struct udev_device *, udev_device_new_from_syspath,
               (struct udev *, const char *));
   UDEV_SYMBOL(const char *, udev_device_get_property_value,
               (struct udev_device *, const char *));
   UDEV_SYMBOL(const char *, udev_device_get_devnode,
               (struct udev_device *));
   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
               (struct udev_device *));

   e = udev_enumerate_new(udev);
   udev_enumerate_add_match_subsystem(e, "drm");
   udev_enumerate_add_match_sysname(e, "render*");

   udev_enumerate_scan_devices(e);
   udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(e)) {
      path = udev_list_entry_get_name(entry);
      device = udev_device_new_from_syspath(udev, path);
      if (!device)
         continue;
      id_path_tag_tmp = udev_device_get_property_value(device, "ID_PATH_TAG");
      if (id_path_tag_tmp) {
         if ((!another_tag && !strcmp(id_path_tag, id_path_tag_tmp)) ||
             (another_tag && strcmp(id_path_tag, id_path_tag_tmp))) {
            found = 1;
            break;
         }
      }
      udev_device_unref(device);
   }

   udev_enumerate_unref(e);

   if (found) {
      path_res = strdup(udev_device_get_devnode(device));
      udev_device_unref(device);
      return path_res;
   }
   return NULL;
}

static char *
get_id_path_tag_from_fd(struct udev *udev, int fd)
{
   struct udev_device *device;
   const char *id_path_tag_tmp;
   char *id_path_tag;
   UDEV_SYMBOL(const char *, udev_device_get_property_value,
               (struct udev_device *, const char *));
   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
               (struct udev_device *));

   device = udev_device_new_from_fd(udev, fd);
   if (!device)
      return NULL;

   id_path_tag_tmp = udev_device_get_property_value(device, "ID_PATH_TAG");
   if (!id_path_tag_tmp)
      return NULL;

   id_path_tag = strdup(id_path_tag_tmp);

   udev_device_unref(device);
   return id_path_tag;
}
#endif

#if defined(HAVE_LIBDRM)
#ifdef USE_DRICONF
static const char __driConfigOptionsLoader[] =
DRI_CONF_BEGIN
    DRI_CONF_SECTION_INITIALIZATION
        DRI_CONF_DEVICE_ID_PATH_TAG()
    DRI_CONF_SECTION_END
DRI_CONF_END;

static char *loader_get_dri_config_device_id(void)
{
   driOptionCache defaultInitOptions;
   driOptionCache userInitOptions;
   char *prime = NULL;

   driParseOptionInfo(&defaultInitOptions, __driConfigOptionsLoader);
   driParseConfigFiles(&userInitOptions, &defaultInitOptions, 0, "loader");
   if (driCheckOption(&userInitOptions, "device_id", DRI_STRING))
      prime = strdup(driQueryOptionstr(&userInitOptions, "device_id"));
   driDestroyOptionCache(&userInitOptions);
   driDestroyOptionInfo(&defaultInitOptions);

   return prime;
}
#endif

static char *drm_construct_id_path_tag(drmDevicePtr device)
{
/* Length of "pci-xxxx_xx_xx_x\0" */
#define PCI_ID_PATH_TAG_LENGTH 17
   char *tag = NULL;

   if (device->bustype == DRM_BUS_PCI) {
        tag = calloc(PCI_ID_PATH_TAG_LENGTH, sizeof(char));
        if (tag == NULL)
            return NULL;

        snprintf(tag, PCI_ID_PATH_TAG_LENGTH, "pci-%04x_%02x_%02x_%1u",
                 device->businfo.pci->domain, device->businfo.pci->bus,
                 device->businfo.pci->dev, device->businfo.pci->func);
   }
   return tag;
}

static bool drm_device_matches_tag(drmDevicePtr device, const char *prime_tag)
{
   char *tag = drm_construct_id_path_tag(device);
   int ret;

   if (tag == NULL)
      return false;

   ret = strcmp(tag, prime_tag);

   free(tag);
   return ret == 0;
}

static char *drm_get_id_path_tag_for_fd(int fd)
{
   drmDevicePtr device;
   char *tag;

   if (drmGetDevice(fd, &device) != 0)
       return NULL;

   tag = drm_construct_id_path_tag(device);
   drmFreeDevice(&device);
   return tag;
}

int loader_get_user_preferred_fd(int default_fd, int *different_device)
{
/* Arbitrary "maximum" value of drm devices. */
#define MAX_DRM_DEVICES 32
   const char *dri_prime = getenv("DRI_PRIME");
   char *default_tag, *prime = NULL;
   drmDevicePtr devices[MAX_DRM_DEVICES];
   int i, num_devices, fd;
   bool found = false;

   if (dri_prime)
      prime = strdup(dri_prime);
#ifdef USE_DRICONF
   else
      prime = loader_get_dri_config_device_id();
#endif

   if (prime == NULL) {
      *different_device = 0;
      return default_fd;
   }

   default_tag = drm_get_id_path_tag_for_fd(default_fd);
   if (default_tag == NULL)
      goto err;

   num_devices = drmGetDevices(devices, MAX_DRM_DEVICES);
   if (num_devices < 0)
      goto err;

   /* two format are supported:
    * "1": choose any other card than the card used by default.
    * id_path_tag: (for example "pci-0000_02_00_0") choose the card
    * with this id_path_tag.
    */
   if (!strcmp(prime,"1")) {
      /* Hmm... detection for 2-7 seems to be broken. Oh well ...
       * Pick the first render device that is not our own.
       */
      for (i = 0; i < num_devices; i++) {
         if (devices[i]->available_nodes & 1 << DRM_NODE_RENDER &&
             !drm_device_matches_tag(devices[i], default_tag)) {

            found = true;
            break;
         }
      }
   } else {
      for (i = 0; i < num_devices; i++) {
         if (devices[i]->available_nodes & 1 << DRM_NODE_RENDER &&
            drm_device_matches_tag(devices[i], prime)) {

            found = true;
            break;
         }
      }
   }

   if (!found) {
      drmFreeDevices(devices, num_devices);
      goto err;
   }

   fd = loader_open_device(devices[i]->nodes[DRM_NODE_RENDER]);
   drmFreeDevices(devices, num_devices);
   if (fd < 0)
      goto err;

   close(default_fd);

   *different_device = !!strcmp(default_tag, prime);

   free(default_tag);
   free(prime);
   return fd;

 err:
   *different_device = 0;

   free(default_tag);
   free(prime);
   return default_fd;
}
#else
int loader_get_user_preferred_fd(int default_fd, int *different_device)
{
   *different_device = 0;
   return default_fd;
}
#endif

#if defined(HAVE_LIBDRM)
static int
dev_node_from_fd(int fd, unsigned int *maj, unsigned int *min)
{
   struct stat buf;

   if (fstat(fd, &buf) < 0) {
      log_(_LOADER_WARNING, "MESA-LOADER: failed to stat fd %d\n", fd);
      return -1;
   }

   if (!S_ISCHR(buf.st_mode)) {
      log_(_LOADER_WARNING, "MESA-LOADER: fd %d not a character device\n", fd);
      return -1;
   }

   *maj = major(buf.st_rdev);
   *min = minor(buf.st_rdev);

   return 0;
}
#endif

#if defined(HAVE_LIBDRM)

static int
drm_get_pci_id_for_fd(int fd, int *vendor_id, int *chip_id)
{
   drmDevicePtr device;
   int ret;

   if (drmGetDevice(fd, &device) == 0) {
      if (device->bustype == DRM_BUS_PCI) {
         *vendor_id = device->deviceinfo.pci->vendor_id;
         *chip_id = device->deviceinfo.pci->device_id;
         ret = 1;
      }
      else {
         log_(_LOADER_WARNING, "MESA-LOADER: device is not located on the PCI bus\n");
         ret = 0;
      }
      drmFreeDevice(&device);
   }
   else {
      log_(_LOADER_WARNING, "MESA-LOADER: failed to retrieve device information\n");
      ret = 0;
   }

   return ret;
}
#endif


int
loader_get_pci_id_for_fd(int fd, int *vendor_id, int *chip_id)
{
#if HAVE_LIBDRM
   if (drm_get_pci_id_for_fd(fd, vendor_id, chip_id))
      return 1;
#endif
#if HAVE_LIBUDEV
   if (libudev_get_pci_id_for_fd(fd, vendor_id, chip_id))
      return 1;
#endif   
   return 0;
}

#ifdef HAVE_LIBUDEV
static char *
libudev_get_device_name_for_fd(int fd)
{
   char *device_name = NULL;
   struct udev *udev;
   struct udev_device *device;
   const char *const_device_name;
   UDEV_SYMBOL(struct udev *, udev_new, (void));
   UDEV_SYMBOL(const char *, udev_device_get_devnode,
               (struct udev_device *));
   UDEV_SYMBOL(struct udev_device *, udev_device_unref,
               (struct udev_device *));
   UDEV_SYMBOL(struct udev *, udev_unref, (struct udev *));

   if (dlsym_failed)
      return NULL;

   udev = udev_new();
   device = udev_device_new_from_fd(udev, fd);
   if (device == NULL)
      return NULL;

   const_device_name = udev_device_get_devnode(device);
   if (!const_device_name)
      goto out;
   device_name = strdup(const_device_name);

out:
   udev_device_unref(device);
   udev_unref(udev);
   return device_name;
}
#endif


#if defined(HAVE_LIBDRM)
static char *
drm_get_device_name_for_fd(int fd)
{
   unsigned int maj, min;
   char buf[0x40];
   int n;

   if (dev_node_from_fd(fd, &maj, &min) < 0)
      return NULL;

   n = snprintf(buf, sizeof(buf), DRM_DEV_NAME, DRM_DIR_NAME, min);
   if (n == -1 || n >= sizeof(buf))
      return NULL;

   return strdup(buf);
}
#endif

char *
loader_get_device_name_for_fd(int fd)
{
   char *result = NULL;

#if HAVE_LIBUDEV
   if ((result = libudev_get_device_name_for_fd(fd)))
      return result;
#endif
#if HAVE_LIBDRM
   if ((result = drm_get_device_name_for_fd(fd)))
      return result;
#endif
   return result;
}

char *
loader_get_driver_for_fd(int fd)
{
   int vendor_id, chip_id, i, j;
   char *driver = NULL;

   if (!loader_get_pci_id_for_fd(fd, &vendor_id, &chip_id)) {

#if HAVE_LIBDRM
      /* fallback to drmGetVersion(): */
      drmVersionPtr version = drmGetVersion(fd);

      if (!version) {
         log_(_LOADER_WARNING, "failed to get driver name for fd %d\n", fd);
         return NULL;
      }

      driver = strndup(version->name, version->name_len);
      log_(_LOADER_INFO, "using driver %s for %d\n", driver, fd);

      drmFreeVersion(version);
#endif

      return driver;
   }

   for (i = 0; driver_map[i].driver; i++) {
      if (vendor_id != driver_map[i].vendor_id)
         continue;

      if (driver_map[i].predicate && !driver_map[i].predicate(fd))
         continue;

      if (driver_map[i].num_chips_ids == -1) {
         driver = strdup(driver_map[i].driver);
         goto out;
      }

      for (j = 0; j < driver_map[i].num_chips_ids; j++)
         if (driver_map[i].chip_ids[j] == chip_id) {
            driver = strdup(driver_map[i].driver);
            goto out;
         }
   }

out:
   log_(driver ? _LOADER_DEBUG : _LOADER_WARNING,
	"UGH....pci id for fd %d: %04x:%04x, driver %s\n",
         fd, vendor_id, chip_id, driver);
   return driver;
}

void
loader_set_logger(void (*logger)(int level, const char *fmt, ...))
{
   log_ = logger;
}
