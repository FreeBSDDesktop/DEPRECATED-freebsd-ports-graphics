--- src/x11/fg_internal_x11.h.orig	2015-10-16 10:42:49 UTC
+++ src/x11/fg_internal_x11.h
@@ -153,6 +153,28 @@ struct tagSFG_PlatformWindowState
 #        endif
 #        define JS_DATA_TYPE joystick
 #        define JS_RETURN (sizeof(struct JS_DATA_TYPE))
+
+struct os_specific_s {
+  char             fname [128 ];
+  int              fd;
+  int              is_analog;
+  /* The following structure members are specific to analog joysticks */
+  struct joystick  ajs;
+#    ifdef HAVE_USB_JS
+  /* The following structure members are specific to USB joysticks */
+  struct hid_item *hids;
+  int              hid_dlen;
+  int              hid_offset;
+  char            *hid_data_buf;
+  int              axes_usage [ _JS_MAX_AXES ];
+#    endif
+  /* We keep button and axes state ourselves, as they might not be updated
+   * on every read of a USB device
+   */
+  int              cache_buttons;
+  float            cache_axes [ _JS_MAX_AXES ];
+};
+
 #    endif
 
 #    if defined(__linux__)
