--- src/wlc.c.orig	2016-12-08 15:15:47 UTC
+++ src/wlc.c
@@ -98,9 +98,11 @@ wlc_get_time(struct timespec *out_ts)
 void
 wlc_set_active(bool active)
 {
-   if (active == wlc.active)
+   wlc_log(WLC_LOG_INFO, "wlc_set_active %d",active);
+   if (active == wlc.active) {
+	  wlc_log(WLC_LOG_INFO, "already active");
       return;
-
+   }
    wlc.active = active;
    struct wlc_activate_event ev = { .active = active, .vt = 0 };
    wl_signal_emit(&wlc.signals.activate, &ev);
