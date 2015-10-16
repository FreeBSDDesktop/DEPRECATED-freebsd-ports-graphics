--- src/fg_joystick.c.orig	2014-10-20 15:27:04 UTC
+++ src/fg_joystick.c
@@ -44,53 +44,9 @@
 
 #if defined(__NetBSD__) || defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
 
-#    ifdef HAVE_USB_JS
-#        if defined(__NetBSD__)
-/* XXX The below hack is done until freeglut's autoconf is updated. */
-#            define HAVE_USBHID_H 1
-#            ifdef HAVE_USBHID_H
-#                include <usbhid.h>
-#            else
-#                include <usb.h>
-#            endif
-#        elif defined(__FreeBSD__) || defined(__FreeBSD_kernel__)
-#            ifdef HAVE_USBHID_H
-#                include <usbhid.h>
-#            else
-#                include <libusbhid.h>
-#            endif
-#        endif
-#        include <legacy/dev/usb/usb.h>
-#        include <dev/usb/usbhid.h>
-
-/* Compatibility with older usb.h revisions */
-#        if !defined(USB_MAX_DEVNAMES) && defined(MAXDEVNAMES)
-#            define USB_MAX_DEVNAMES MAXDEVNAMES
-#        endif
-#    endif
 
 static int hatmap_x[9] = { 0, 0, 1, 1, 1, 0, -1, -1, -1 };
 static int hatmap_y[9] = { 0, 1, 1, 0, -1, -1, -1, 0, 1 };
-struct os_specific_s {
-  char             fname [128 ];
-  int              fd;
-  int              is_analog;
-  /* The following structure members are specific to analog joysticks */
-  struct joystick  ajs;
-#    ifdef HAVE_USB_JS
-  /* The following structure members are specific to USB joysticks */
-  struct hid_item *hids;
-  int              hid_dlen;
-  int              hid_offset;
-  char            *hid_data_buf;
-  int              axes_usage [ _JS_MAX_AXES ];
-#    endif
-  /* We keep button and axes state ourselves, as they might not be updated
-   * on every read of a USB device
-   */
-  int              cache_buttons;
-  float            cache_axes [ _JS_MAX_AXES ];
-};
 
 /* Idents lower than USB_IDENT_OFFSET are for analog joysticks. */
 #    define USB_IDENT_OFFSET    2
@@ -107,6 +63,7 @@ struct os_specific_s {
  */
 static char *fghJoystickWalkUSBdev(int f, char *dev, char *out, int outlen)
 {
+#if __FreeBSD_version < 800061
   struct usb_device_info di;
   int i, a;
   char *cp;
@@ -128,6 +85,7 @@ static char *fghJoystickWalkUSBdev(int f
         return out;
       }
   }
+#endif
   return NULL;
 }
 
