--- include/sys/epoll.h.orig	2016-08-20 21:38:57 UTC
+++ include/sys/epoll.h
@@ -54,7 +54,7 @@ __attribute__((packed))
 #endif
 ;
 
-#if 0
+#if 1
 int epoll_create(int);
 #endif
 int epoll_create1(int);
