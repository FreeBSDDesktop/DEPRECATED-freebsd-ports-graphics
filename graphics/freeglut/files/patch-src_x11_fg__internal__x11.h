--- src/x11/fg_internal_x11.h.orig	2015-10-16 10:42:49 UTC
+++ src/x11/fg_internal_x11.h
@@ -138,6 +138,31 @@ struct tagSFG_PlatformWindowState
 #    if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__)
 /* XXX The below hack is done until freeglut's autoconf is updated. */
 #        define HAVE_USB_JS    1
+#        if defined(__NetBSD__)
+/* XXX The below hack is done until freeglut's autoconf is updated. */
+#            define HAVE_USBHID_H 1
+#            ifdef HAVE_USBHID_H
+#                include <usbhid.h>
+#            else
+#                include <usb.h>
+#            endif
+#        elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
+#            ifdef HAVE_USBHID_H
+#                include <usbhid.h>
+#            else
+#                include <libusbhid.h>
+#            endif
+#        endif
+#        include <dev/usb/usb.h>
+#        if __FreeBSD_version >= 800061
+#            include <dev/usb/usb_ioctl.h>
+#        endif
+#        include <dev/usb/usbhid.h>
+
+/* Compatibility with older usb.h revisions */
+#        if !defined(USB_MAX_DEVNAMES) && defined(MAXDEVNAMES)
+#            define USB_MAX_DEVNAMES MAXDEVNAMES
+#        endif
 
 #        if defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
 #            include <sys/joystick.h>
@@ -189,6 +214,30 @@ struct tagSFG_PlatformWindowState
  */
 #    define _JS_MAX_AXES 16
 typedef struct tagSFG_PlatformJoystick SFG_PlatformJoystick;
+
+#    if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__)
+struct os_specific_s {
+  char             fname [128 ];
+  int              fd;
+  int              is_analog;
+  /* The following structure members are specific to analog joysticks */
+  struct joystick  ajs;
+#      ifdef HAVE_USB_JS
+  /* The following structure members are specific to USB joysticks */
+  struct hid_item *hids;
+  int              hid_dlen;
+  int              hid_offset;
+  char            *hid_data_buf;
+  int              axes_usage [ _JS_MAX_AXES ];
+#      endif
+  /* We keep button and axes state ourselves, as they might not be updated
+   * on every read of a USB device
+   */
+  int              cache_buttons;
+  float            cache_axes [ _JS_MAX_AXES ];
+};
+#    endif
+
 struct tagSFG_PlatformJoystick
 {
 #   if defined(__FreeBSD__) || defined(__FreeBSD_kernel__) || defined(__NetBSD__)
