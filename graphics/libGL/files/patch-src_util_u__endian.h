--- src/util/u_endian.h.orig	2016-11-22 00:06:47.592740000 +0100
+++ src/util/u_endian.h	2016-11-22 00:07:30.762691000 +0100
@@ -54,7 +54,8 @@
 # define PIPE_ARCH_BIG_ENDIAN
 #endif
 
-#elif defined(__OpenBSD__) || defined(__NetBSD__)
+#elif defined(__OpenBSD__) || defined(__NetBSD__)|| \
+  defined(__FreeBSD__) || defined(__DragonFly__)
 #include <sys/types.h>
 #include <machine/endian.h>
 
