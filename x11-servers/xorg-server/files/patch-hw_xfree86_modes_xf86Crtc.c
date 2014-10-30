From d1440783a7367ff0d0c47d256bbca3b3cf8a5034 Mon Sep 17 00:00:00 2001
From: Dave Airlie <airlied@redhat.com>
Date: Tue, 29 Oct 2013 12:09:26 -0400
Subject: xfree86: return NULL for compat output if no outputs.

With outputless GPUs showing up we crash here if there are not outputs
try and recover with a bit of grace.

Reviewed-by: Adam Jackson <ajax@redhat.com>
Signed-off-by: Dave Airlie <airlied@redhat.com>
Signed-off-by: Keith Packard <keithp@keithp.com>

diff --git a/hw/xfree86/modes/xf86Crtc.c b/hw/xfree86/modes/xf86Crtc.c
index 2a02c85..a441fd1 100644
--- hw/xfree86/modes/xf86Crtc.c
+++ hw/xfree86/modes/xf86Crtc.c
@@ -1863,6 +1863,9 @@ SetCompatOutput(xf86CrtcConfigPtr config)
     DisplayModePtr maxmode = NULL, testmode, mode;
     int o, compat = -1, count, mincount = 0;
 
+    if (config->num_output == 0)
+        return NULL;
+
     /* Look for one that's definitely connected */
     for (o = 0; o < config->num_output; o++) {
         test = config->output[o];
-- 
cgit v0.10.2

