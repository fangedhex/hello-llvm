// STD Includes
#include <memory>

// LLVM Includes
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/ExecutionEngine/ExecutionEngine.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/ExecutionEngine/Orc/ObjectLinkingLayer.h>
#include <llvm/ExecutionEngine/Orc/IRCompileLayer.h>
#include <llvm/ExecutionEngine/GenericValue.h>

typedef llvm::orc::ObjectLinkingLayer ObjLayerT;
typedef llvm::orc::IRCompileLayer CompileLayerT;

int main(int argc, char const *argv[])
{
  llvm::LLVMContext context;
  llvm::Module module("top", context);
  llvm::IRBuilder<> builder(context);

  auto funcType = llvm::FunctionType::get(builder.getInt32Ty(), false);
  auto mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, "main", module);

  llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
  builder.SetInsertPoint(entry);

  llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");

  std::vector<llvm::Type *> putsArgs;
  putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
  llvm::ArrayRef<llvm::Type *> argsRef(putsArgs);

  auto putsType = llvm::FunctionType::get(builder.getInt32Ty(), argsRef, false);
  auto putsFunc = module.getOrInsertFunction("puts", putsType);

  builder.CreateCall(putsFunc, helloWorld);
  builder.CreateRetVoid();

  module.print(llvm::errs(), nullptr);

  // Initilaze native target
  llvm::InitializeNativeTarget();
  llvm::InitializeNativeTargetAsmPrinter();
  llvm::InitializeNativeTargetAsmParser();

  std::unique_ptr<llvm::Module> ptrModule(&module);
  llvm::EngineBuilder engineBuilder(std::move(ptrModule));
  engineBuilder.setEngineKind(llvm::EngineKind::Interpreter);

  std::string err;
  engineBuilder.setErrorStr(&err);
  auto executionEngine = engineBuilder.create();
  executionEngine->runStaticConstructorsDestructors(false);
  auto main = executionEngine->FindFunctionNamed("main");
  if (!main)
    return EXIT_FAILURE;
  executionEngine->runFunction(main, llvm::ArrayRef<llvm::GenericValue>());

  return EXIT_SUCCESS;
}
