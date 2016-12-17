--- compositor/main.c.orig	2016-09-17 07:06:45 UTC
+++ compositor/main.c
@@ -41,7 +41,9 @@
 #include <sys/socket.h>
 #include <libinput.h>
 #include <sys/time.h>
-#include <linux/limits.h>
+/* #include <linux/limits.h> */
+#include <limits.h>
+#define PATH_MAX _POSIX_PATH_MAX
 
 #ifdef HAVE_LIBUNWIND
 #define UNW_LOCAL_ONLY
@@ -636,9 +638,9 @@ clock_name(clockid_t clk_id)
 	static const char *names[] = {
 		[CLOCK_REALTIME] =		"CLOCK_REALTIME",
 		[CLOCK_MONOTONIC] =		"CLOCK_MONOTONIC",
-		[CLOCK_MONOTONIC_RAW] =		"CLOCK_MONOTONIC_RAW",
-		[CLOCK_REALTIME_COARSE] =	"CLOCK_REALTIME_COARSE",
-		[CLOCK_MONOTONIC_COARSE] =	"CLOCK_MONOTONIC_COARSE",
+		/* [CLOCK_MONOTONIC_RAW] =		"CLOCK_MONOTONIC_RAW", */
+		/* [CLOCK_REALTIME_COARSE] =	"CLOCK_REALTIME_COARSE", */
+		/* [CLOCK_MONOTONIC_COARSE] =	"CLOCK_MONOTONIC_COARSE", */
 #ifdef CLOCK_BOOTTIME
 		[CLOCK_BOOTTIME] =		"CLOCK_BOOTTIME",
 #endif
