--- configure.ac.orig	2016-11-10 23:06:40.000000000 +0100
+++ configure.ac	2016-12-10 21:27:43.157568000 +0100
@@ -910,7 +910,7 @@
 AC_SUBST(LIBSENSORS_LDFLAGS)
 
 case "$host_os" in
-linux*)
+linux*|freebsd*)
     dri3_default=yes
     ;;
 *)
@@ -1559,9 +1559,24 @@
             ;;
         esac
         ;;
-    *freebsd* | dragonfly* | *netbsd* | openbsd*)
+    dragonfly* | *netbsd* | openbsd*)
         DEFINES="$DEFINES -DHAVE_ALIAS"
         ;;
+    *freebsd*)
+        DEFINES="$DEFINES -DHAVE_ALIAS"
+        if test "x$enable_dri3" = xyes; then
+            DEFINES="$DEFINES -DHAVE_DRI3"
+        fi
+
+        case "$host_cpu" in
+        powerpc* | sparc*)
+            # Build only the drivers for cards that exist on PowerPC/sparc
+            if test "x$with_dri_drivers" = "xyes"; then
+                with_dri_drivers="r200 radeon swrast"
+            fi
+            ;;
+        esac
+        ;;
     gnu*)
         DEFINES="$DEFINES -DHAVE_ALIAS"
         ;;
@@ -1945,6 +1960,14 @@
         AC_MSG_ERROR([cannot enable OpenCL without Gallium])
     fi
 
+    if test "x$acv_mesa_CLANG" = xno; then
+        GCC_VERSION=`$CC -dumpversion`
+        if test $? -eq 0; then
+            GCC_VERSION_MAJOR=`echo $GCC_VERSION | cut -d. -f1`
+            GCC_VERSION_MINOR=`echo $GCC_VERSION | cut -d. -f2`
+        fi
+    fi
+
     if test $GCC_VERSION_MAJOR -lt 4 -o $GCC_VERSION_MAJOR -eq 4 -a $GCC_VERSION_MINOR -lt 7; then
         AC_MSG_ERROR([gcc >= 4.7 is required to build clover])
     fi
@@ -2121,9 +2144,9 @@
 strip_unwanted_llvm_flags() {
     # Use \> (marks the end of the word)
     echo `$1` | sed \
-	-e 's/-march=\S*//g' \
-	-e 's/-mtune=\S*//g' \
-	-e 's/-mcpu=\S*//g' \
+	-e 's/-march=[[:graph:]]* //g' \
+	-e 's/-mtune=[[:graph:]]* //g' \
+	-e 's/-mcpu=[[:graph:]]* //g' \
 	-e 's/-DNDEBUG\>//g' \
 	-e 's/-D_GNU_SOURCE\>//g' \
 	-e 's/-pedantic\>//g' \
@@ -2219,8 +2242,6 @@
                 CLANG_LIBDIR=${LLVM_LIBDIR}
             fi
             CLANG_RESOURCE_DIR=$CLANG_LIBDIR/clang/${LLVM_VERSION}
-            AS_IF([test ! -f "$CLANG_RESOURCE_DIR/include/stddef.h"],
-                [AC_MSG_ERROR([Could not find clang internal header stddef.h in $CLANG_RESOURCE_DIR Use --with-clang-libdir to specify the correct path to the clang libraries.])])
         fi
     else
         MESA_LLVM=0
