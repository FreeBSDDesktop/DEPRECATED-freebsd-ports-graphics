--- clients/terminal.c.orig	2016-09-17 07:06:45 UTC
+++ clients/terminal.c
@@ -32,7 +32,11 @@
 #include <unistd.h>
 #include <math.h>
 #include <time.h>
-#include <pty.h>
+/* #include <pty.h> */
+#include <sys/types.h>
+#include <sys/ioctl.h>
+#include <termios.h>
+#include <libutil.h>
 #include <ctype.h>
 #include <cairo.h>
 #include <sys/epoll.h>
@@ -3041,7 +3045,7 @@ terminal_run(struct terminal *terminal, 
 	fcntl(master, F_SETFL, O_NONBLOCK);
 	terminal->io_task.run = io_handler;
 	display_watch_fd(terminal->display, terminal->master,
-			 EPOLLIN | EPOLLHUP, &terminal->io_task);
+					 EPOLLIN | EPOLLHUP, &terminal->io_task);
 
 	if (option_fullscreen)
 		window_set_fullscreen(terminal->window, 1);
@@ -3072,7 +3076,7 @@ int main(int argc, char *argv[])
 
 	option_shell = getenv("SHELL");
 	if (!option_shell)
-		option_shell = "/bin/bash";
+		option_shell = "/bin/sh";
 
 	config_file = weston_config_get_name_from_env();
 	config = weston_config_parse(config_file);
