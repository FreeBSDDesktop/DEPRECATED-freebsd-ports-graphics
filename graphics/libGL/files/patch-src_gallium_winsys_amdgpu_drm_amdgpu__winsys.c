--- src/gallium/winsys/amdgpu/drm/amdgpu_winsys.c.orig	2016-12-24 06:45:23 UTC
+++ src/gallium/winsys/amdgpu/drm/amdgpu_winsys.c
@@ -107,6 +107,7 @@ static bool do_winsys_init(struct amdgpu
    int r, i, j;
    drmDevicePtr devinfo;
 
+#ifdef NEED_REDUNDANT_PCIINFO
    /* Get PCI info. */
    r = drmGetDevice(fd, &devinfo);
    if (r) {
@@ -118,6 +119,7 @@ static bool do_winsys_init(struct amdgpu
    ws->info.pci_dev = devinfo->businfo.pci->dev;
    ws->info.pci_func = devinfo->businfo.pci->func;
    drmFreeDevice(&devinfo);
+#endif
 
    /* Query hardware and driver information. */
    r = amdgpu_query_gpu_info(ws->dev, &ws->amdinfo);
