--- clients/window.c.orig	2016-09-17 07:06:45 UTC
+++ clients/window.c
@@ -6113,7 +6113,8 @@ handle_display_data(struct task *task, u
 	if (events & EPOLLOUT) {
 		ret = wl_display_flush(display->display);
 		if (ret == 0) {
-			ep.events = EPOLLIN | EPOLLERR | EPOLLHUP;
+			// Only EPOLLIN supported by epoll-shim
+			ep.events = EPOLLIN /*| EPOLLERR | EPOLLHUP*/;
 			ep.data.ptr = &display->display_task;
 			epoll_ctl(display->epoll_fd, EPOLL_CTL_MOD,
 				  display->display_fd, &ep);
@@ -6158,7 +6159,8 @@ display_create(int *argc, char *argv[])
 	d->epoll_fd = os_epoll_create_cloexec();
 	d->display_fd = wl_display_get_fd(d->display);
 	d->display_task.run = handle_display_data;
-	display_watch_fd(d, d->display_fd, EPOLLIN | EPOLLERR | EPOLLHUP,
+	// Only EPOLLIN supported by epoll-shim
+	display_watch_fd(d, d->display_fd, EPOLLIN /*| EPOLLERR | EPOLLHUP*/,
 			 &d->display_task);
 
 	wl_list_init(&d->deferred_list);
@@ -6415,7 +6417,7 @@ display_run(struct display *display)
 		ret = wl_display_flush(display->display);
 		if (ret < 0 && errno == EAGAIN) {
 			ep[0].events =
-				EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLHUP;
+				EPOLLIN /*| EPOLLOUT | EPOLLERR | EPOLLHUP*/;
 			ep[0].data.ptr = &display->display_task;
 
 			epoll_ctl(display->epoll_fd, EPOLL_CTL_MOD,
