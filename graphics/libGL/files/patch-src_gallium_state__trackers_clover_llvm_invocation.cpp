--- src/gallium/state_trackers/clover/llvm/invocation.cpp.orig	2016-12-22 23:27:09 UTC
+++ src/gallium/state_trackers/clover/llvm/invocation.cpp
@@ -93,6 +93,12 @@ namespace {
       return ctx;
    }
 
+   char* convert(const std::string&s){
+      char *pc = new char[s.size()+1];
+      std::strcpy(pc, s.c_str());
+      return pc;
+   }
+
    std::unique_ptr<clang::CompilerInstance>
    create_compiler_instance(const target &target,
                             const std::vector<std::string> &opts,
@@ -104,8 +110,9 @@ namespace {
       // Parse the compiler options.  A file name should be present at the end
       // and must have the .cl extension in order for the CompilerInvocation
       // class to recognize it as an OpenCL source file.
-      const std::vector<const char *> copts =
-         map(std::mem_fn(&std::string::c_str), opts);
+      std::vector<const char *> copts;
+      std::transform(opts.begin(), opts.end(), std::back_inserter(copts), convert);
+//         map(std::mem_fn(&std::string::c_str), opts);
 
       if (!clang::CompilerInvocation::CreateFromArgs(
              c->getInvocation(), copts.data(), copts.data() + copts.size(), diag))
