--- src/fg_internal.h.orig	2015-10-16 13:23:29 UTC
+++ src/fg_internal.h
@@ -967,6 +967,7 @@ int         fgJoystickDetect( void );
 void        fgInitialiseJoysticks( void );
 void        fgJoystickClose( void );
 void        fgJoystickPollWindow( SFG_Window* window );
+void        fgJoystickRawRead( SFG_Joystick* joy, int* buttons, float* axes );
 
 /* InputDevice Initialisation and Closure */
 int         fgInputDeviceDetect( void );
