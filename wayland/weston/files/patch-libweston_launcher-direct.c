--- libweston/launcher-direct.c.orig	2016-09-17 07:06:45 UTC
+++ libweston/launcher-direct.c
@@ -31,9 +31,20 @@
 #include <signal.h>
 #include <sys/stat.h>
 #include <sys/ioctl.h>
-#include <linux/vt.h>
-#include <linux/kd.h>
-#include <linux/major.h>
+
+#if defined(__FreeBSD__)
+#  include <termios.h>
+#  include <sys/consio.h>
+#  include <sys/kbio.h>
+#  define TTY_BASENAME    "/dev/ttyv"
+#  define TTY_0           "/dev/ttyv0"
+#else
+#  include <linux/kd.h>
+#  include <linux/major.h>
+#  include <linux/vt.h>
+#  define TTY_BASENAME "/dev/tty"
+#  define TTY_0        "/dev/tty0"
+#endif
 
 #include "launcher-impl.h"
 
@@ -122,7 +133,7 @@ setup_tty(struct launcher_direct *launch
 			return -1;
 		}
 	} else {
-		snprintf(tty_device, sizeof tty_device, "/dev/tty%d", tty);
+		snprintf(tty_device, sizeof tty_device, "%s%d", TTY_BASENAME, tty);
 		launcher->tty = open(tty_device, O_RDWR | O_CLOEXEC);
 		if (launcher->tty == -1) {
 			weston_log("couldn't open tty %s: %m\n", tty_device);
@@ -130,6 +141,8 @@ setup_tty(struct launcher_direct *launch
 		}
 	}
 
+#if defined(__FreeBSD__)
+#else
 	if (fstat(launcher->tty, &buf) == -1 ||
 	    major(buf.st_rdev) != TTY_MAJOR || minor(buf.st_rdev) == 0) {
 		weston_log("%s not a vt\n", tty_device);
@@ -137,6 +150,7 @@ setup_tty(struct launcher_direct *launch
 			   "use --tty to specify a tty\n");
 		goto err_close;
 	}
+#endif
 
 	ret = ioctl(launcher->tty, KDGETMODE, &kd_mode);
 	if (ret) {
@@ -157,11 +171,29 @@ setup_tty(struct launcher_direct *launch
 		goto err_close;
 	}
 
+#if defined(__FreeBSD__)
+	if (ioctl(launcher->tty, KDSKBMODE, K_CODE) == -1) {
+		weston_log("Could not set keyboard mode to K_CODE");
+		goto err_close;
+	}
+	/* Put the tty into raw mode */
+	struct termios tios;
+	if (tcgetattr(launcher->tty, &tios)) {
+		weston_log("Failed to get terminal attribute");
+		goto err_close;
+	}
+	cfmakeraw(&tios);
+	if (tcsetattr(launcher->tty, TCSANOW, &tios)) {
+		weston_log("Failed to set terminal attribute");
+		goto err_close;
+	}
+#else
 	if (ioctl(launcher->tty, KDSKBMUTE, 1) &&
 	    ioctl(launcher->tty, KDSKBMODE, K_OFF)) {
 		weston_log("failed to set K_OFF keyboard mode: %m\n");
 		goto err_close;
 	}
+#endif
 
 	ret = ioctl(launcher->tty, KDSETMODE, KD_GRAPHICS);
 	if (ret) {
@@ -184,6 +216,9 @@ setup_tty(struct launcher_direct *launch
 	mode.mode = VT_PROCESS;
 	mode.relsig = SIGRTMIN;
 	mode.acqsig = SIGRTMIN;
+#if defined(__FreeBSD__)
+	mode.frsig = SIGIO; /* not used, but has to be set anyway */
+#endif
 	if (ioctl(launcher->tty, VT_SETMODE, &mode) < 0) {
 		weston_log("failed to take control of vt handling\n");
 		goto err_close;
@@ -218,14 +253,14 @@ launcher_direct_open(struct weston_launc
 		return -1;
 	}
 
-	if (major(s.st_rdev) == DRM_MAJOR) {
+	/* if (major(s.st_rdev) == DRM_MAJOR) { */
 		launcher->drm_fd = fd;
 		if (!is_drm_master(fd)) {
 			weston_log("drm fd not master\n");
 			close(fd);
 			return -1;
 		}
-	}
+	/* } */
 
 	return fd;
 }
