--- src/mesa/state_tracker/st_atom_framebuffer.c.orig	2016-12-20 08:02:59 UTC
+++ src/mesa/state_tracker/st_atom_framebuffer.c
@@ -173,7 +173,7 @@ update_framebuffer_state( struct st_cont
     */
    strb = st_renderbuffer(fb->Attachment[BUFFER_DEPTH].Renderbuffer);
    if (strb) {
-      if (strb->is_rtt) {
+      if (strb->is_rtt || !strb->surface) {
          /* rendering to a GL texture, may have to update surface */
          st_update_renderbuffer_surface(st, strb);
       }
@@ -183,7 +183,7 @@ update_framebuffer_state( struct st_cont
    else {
       strb = st_renderbuffer(fb->Attachment[BUFFER_STENCIL].Renderbuffer);
       if (strb) {
-         if (strb->is_rtt) {
+         if (strb->is_rtt || !strb->surface) {
             /* rendering to a GL texture, may have to update surface */
             st_update_renderbuffer_surface(st, strb);
          }
