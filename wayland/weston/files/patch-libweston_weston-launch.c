--- libweston/weston-launch.c.orig	2016-09-17 07:06:45 UTC
+++ libweston/weston-launch.c
@@ -33,7 +33,6 @@
 #include <poll.h>
 #include <errno.h>
 
-#include <error.h>
 #include <getopt.h>
 
 #include <sys/types.h>
@@ -46,9 +45,25 @@
 #include <unistd.h>
 #include <fcntl.h>
 
-#include <linux/vt.h>
-#include <linux/major.h>
-#include <linux/kd.h>
+#if defined(__FreeBSD__)
+#  include <termios.h>
+#  include <sys/consio.h>
+#  include <sys/kbio.h>
+#  define TTY_BASENAME    "/dev/ttyv"
+#  define TTY_0           "/dev/ttyv0"
+#  define error(s,e,...) do{ fprintf(stderr, __VA_ARGS__); } while(false)
+static inline int clearenv(void) {   extern char **environ;
+   environ[0] = NULL;
+   return 0;
+}
+#else
+#  include <error.h>
+#  include <linux/kd.h>
+#  include <linux/major.h>
+#  include <linux/vt.h>
+#  define TTY_BASENAME "/dev/tty"
+#  define TTY_0        "/dev/tty0"
+#endif
 
 #include <pwd.h>
 #include <grp.h>
@@ -315,21 +330,21 @@ handle_open(struct weston_launch *wl, st
 		goto err0;
 	}
 
-	if (fstat(fd, &s) < 0) {
-		close(fd);
-		fd = -1;
-		fprintf(stderr, "Failed to stat %s\n", message->path);
-		goto err0;
-	}
+	/* if (fstat(fd, &s) < 0) { */
+	/* 	close(fd); */
+	/* 	fd = -1; */
+	/* 	fprintf(stderr, "Failed to stat %s\n", message->path); */
+	/* 	goto err0; */
+	/* } */
 
-	if (major(s.st_rdev) != INPUT_MAJOR &&
-	    major(s.st_rdev) != DRM_MAJOR) {
-		close(fd);
-		fd = -1;
-		fprintf(stderr, "Device %s is not an input or drm device\n",
-			message->path);
-		goto err0;
-	}
+	/* if (major(s.st_rdev) != INPUT_MAJOR && */
+	/*     major(s.st_rdev) != DRM_MAJOR) { */
+	/* 	close(fd); */
+	/* 	fd = -1; */
+	/* 	fprintf(stderr, "Device %s is not an input or drm device\n", */
+	/* 		message->path); */
+	/* 	goto err0; */
+	/* } */
 
 err0:
 	memset(&nmsg, 0, sizeof nmsg);
@@ -360,10 +375,9 @@ err0:
 	if (len < 0)
 		return -1;
 
-	if (fd != -1 && major(s.st_rdev) == DRM_MAJOR)
+	if (fd != -1)
 		wl->drm_fd = fd;
-	if (fd != -1 && major(s.st_rdev) == INPUT_MAJOR &&
-	    wl->last_input_fd < fd)
+	if (fd != -1 && wl->last_input_fd < fd)
 		wl->last_input_fd = fd;
 
 	return 0;
@@ -422,9 +436,20 @@ quit(struct weston_launch *wl, int statu
 		pam_end(wl->ph, err);
 	}
 
-	if (ioctl(wl->tty, KDSKBMUTE, 0) &&
-	    ioctl(wl->tty, KDSKBMODE, wl->kb_mode))
+#if defined(__FreeBSD__)
+	/* Enable keyboard in vt */
+	if (ioctl(wl->tty, KDSKBMODE, wl->kb_mode))
+		error(1, errno, "Could not restore keyboard\n");
+	struct termios tios;
+	if (tcgetattr(wl->tty, &tios))
+		error(1, errno, "Failed to get terminal attribute\n");
+	cfmakesane(&tios);
+	if (tcsetattr(wl->tty , TCSANOW | TCSAFLUSH, &tios))
+		error(1, errno, "Failed to set terminal attribute\n");
+#else
+	if (ioctl(wl->tty, KDSKBMUTE, 0) && ioctl(wl->tty, KDSKBMODE, wl->kb_mode))
 		fprintf(stderr, "failed to restore keyboard mode: %m\n");
+#endif
 
 	if (ioctl(wl->tty, KDSETMODE, KD_TEXT))
 		fprintf(stderr, "failed to set KD_TEXT mode on tty: %m\n");
@@ -435,6 +460,10 @@ quit(struct weston_launch *wl, int statu
 	drmDropMaster(wl->drm_fd);
 
 	mode.mode = VT_AUTO;
+#if defined(__FreeBSD__)
+	mode.frsig = SIGIO; /* not used, but has to be set anyway */
+#endif
+	// Fails on FreeBSD for some reason
 	if (ioctl(wl->tty, VT_SETMODE, &mode) < 0)
 		fprintf(stderr, "could not reset vt handling\n");
 
@@ -448,12 +477,12 @@ close_input_fds(struct weston_launch *wl
 	int fd;
 
 	for (fd = 3; fd <= wl->last_input_fd; fd++) {
-		if (fstat(fd, &s) == 0 && major(s.st_rdev) == INPUT_MAJOR) {
+		/* if (fstat(fd, &s) == 0 && major(s.st_rdev) == INPUT_MAJOR) { */
 			/* EVIOCREVOKE may fail if the kernel doesn't
 			 * support it, but all we can do is ignore it. */
 			ioctl(fd, EVIOCREVOKE, 0);
 			close(fd);
-		}
+		/* } */
 	}
 }
 
@@ -527,7 +556,7 @@ setup_tty(struct weston_launch *wl, cons
 		else
 			wl->tty = open(tty, O_RDWR | O_NOCTTY);
 	} else {
-		int tty0 = open("/dev/tty0", O_WRONLY | O_CLOEXEC);
+		int tty0 = open(TTY_0, O_WRONLY | O_CLOEXEC);
 		char filename[16];
 
 		if (tty0 < 0)
@@ -536,7 +565,7 @@ setup_tty(struct weston_launch *wl, cons
 		if (ioctl(tty0, VT_OPENQRY, &wl->ttynr) < 0 || wl->ttynr == -1)
 			error(1, errno, "failed to find non-opened console");
 
-		snprintf(filename, sizeof filename, "/dev/tty%d", wl->ttynr);
+		snprintf(filename, sizeof filename, "%s%d", TTY_BASENAME, wl->ttynr);
 		wl->tty = open(filename, O_RDWR | O_NOCTTY);
 		close(tty0);
 	}
@@ -544,26 +573,38 @@ setup_tty(struct weston_launch *wl, cons
 	if (wl->tty < 0)
 		error(1, errno, "failed to open tty");
 
-	if (fstat(wl->tty, &buf) == -1 ||
-	    major(buf.st_rdev) != TTY_MAJOR || minor(buf.st_rdev) == 0)
-		error(1, 0, "weston-launch must be run from a virtual terminal");
+	/* if (fstat(wl->tty, &buf) == -1 || */
+	/*     major(buf.st_rdev) != TTY_MAJOR || minor(buf.st_rdev) == 0) */
+	/* 	error(1, 0, "weston-launch must be run from a virtual terminal"); */
 
 	if (tty) {
 		if (fstat(wl->tty, &buf) < 0)
 			error(1, errno, "stat %s failed", tty);
+	/* 	if (major(buf.st_rdev) != TTY_MAJOR) */
+	/* 		error(1, 0, "invalid tty device: %s", tty); */
 
-		if (major(buf.st_rdev) != TTY_MAJOR)
-			error(1, 0, "invalid tty device: %s", tty);
-
-		wl->ttynr = minor(buf.st_rdev);
+		// NOTE: What should we put here?
+		wl->ttynr = 0; //minor(buf.st_rdev);
 	}
 
 	if (ioctl(wl->tty, KDGKBMODE, &wl->kb_mode))
 		error(1, errno, "failed to get current keyboard mode: %m\n");
 
+#if defined(__FreeBSD__)
+	/* Disable keyboard in vt */
+	if (ioctl(wl->tty, KDSKBMODE, K_CODE) == -1)
+		error(1, errno, "Could not set keyboard mode to K_CODE\n");
+	struct termios tios;
+	if (tcgetattr(wl->tty, &tios))
+		error(1, errno, "Failed to get terminal attribute %m\n");
+	cfmakeraw(&tios);
+	if (tcsetattr(wl->tty , TCSANOW | TCSAFLUSH, &tios))
+		error(1, errno, "Failed to get terminal attribute %m\n");
+#else
 	if (ioctl(wl->tty, KDSKBMUTE, 1) &&
 	    ioctl(wl->tty, KDSKBMODE, K_OFF))
 		error(1, errno, "failed to set K_OFF keyboard mode: %m\n");
+#endif
 
 	if (ioctl(wl->tty, KDSETMODE, KD_GRAPHICS))
 		error(1, errno, "failed to set KD_GRAPHICS mode on tty: %m\n");
@@ -571,6 +612,9 @@ setup_tty(struct weston_launch *wl, cons
 	mode.mode = VT_PROCESS;
 	mode.relsig = SIGUSR1;
 	mode.acqsig = SIGUSR2;
+#if defined(__FreeBSD__)
+	mode.frsig = SIGIO; /* not used, but has to be set anyway */
+#endif
 	if (ioctl(wl->tty, VT_SETMODE, &mode) < 0)
 		error(1, errno, "failed to take control of vt handling\n");
 
