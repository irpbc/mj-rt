#include "compiler.h"

#include <llvm/IR/LegacyPassManager.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/CodeGen/GCs.h>

using namespace llvm;

extern "C" {

EXPORT void AddRewriteStatepointsForGCPass(LLVMPassManagerRef passManagerRef) {
    ModulePass* P = createRewriteStatepointsForGCPass();
    unwrap(passManagerRef)->add(P);

	linkStatepointExampleGC();
}

}