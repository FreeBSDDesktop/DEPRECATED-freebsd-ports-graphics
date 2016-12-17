--- libweston/compositor-drm.c.orig	2016-09-17 07:06:45 UTC
+++ libweston/compositor-drm.c
@@ -34,7 +34,7 @@
 #include <fcntl.h>
 #include <unistd.h>
 #include <linux/input.h>
-#include <linux/vt.h>
+//#include <linux/vt.h>
 #include <assert.h>
 #include <sys/mman.h>
 #include <dlfcn.h>
@@ -2853,13 +2853,19 @@ find_primary_gpu(struct drm_backend *b, 
 	struct udev_device *device, *drm_device, *pci;
 
 	e = udev_enumerate_new(b->udev);
+
+	// libudev don't support matching properly yet
+	device = udev_device_new_from_syspath(e, "/dev/dri/card0");
+	return device;
+
 	udev_enumerate_add_match_subsystem(e, "drm");
-	udev_enumerate_add_match_sysname(e, "card[0-9]*");
+	udev_enumerate_add_match_sysname(e, "card0");
 
 	udev_enumerate_scan_devices(e);
 	drm_device = NULL;
 	udev_list_entry_foreach(entry, udev_enumerate_get_list_entry(e)) {
 		path = udev_list_entry_get_name(entry);
+		printf("udev iterate got path %s\n",path);
 		device = udev_device_new_from_syspath(b->udev, path);
 		if (!device)
 			continue;
