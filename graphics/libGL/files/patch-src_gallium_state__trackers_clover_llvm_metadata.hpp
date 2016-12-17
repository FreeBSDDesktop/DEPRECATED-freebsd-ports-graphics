--- src/gallium/state_trackers/clover/llvm/metadata.hpp.orig	2016-11-23 14:20:50.588478000 +0100
+++ src/gallium/state_trackers/clover/llvm/metadata.hpp	2016-11-23 14:22:45.224851000 +0100
@@ -42,7 +42,7 @@ namespace clover {
          get_kernel_nodes(const ::llvm::Module &mod) {
             if (const ::llvm::NamedMDNode *n =
                    mod.getNamedMetadata("opencl.kernels"))
-               return { n->op_begin(), n->op_end() };
+               return { n->getOperand(0), n->getOperand(n->getNumOperands()) };
             else
                return {};
          }
