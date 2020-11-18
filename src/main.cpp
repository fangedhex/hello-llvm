// STD Includes
#include <memory>

// LLVM Includes
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>

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

  return 0;
}
