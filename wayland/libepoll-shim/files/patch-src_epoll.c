--- src/epoll.c.orig	2016-08-20 21:38:57 UTC
+++ src/epoll.c
@@ -8,8 +8,9 @@
 #include <errno.h>
 #include <poll.h>
 #include <signal.h>
+#include <stdio.h>
 
-#if 0
+#if 1
 int epoll_create(int size)
 {
 	return epoll_create1(0);
@@ -37,6 +38,7 @@ epoll_ctl(int fd, int op, int fd2, struc
 	struct kevent kev;
 	if (op == EPOLL_CTL_ADD) {
 		if (ev->events != EPOLLIN) {
+			fprintf(stderr, "ERROR: Currently only EPOLLIN is supported in epoll-shim. Application may not function as intended.\n");
 			errno = EINVAL;
 			return -1;
 		}
