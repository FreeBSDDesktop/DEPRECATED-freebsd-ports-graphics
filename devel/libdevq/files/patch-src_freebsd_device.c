--- src/freebsd/device.c.orig	2016-07-28 12:50:48.000000000 +0200
+++ src/freebsd/device.c	2016-12-12 19:28:27.292210000 +0100
@@ -334,7 +334,7 @@ devq_device_get_pciid_full_from_fd(int f
 	 */
 	sprintf(sysctl_name, "hw.dri.%d.busid", dev);
 
-	busid_format = "pci:%d:%d:%d.%d";
+	busid_format = "pci:%x:%x:%x.%x";
 	sysctl_value_len = sizeof(sysctl_value);
 	memset(sysctl_value, 0, sysctl_value_len);
 	ret = sysctlbyname(sysctl_name, sysctl_value, &sysctl_value_len,
