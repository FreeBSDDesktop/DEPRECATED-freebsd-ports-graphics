--- src/sna/sna_display.c.orig	2015-11-19 09:53:59.499570000 +0100
+++ src/sna/sna_display.c	2015-11-19 09:55:11.879113000 +0100
@@ -1116,7 +1116,7 @@ static bool wait_for_shadow(struct sna *
 
 	if ((flags & MOVE_WRITE) == 0) {
 		if ((flags & __MOVE_SCANOUT) == 0) {
-			struct sna_crtc *crtc;
+			struct sna_crtc *crtc = NULL;
 
 			list_for_each_entry(crtc, &sna->mode.shadow_crtc, shadow_link) {
 				if (overlap(&sna->mode.shadow_region.extents,
@@ -1160,7 +1160,7 @@ static bool wait_for_shadow(struct sna *
 
 	flip_active = sna->mode.flip_active;
 	if (flip_active) {
-		struct sna_crtc *crtc;
+		struct sna_crtc *crtc = NULL;
 		list_for_each_entry(crtc, &sna->mode.shadow_crtc, shadow_link)
 			flip_active -= crtc->flip_pending;
 		DBG(("%s: %d flips still pending, shadow flip_active=%d\n",
@@ -1172,7 +1172,7 @@ static bool wait_for_shadow(struct sna *
 		sna->kgem.need_throttle = false;
 
 		while (flip_active && sna_mode_wakeup(sna)) {
-			struct sna_crtc *crtc;
+			struct sna_crtc *crtc = NULL;
 
 			flip_active = sna->mode.flip_active;
 			list_for_each_entry(crtc, &sna->mode.shadow_crtc, shadow_link)
