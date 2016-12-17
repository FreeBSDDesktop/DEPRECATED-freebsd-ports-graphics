--- tests/nouveau/threaded.c.orig	2016-12-17 11:10:27.336998000 +0100
+++ tests/nouveau/threaded.c	2016-12-17 11:11:37.362625000 +0100
@@ -78,7 +78,7 @@ int main(int argc, char *argv[])
 	int err, fd, fd2;
 	struct nouveau_device *nvdev, *nvdev2;
 	struct nouveau_bo *bo;
-	pthread_t t1, t2;
+	pthread_t t1 = NULL, t2 = NULL;
 
 	old_ioctl = dlsym(RTLD_NEXT, "ioctl");
 
