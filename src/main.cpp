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
#include <llvm/Support/Host.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/IR/LegacyPassManager.h>

typedef llvm::orc::ObjectLinkingLayer ObjLayerT;
typedef llvm::orc::IRCompileLayer CompileLayerT;

#define MAIN_FUNC "_start"

int main(int argc, char const *argv[])
{
  /**
   * LLVM IR Code generation
   */
  llvm::LLVMContext context;
  llvm::Module module("top", context);
  llvm::IRBuilder<> builder(context);

  auto funcType = llvm::FunctionType::get(builder.getVoidTy(), false);
  auto mainFunc = llvm::Function::Create(funcType, llvm::Function::ExternalLinkage, MAIN_FUNC, module);

  llvm::BasicBlock *entry = llvm::BasicBlock::Create(context, "entrypoint", mainFunc);
  builder.SetInsertPoint(entry);

  llvm::Value *helloWorld = builder.CreateGlobalStringPtr("hello world!\n");

  // We "import" the puts function
  std::vector<llvm::Type *> putsArgs;
  putsArgs.push_back(builder.getInt8Ty()->getPointerTo());
  auto putsType = llvm::FunctionType::get(builder.getInt32Ty(), putsArgs, false);
  auto putsFunc = module.getOrInsertFunction("puts", putsType);

  // We "import" the exit function
  std::vector<llvm::Type *> exitArgs;
  exitArgs.push_back(builder.getInt32Ty());
  auto exitType = llvm::FunctionType::get(builder.getVoidTy(), exitArgs, false);
  auto exitFunc = module.getOrInsertFunction("exit", exitType);

  builder.CreateCall(putsFunc, helloWorld);
  builder.CreateCall(exitFunc, builder.getInt32(0));
  builder.CreateRetVoid();

  module.print(llvm::errs(), nullptr);

  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  /**
   * Running our generated IR code directly here
   */

  /*std::unique_ptr<llvm::Module> ptrModule(&module);
  llvm::EngineBuilder engineBuilder(std::move(ptrModule));
  engineBuilder.setEngineKind(llvm::EngineKind::Interpreter);

  std::string err;
  engineBuilder.setErrorStr(&err);
  auto executionEngine = engineBuilder.create();
  executionEngine->runStaticConstructorsDestructors(false);
  auto main = executionEngine->FindFunctionNamed(MAIN_FUNC);
  if (!main)
    return EXIT_FAILURE;
  executionEngine->runFunction(main, llvm::ArrayRef<llvm::GenericValue>());*/

  /**
   * Generating an object file and linking it through ld
   */

  auto targetTriple = llvm::sys::getDefaultTargetTriple();

  std::string Error;
  auto target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!target)
  {
    llvm::errs() << Error;
    return 1;
  }

  auto CPU = "generic";
  auto Features = "";

  llvm::TargetOptions opt;
  auto RM = llvm::Optional<llvm::Reloc::Model>();
  auto targetMachine = target->createTargetMachine(targetTriple, CPU, Features, opt, RM);

  module.setDataLayout(targetMachine->createDataLayout());
  module.setTargetTriple(targetTriple);

  auto Filename = "output.o";
  std::error_code EC;
  llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

  if (EC)
  {
    llvm::errs() << "Could not open file: " << EC.message();
    return EXIT_FAILURE;
  }

  llvm::legacy::PassManager pass;
  auto FileType = llvm::CGFT_ObjectFile;

  if (targetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType))
  {
    llvm::errs() << "TargetMachine can't emit a file of this type";
    return EXIT_FAILURE;
  }

  pass.run(module);
  dest.flush();

  return EXIT_SUCCESS;
}
