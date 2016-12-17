Not sure if these mmap() flags are right ...

--- src/intel/tools/aubinator.c.orig	2016-11-22 00:15:57.479799000 +0100
+++ src/intel/tools/aubinator.c	2016-11-22 00:23:24.929643000 +0100
@@ -1226,7 +1226,7 @@
    /* mmap a terabyte for our gtt space. */
    gtt_size = 1ul << 40;
    gtt = mmap(NULL, gtt_size, PROT_READ | PROT_WRITE,
-              MAP_PRIVATE | MAP_ANONYMOUS |  MAP_NORESERVE, -1, 0);
+              MAP_PRIVATE | MAP_ANONYMOUS | MAP_NOSYNC | MAP_NOCORE, -1, 0);
    if (gtt == MAP_FAILED) {
       fprintf(stderr, "failed to alloc gtt space: %s\n", strerror(errno));
       exit(EXIT_FAILURE);
