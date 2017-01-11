--- src/session/udev.c.orig	2016-12-08 15:15:47 UTC
+++ src/session/udev.c
@@ -28,13 +28,17 @@ static struct udev {
 static int
 input_open_restricted(const char *path, int flags, void *user_data)
 {
+   wlc_log(WLC_LOG_INFO,"[udev.c %s]",__func__);
    (void)user_data;
-   return wlc_fd_open(path, flags, WLC_FD_INPUT);
+   int ret = wlc_fd_open(path, flags, WLC_FD_INPUT);
+   wlc_log(WLC_LOG_INFO,"udev.c input_open_restricted: path %s, fd %d",path,ret);
+   return ret;
 }
 
 static void
 input_close_restricted(int fd, void *user_data)
 {
+   wlc_log(WLC_LOG_INFO,"[udev.c %s] fd %d",__func__,fd);
    (void)user_data;
    wlc_fd_close(fd);
 }
@@ -111,10 +115,12 @@ input_event(int fd, uint32_t mask, void 
 
       switch (libinput_event_get_type(event)) {
          case LIBINPUT_EVENT_DEVICE_ADDED:
+			wlc_log(WLC_LOG_INFO, "LIBINPUT_EVENT_DEVICE_ADDED");
             WLC_INTERFACE_EMIT(input.created, device);
             break;
 
          case LIBINPUT_EVENT_DEVICE_REMOVED:
+			wlc_log(WLC_LOG_INFO, "LIBINPUT_EVENT_DEVICE_REMOVED");
             WLC_INTERFACE_EMIT(input.destroyed, device);
             break;
 
@@ -334,13 +340,18 @@ activate_event(struct wl_listener *liste
    (void)listener;
 
    struct wlc_activate_event *ev = data;
+   wlc_log(WLC_LOG_INFO, "udev.c activate_event: active %d", ev->active);
+
    if (input.handle) {
       if (!ev->active) {
          wlc_log(WLC_LOG_INFO, "libinput: suspend");
          libinput_suspend(input.handle);
       } else {
          wlc_log(WLC_LOG_INFO, "libinput: resume");
-         libinput_resume(input.handle);
+         int ret = libinput_resume(input.handle);
+		 if(ret) {
+			 wlc_log(WLC_LOG_ERROR, "libinput: resume failed");
+		 }
       }
    }
 }
@@ -386,7 +397,7 @@ wlc_input_init(void)
       goto failed_to_assign_seat;
 
    libinput_log_set_handler(input.handle, &cb_input_log_handler);
-   libinput_log_set_priority(input.handle, LIBINPUT_LOG_PRIORITY_ERROR);
+   libinput_log_set_priority(input.handle, LIBINPUT_LOG_PRIORITY_INFO);
    return input_set_event_loop(wlc_event_loop());
 
 failed_to_create_context:
