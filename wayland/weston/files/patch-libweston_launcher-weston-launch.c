--- libweston/launcher-weston-launch.c.orig	2016-09-17 07:06:45 UTC
+++ libweston/launcher-weston-launch.c
@@ -37,9 +37,21 @@
 #include <sys/ioctl.h>
 #include <fcntl.h>
 #include <unistd.h>
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
+
 
 #include "compositor.h"
 #include "weston-launch.h"
@@ -240,8 +252,11 @@ launcher_weston_launch_connect(struct we
 		/* We don't get a chance to read out the original kb
 		 * mode for the tty, so just hard code K_UNICODE here
 		 * in case we have to clean if weston-launch dies. */
+#if defined(__FreeBSD__)
+		launcher->kb_mode = K_XLATE;
+#else
 		launcher->kb_mode = K_UNICODE;
-
+#endif
 		loop = wl_display_get_event_loop(compositor->wl_display);
 		launcher->source = wl_event_loop_add_fd(loop, launcher->fd,
 							WL_EVENT_READABLE,
