--- xf86drm.c.orig	2016-12-23 03:12:31 UTC
+++ xf86drm.c
@@ -82,8 +82,12 @@
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
