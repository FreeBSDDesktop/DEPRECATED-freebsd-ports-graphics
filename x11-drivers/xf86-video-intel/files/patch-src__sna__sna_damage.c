--- src/sna/sna_damage.c.orig	2015-11-19 00:07:31.997613000 +0100
+++ src/sna/sna_damage.c	2015-11-19 00:08:00.384133000 +0100
@@ -215,7 +215,7 @@ static void __sna_damage_reduce(struct s
 	int n, nboxes;
 	BoxPtr boxes, free_boxes = NULL;
 	pixman_region16_t *region = &damage->region;
-	struct sna_damage_box *iter;
+	struct sna_damage_box *iter = NULL;
 
 	assert(damage->mode != DAMAGE_ALL);
 	assert(damage->dirty);
@@ -1808,7 +1808,7 @@ void _sna_damage_debug_get_region(struct
 {
 	int n, nboxes;
 	BoxPtr boxes;
-	struct sna_damage_box *iter;
+	struct sna_damage_box *iter = NULL;
 
 	RegionCopy(r, &damage->region);
 	if (!damage->dirty)
