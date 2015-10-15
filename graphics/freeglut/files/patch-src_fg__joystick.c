--- src/fg_joystick.c.orig	2014-10-20 15:27:04 UTC
+++ src/fg_joystick.c
@@ -60,7 +60,10 @@
 #                include <libusbhid.h>
 #            endif
 #        endif
-#        include <legacy/dev/usb/usb.h>
+#        include <dev/usb/usb.h>
+#        if __FreeBSD_version >= 800061
+#            include <dev/usb/usb_ioctl.h>
+#        endif
 #        include <dev/usb/usbhid.h>
 
 /* Compatibility with older usb.h revisions */
@@ -107,6 +110,7 @@ struct os_specific_s {
  */
 static char *fghJoystickWalkUSBdev(int f, char *dev, char *out, int outlen)
 {
+#if __FreeBSD_version < 800061
   struct usb_device_info di;
   int i, a;
   char *cp;
@@ -128,6 +132,7 @@ static char *fghJoystickWalkUSBdev(int f
         return out;
       }
   }
+#endif
   return NULL;
 }
 
