--- lib/CodeGen/AsmPrinter/AsmPrinter.cpp.orig	2016-12-24 07:44:35 UTC
+++ lib/CodeGen/AsmPrinter/AsmPrinter.cpp
@@ -114,7 +114,7 @@ AsmPrinter::AsmPrinter(TargetMachine &tm
 }
 
 AsmPrinter::~AsmPrinter() {
-  assert(!DD && Handlers.empty() && "Debug/EH info didn't get finalized");
+//  assert(!DD && Handlers.empty() && "Debug/EH info didn't get finalized");
 
   if (GCMetadataPrinters) {
     gcp_map_type &GCMap = getGCMap(GCMetadataPrinters);
