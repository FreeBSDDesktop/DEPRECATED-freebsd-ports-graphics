--- src/sna/kgem.c.orig	2014-12-09 17:08:21.000000000 +0100
+++ src/sna/kgem.c	2015-11-19 00:06:27.883010000 +0100
@@ -25,6 +25,7 @@
  *
  */
 
+#define _WITH_GETLINE
 #ifdef HAVE_CONFIG_H
 #include "config.h"
 #endif
@@ -2364,7 +2365,8 @@ static bool kgem_bo_move_to_cache(struct
 static struct kgem_bo *
 search_snoop_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags)
 {
-	struct kgem_bo *bo, *first = NULL;
+	struct kgem_bo *bo = NULL;
+	struct kgem_bo *first = NULL;
 
 	DBG(("%s: num_pages=%d, flags=%x\n", __FUNCTION__, num_pages, flags));
 
@@ -2625,7 +2627,8 @@ void kgem_retire__buffers(struct kgem *k
 
 static bool kgem_retire__flushing(struct kgem *kgem)
 {
-	struct kgem_bo *bo, *next;
+	struct kgem_bo *bo = NULL;
+	struct kgem_bo *next;
 	bool retired = false;
 
 	list_for_each_entry_safe(bo, next, &kgem->flushing, request) {
@@ -2890,7 +2893,7 @@ static void kgem_commit__check_reloc(str
 #ifndef NDEBUG
 static void kgem_commit__check_buffers(struct kgem *kgem)
 {
-	struct kgem_buffer *bo;
+	struct kgem_buffer *bo = NULL;
 
 	list_for_each_entry(bo, &kgem->active_buffers, base.list)
 		assert(bo->base.exec == NULL);
@@ -2902,7 +2905,8 @@ static void kgem_commit__check_buffers(s
 static void kgem_commit(struct kgem *kgem)
 {
 	struct kgem_request *rq = kgem->next_request;
-	struct kgem_bo *bo, *next;
+	struct kgem_bo *bo = NULL;
+	struct kgem_bo *next;
 
 	kgem_commit__check_reloc(kgem);
 
@@ -2994,7 +2998,8 @@ static void kgem_close_inactive(struct k
 
 static void kgem_finish_buffers(struct kgem *kgem)
 {
-	struct kgem_buffer *bo, *next;
+	struct kgem_buffer *bo = NULL;
+	struct kgem_buffer *next;
 
 	list_for_each_entry_safe(bo, next, &kgem->batch_buffers, base.list) {
 		DBG(("%s: buffer handle=%d, used=%d, exec?=%d, write=%d, mmapped=%s, refcnt=%d\n",
@@ -3669,7 +3674,7 @@ void _kgem_submit(struct kgem *kgem)
 			       kgem->nreloc, kgem->nexec, kgem->nfence, kgem->aperture, kgem->aperture_fenced, kgem->aperture_high, kgem->aperture_total, -ret);
 
 			for (i = 0; i < kgem->nexec; i++) {
-				struct kgem_bo *bo, *found = NULL;
+				struct kgem_bo *bo = NULL, *found = NULL;
 
 				list_for_each_entry(bo, &kgem->next_request->buffers, request) {
 					if (bo->handle == kgem->exec[i].handle) {
@@ -4055,7 +4060,8 @@ bool kgem_cleanup_cache(struct kgem *kge
 static struct kgem_bo *
 search_linear_cache(struct kgem *kgem, unsigned int num_pages, unsigned flags)
 {
-	struct kgem_bo *bo, *first = NULL;
+	struct kgem_bo *bo = NULL;
+	struct kgem_bo *first = NULL;
 	bool use_active = (flags & CREATE_INACTIVE) == 0;
 	struct list *cache;
 
@@ -4817,7 +4823,7 @@ struct kgem_bo *kgem_create_2d(struct kg
 			       uint32_t flags)
 {
 	struct list *cache;
-	struct kgem_bo *bo;
+	struct kgem_bo *bo = NULL;
 	uint32_t pitch, tiled_height, size;
 	uint32_t handle;
 	int i, bucket, retry;
@@ -6568,7 +6574,7 @@ void kgem_bo_sync__gtt(struct kgem *kgem
 void kgem_clear_dirty(struct kgem *kgem)
 {
 	struct list * const buffers = &kgem->next_request->buffers;
-	struct kgem_bo *bo;
+	struct kgem_bo *bo = NULL;
 
 	list_for_each_entry(bo, buffers, request) {
 		if (!bo->gpu_dirty)
@@ -6848,7 +6854,7 @@ struct kgem_bo *kgem_create_buffer(struc
 				   uint32_t size, uint32_t flags,
 				   void **ret)
 {
-	struct kgem_buffer *bo;
+	struct kgem_buffer *bo = NULL;
 	unsigned offset, alloc;
 	struct kgem_bo *old;
 
