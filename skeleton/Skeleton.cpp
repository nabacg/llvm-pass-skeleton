#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace {

struct SkeletonPass : public PassInfoMixin<SkeletonPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            errs() << "Function " << F.getName()  << "!\n";
            errs() << "Return type: " << *(F.getReturnType()) << "\n";
            errs() << "Number of arguments: " << F.arg_size() << "\n";
            errs() << "Is declaration: " << (F.isDeclaration() ? "yes" : "no") << "\n";
            errs() << "Is vararg: " << (F.isVarArg() ? "yes" : "no") << "\n";
            errs() << "Calling convention: " << F.getCallingConv() << "\n";
            // errs() << "Attributes: " << F.getAttributes() << "\n";
            if (!F.isDeclaration()) {
                errs() << "Function body:\n" << F << "\n";
            }
        }
        return PreservedAnalyses::all();
    };
};

struct BasicBlockPrinterPass : public PassInfoMixin<BasicBlockPrinterPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            errs() << "Function body:" << F  << "!\n";
            for (auto &B : F) { 
                errs() << "Basic Block:\n" << B << "\n";
                for(auto &I : B) {
                    errs() << "Instruction: " << I << "\n";
                }
            }
        }
        return PreservedAnalyses::all();
    };
};


struct BinaryOpReplacerPass : public PassInfoMixin<BasicBlockPrinterPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            for (auto &B : F) { 
                for(auto &I : B) {
                    // if instruction is a BinaryOperator using https://llvm.org/docs/ProgrammersManual.html#isa
                    if(auto* op = dyn_cast<BinaryOperator>(&I)) {
                        // Insert at the point where instruction `op` appears.
                        IRBuilder<> builder(op);

                        //use the same operands as `op`
                        Value* lhs = op->getOperand(0);
                        Value* rhs = op->getOperand(1);
                        Value* mulOp = builder.CreateMul(lhs, rhs);

                        //scan the uses of `op` and everywhere it was use, 
                        // we want to replace that with new `mulOp`
                        for (auto &U : op->uses()) {
                            User* user = U.getUser(); // User is anything with operands
                            user->setOperand(U.getOperandNo(), mulOp); // replace current operand with `mulOp`
                        }

                        // we modified the code, so we need return this 
                        return PreservedAnalyses::none();
                    }
                }
            }
        }
        return PreservedAnalyses::all();
    };
};


struct BinaryOpResultPrinterPass : public PassInfoMixin<BasicBlockPrinterPass> {
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        for (auto &F : M) {
            LLVMContext& Ctx = F.getContext();
            FunctionCallee logFunc = F.getParent()->getOrInsertFunction("logBinOp", 
                Type::getVoidTy(Ctx),
                Type::getInt32Ty(Ctx)
                );

            for (auto &B : F) { 
                for(auto &I : B) {
                    // call a runtime function ()
                    if (auto* op = dyn_cast<BinaryOperator>(&I)) {
                        //Insert call AFTER current `op`
                        IRBuilder<> builder(op);
                        builder.SetInsertPoint(&B, ++builder.GetInsertPoint()); // will increment current point (where `op` was)
                        
                        // Insert actuall call to "logBinOp"
                        Value* args[] = {op};
                        builder.CreateCall(logFunc, args);

                        return PreservedAnalyses::none();
                    }
                }
            }
        }
        return PreservedAnalyses::all();
    };
};


}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "Skeleton pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    // MPM.addPass(SkeletonPass());
                    // MPM.addPass(BasicBlockPrinterPass());
                    // MPM.addPass(BinaryOpReplacerPass());
                    MPM.addPass(BinaryOpResultPrinterPass());
                });
        }
    };
}
