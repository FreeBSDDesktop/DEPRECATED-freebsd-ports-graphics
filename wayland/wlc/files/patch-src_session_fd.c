--- src/session/fd.c.orig	2016-12-08 15:15:47 UTC
+++ src/session/fd.c
@@ -160,7 +160,8 @@ fd_open(const char *path, int flags, enu
    assert(path);
 
    struct wlc_fd *pfd = NULL;
-   for (uint32_t i = 0; i < LENGTH(wlc.fds); ++i) {
+   uint32_t i;
+   for (i = 0; i < LENGTH(wlc.fds); ++i) {
       if (wlc.fds[i].fd < 0) {
          pfd = &wlc.fds[i];
          break;
@@ -173,39 +174,41 @@ fd_open(const char *path, int flags, enu
    }
 
    /* we will only open allowed paths */
-#ifdef __linux__
 #define FILTER(x, m) { x, (sizeof(x) > 32 ? 32 : sizeof(x)) - 1, m }
    static struct {
       const char *base;
       const size_t size;
       uint32_t major;
    } allow[WLC_FD_LAST] = {
-      FILTER("/dev/input/", INPUT_MAJOR), // WLC_FD_INPUT
-      FILTER("/dev/dri/card", DRM_MAJOR), // WLC_FD_DRM
+      FILTER("/dev/input/", 0), // WLC_FD_INPUT
+      FILTER("/dev/dri/card", 0), // WLC_FD_DRM
    };
 #undef FILTER
-#endif
 
-#ifdef __linux__
    if (type > WLC_FD_LAST || memcmp(path, allow[type].base, allow[type].size)) {
       wlc_log(WLC_LOG_WARN, "Denying open from: %s", path);
       return -1;
    }
-#endif
 
    struct stat st;
-   if (stat(path, &st) < 0)
-	  return -1;
+   if (stat(path, &st) < 0) {
+	   wlc_log(WLC_LOG_ERROR,"Could not stat path %s",path);
+	   return -1;
+   }
 
 #ifdef __linux__
    if (major(st.st_rdev) != allow[type].major)
    	  return -1;
 #endif
 
+   wlc_log(WLC_LOG_INFO,"[fd.c %s]: open (syscall) path %s",__func__);
    int fd;
-   if ((fd = open(path, flags | O_CLOEXEC)) < 0)
+   if ((fd = open(path, flags | O_CLOEXEC)) < 0) {
+	  wlc_log(WLC_LOG_INFO,"Could not open path %s",path);
       return fd;
-
+   }
+   wlc_log(WLC_LOG_INFO,"[fd.c %s]: open(syscall) path %s. Stored fd %d to index %d",
+		   __func__,path,fd,i);
    pfd->fd = fd;
    pfd->type = type;
    pfd->st_dev = st.st_dev;
@@ -224,7 +227,10 @@ static void
 fd_close(dev_t st_dev, ino_t st_ino)
 {
    struct wlc_fd *pfd = NULL;
-   for (uint32_t i = 0; i < LENGTH(wlc.fds); ++i) {
+   uint32_t i;
+   wlc_log(WLC_LOG_INFO,"[fd.c %s]: st_dev %d. st_ino %d",
+		   __func__,st_dev, st_ino);
+   for (i = 0; i < LENGTH(wlc.fds); ++i) {
       if (wlc.fds[i].st_dev == st_dev && wlc.fds[i].st_ino == st_ino) {
          pfd = &wlc.fds[i];
          break;
@@ -239,7 +245,9 @@ fd_close(dev_t st_dev, ino_t st_ino)
    if (pfd->type == WLC_FD_DRM)
       drmDropMaster(pfd->fd);
 
-   close(pfd->fd);
+   wlc_log(WLC_LOG_INFO,"[fd.c %s]: Close index %d, fd %d",__func__,i,pfd->fd);
+
+   // close(pfd->fd);
    pfd->fd = -1;
 }
 
@@ -247,18 +255,23 @@ static bool
 activate(void)
 {
    for (uint32_t i = 0; i < LENGTH(wlc.fds); ++i) {
+	  wlc_log(WLC_LOG_INFO,"Activate: Iterate fd %d, value %d",i,wlc.fds[i].fd);
       if (wlc.fds[i].fd < 0)
          continue;
 
       switch (wlc.fds[i].type) {
          case WLC_FD_DRM:
-            if (drmSetMaster(wlc.fds[i].fd)) {
-               wlc_log(WLC_LOG_WARN, "Could not set master for drm fd (%d)", wlc.fds[i].fd);
+			wlc_log(WLC_LOG_INFO,"Activate: FD_DRM");
+			wlc_log(WLC_LOG_INFO,"Set drm master");
+			if (drmSetMaster(wlc.fds[i].fd)) {
+               wlc_log(WLC_LOG_WARN, "Could not set master for drm fd (%d)",
+					   wlc.fds[i].fd);
                return false;
             }
             break;
 
          case WLC_FD_INPUT:
+			 wlc_log(WLC_LOG_INFO,"Activate: FD_INPUT");
          case WLC_FD_LAST:
             break;
       }
@@ -275,6 +288,7 @@ deactivate(void)
       if (wlc.fds[i].fd < 0 || wlc.fds[i].type != WLC_FD_DRM)
          continue;
 
+	  wlc_log(WLC_LOG_INFO,"Drop drm master");
       if (drmDropMaster(wlc.fds[i].fd)) {
          wlc_log(WLC_LOG_WARN, "Could not drop master for drm fd (%d)", wlc.fds[i].fd);
          return false;
@@ -282,11 +296,14 @@ deactivate(void)
    }
 
    for (uint32_t i = 0; i < LENGTH(wlc.fds); ++i) {
+	  wlc_log(WLC_LOG_INFO,"Deactivate: Iterate fd %d, value %d",i,wlc.fds[i].fd);
       if (wlc.fds[i].fd < 0)
          continue;
 
       switch (wlc.fds[i].type) {
          case WLC_FD_INPUT:
+			wlc_log(WLC_LOG_INFO,"Got WLC_FD_INPUT type");
+			wlc_log(WLC_LOG_INFO,"Revoke and close input fd %d",wlc.fds[i].fd);
             if (ioctl(wlc.fds[i].fd, EVIOCREVOKE, 0) == -1) {
                wlc_log(WLC_LOG_WARN, "Kernel does not support EVIOCREVOKE, can not revoke input devices");
                return false;
@@ -296,6 +313,7 @@ deactivate(void)
             break;
 
          case WLC_FD_DRM:
+			 wlc_log(WLC_LOG_INFO,"Got WLC_FD_DRM type");
          case WLC_FD_LAST:
             break;
       }
@@ -307,56 +325,70 @@ deactivate(void)
 static void
 handle_request(int sock, int fd, const struct msg_request *request)
 {
+   wlc_log(WLC_LOG_INFO,"fd.c handle_request");
    struct msg_response response;
    memset(&response, 0, sizeof(response));
    response.type = request->type;
 
    switch (request->type) {
       case TYPE_CHECK:
+		 wlc_log(WLC_LOG_INFO,"fd.c handle_request: TYPE_CHECK");
          write_fd(sock, fd, &response, sizeof(response));
          break;
       case TYPE_FD_OPEN:
+		 wlc_log(WLC_LOG_INFO,"fd.c handle_request: FD_OPEN");
          fd = fd_open(request->fd_open.path, request->fd_open.flags, request->fd_open.type);
          write_fd(sock, fd, &response, sizeof(response));
          break;
       case TYPE_FD_CLOSE:
+		 wlc_log(WLC_LOG_INFO,"fd.c handle_request: FD_CLOSE");
          /* we will only close file descriptors opened by us. */
          fd_close(request->fd_close.st_dev, request->fd_close.st_ino);
          break;
       case TYPE_ACTIVATE:
+		 wlc_log(WLC_LOG_INFO,"fd.c handle_request: TYPE_ACTIVATE");
          response.activate = activate();
          write_fd(sock, fd, &response, sizeof(response));
          break;
       case TYPE_DEACTIVATE:
+		 wlc_log(WLC_LOG_INFO,"fd.c handle_request: TYPE_DEACTIVATE");
          response.deactivate = deactivate();
          write_fd(sock, fd, &response, sizeof(response));
          break;
       case TYPE_ACTIVATE_VT:
+		 wlc_log(WLC_LOG_INFO,"fd.c handle_request: TYPE_ACTIVATE_VT");
          response.activate = wlc_tty_activate_vt(request->vt_activate.vt);
          write_fd(sock, fd, &response, sizeof(response));
+		 break;
+   default:
+	   wlc_log(WLC_LOG_INFO,"fd.c handle_request: UNKNOWN");
    }
 }
 
 static void
 communicate(int sock, pid_t parent)
 {
+   wlc_log(WLC_LOG_INFO,"fd.c communicate");
    memset(wlc.fds, -1, sizeof(wlc.fds));
 
    do {
       int fd = -1;
       struct msg_request request;
-      while (recv_fd(sock, &fd, &request, sizeof(request)) == sizeof(request))
+      while (recv_fd(sock, &fd, &request, sizeof(request)) == sizeof(request)) {
          handle_request(sock, fd, &request);
+	  }
    } while (kill(parent, 0) == 0);
 
    // Close all open fds
+   wlc_log(WLC_LOG_INFO,"fd.c communicate: closing all fds");
    for (uint32_t i = 0; i < LENGTH(wlc.fds); ++i) {
       if (wlc.fds[i].fd < 0)
          continue;
 
       if (wlc.fds[i].type == WLC_FD_DRM)
-         drmDropMaster(wlc.fds[i].fd);
+		  drmDropMaster(wlc.fds[i].fd);
 
+	  wlc_log(WLC_LOG_INFO,"fd.c communicate: close fd %d", wlc.fds[i].fd);
       close(wlc.fds[i].fd);
    }
 
@@ -426,6 +458,7 @@ signal_handler(int signal)
 int
 wlc_fd_open(const char *path, int flags, enum wlc_fd_type type)
 {
+   wlc_log(WLC_LOG_INFO,"fd.c wlc_fd_open %s",path);
 #ifdef HAS_LOGIND
    if (wlc.has_logind)
       return wlc_logind_open(path, flags);
@@ -450,6 +483,7 @@ wlc_fd_open(const char *path, int flags,
 void
 wlc_fd_close(int fd)
 {
+   wlc_log(WLC_LOG_INFO,"fd.c wlc_fd_close %d",fd);
 #ifdef HAS_LOGIND
    if (wlc.has_logind) {
       wlc_logind_close(fd);
@@ -459,6 +493,8 @@ wlc_fd_close(int fd)
 
    struct stat st;
    if (fstat(fd, &st) == 0) {
+	   wlc_log(WLC_LOG_INFO,"fd.c wlc_fd_close: fd %d. st_dev %d. st_ino %d",
+			   fd,st.st_dev, st.st_ino);
       struct msg_request request;
       memset(&request, 0, sizeof(request));
       request.type = TYPE_FD_CLOSE;
@@ -481,6 +517,7 @@ wlc_fd_activate(void)
       return wlc_tty_activate();
 #endif
 
+   wlc_log(WLC_LOG_INFO,"fd.c wlc_fd_activate");
    struct msg_response response;
    struct msg_request request;
    memset(&request, 0, sizeof(request));
@@ -497,6 +534,7 @@ wlc_fd_deactivate(void)
       return wlc_tty_deactivate();
 #endif
 
+   wlc_log(WLC_LOG_INFO,"fd.c wlc_fd_deactivate");
    struct msg_response response;
    struct msg_request request;
    memset(&request, 0, sizeof(request));
@@ -513,6 +551,7 @@ wlc_fd_activate_vt(int vt)
       return wlc_tty_activate_vt(vt);
 #endif
 
+   wlc_log(WLC_LOG_INFO,"fd.c wlc_fd_activate_vt %d",vt);
    struct msg_response response;
    struct msg_request request;
    memset(&request, 0, sizeof(request));
