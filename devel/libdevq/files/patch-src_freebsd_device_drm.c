--- src/freebsd/device_drm.c.orig	2016-07-09 18:39:04.000000000 +0200
+++ src/freebsd/device_drm.c	2016-12-05 16:42:29.862372000 +0100
@@ -33,8 +33,59 @@
 #include <stdlib.h>
 #include <string.h>
 
+#include <fcntl.h>
+#include <unistd.h>
+
 #include "libdevq.h"
 
+#define DRM_DIR_NAME "/dev/dri/"
+
+/*
+ * XXX temporary workaround, because FreeBSD doesn't provide
+ * pcibus device sysctl trees for renderD and controlD nodes (yet)
+ */
+static int
+drmBSDDeviceNameHack(const int fd)
+{
+  int i, size, temp_fd;
+  long val;
+  char *hacked_path;
+
+  char *path;
+
+// check if fd is "known" /dev/dri prefix and point to /dev/dri/card is so
+//  fstat(fd, &buf);
+//  path = devname(buf.st_rdev, S_IFCHR);
+  path = fdevname(fd);
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
+  temp_fd = open(hacked_path, O_RDONLY);
+
+  return temp_fd;
+}
+
 int
 devq_device_drm_get_drvname_from_fd(int fd,
     char *driver_name, size_t *driver_name_len)
@@ -44,15 +95,24 @@ devq_device_drm_get_drvname_from_fd(int 
 	char sysctl_name[32], sysctl_value[128];
 	size_t sysctl_value_len, name_len;
 	long dev;
+	int temp_fd;
 
-	ret = fstat(fd, &st);
-	if (ret != 0)
+	// temp workaround until all /dev/dri entries have a valid sysctl tree
+	// so we can find out information we want
+	temp_fd = drmBSDDeviceNameHack(fd);
+
+	ret = fstat(temp_fd, &st);
+	if (ret != 0) {
+		close(temp_fd);
 		return (-1);
+	}
 	if (!S_ISCHR(st.st_mode)) {
 		errno = EBADF;
+		close(temp_fd);
 		return (-1);
 	}
 
+
 	/*
 	 * Walk all the hw.dri.$n tree and compare the number stored at
 	 * the end of hw.dri.$n.name (eg. "radeon 0x9b") to the value in
@@ -76,6 +136,7 @@ devq_device_drm_get_drvname_from_fd(int 
 			if (*driver_name_len < name_len) {
 				*driver_name_len = name_len;
 				errno = ENOMEM;
+				close(temp_fd);
 				return (-1);
 			}
 
@@ -89,10 +150,13 @@ devq_device_drm_get_drvname_from_fd(int 
 		 * number; this could be useful to others.
 		 */
 		dev = strtol(sysctl_value + name_len, NULL, 16);
-		if (dev == (long)st.st_rdev)
+		if (dev == (long)st.st_rdev) {
+			close(temp_fd);
 			return (i);
+		}
 	}
 
 	errno = ENOENT;
+	close(temp_fd);
 	return (-1);
 }
