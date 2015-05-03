From 96ddc91bfa07d91b412afcf90e13523fe9efaf08 Mon Sep 17 00:00:00 2001
From: =?UTF-8?q?Michel=20D=C3=A4nzer?= <michel.daenzer@amd.com>
Date: Thu, 7 Mar 2013 09:59:29 +0100
Subject: UMS: Swap bytes when uploading to pixmap for solid picture on big
 endian host

Bugzilla: https://bugs.freedesktop.org/show_bug.cgi?id=57649

diff --git a/src/radeon_exa_shared.c b/src/radeon_exa_shared.c
index b83eb81..65048e7 100644
--- src/radeon_exa_shared.c
+++ src/radeon_exa_shared.c
@@ -40,6 +40,7 @@
 #endif
 #include "radeon_macros.h"
 #include "radeon_probe.h"
+#include "radeon_reg.h"
 #include "radeon_version.h"
 #include "radeon_vbo.h"

@@ -159,6 +160,17 @@ PixmapPtr RADEONSolidPixmap(ScreenPtr pScreen, uint32_t solid)

     /* XXX: Big hammer... */
     info->accel_state->exa->WaitMarker(pScreen, info->accel_state->exaSyncMarker);
+
+#if X_BYTE_ORDER == X_BIG_ENDIAN
+    if (pScrn->bitsPerPixel == 32)
+	RADEONCopySwap(info->FB + exaGetPixmapOffset(pPix), (uint8_t*)&solid, 4,
+		       RADEON_HOST_DATA_SWAP_32BIT);
+    else if (pScrn->bitsPerPixel == 16)
+	RADEONCopySwap(info->FB + exaGetPixmapOffset(pPix), (uint8_t*)&solid, 4,
+		       RADEON_HOST_DATA_SWAP_16BIT);
+    else
+	/* Fall through for 8 bpp */
+#endif
     memcpy(info->FB + exaGetPixmapOffset(pPix), &solid, 4);

     return pPix;
--
cgit v0.10.2

