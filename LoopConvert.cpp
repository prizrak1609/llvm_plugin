#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Process.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Signals.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Bitcode/BitcodeWriter.h"

using namespace llvm;

cl::opt<std::string> InputFile(cl::desc("<input bitcode>"), cl::Positional, cl::init("-"));
cl::opt<std::string> OutputFile(cl::desc("output file"), cl::ConsumeAfter, cl::init("a.out"));

int main(int argc, char** argv) {
	PrettyStackTraceProgram X(argc, argv);

	LLVMContext context;

	cl::ParseCommandLineOptions(argc, argv, "llvm interpreter & dynamic compiler\n");

	// Disable core file
	sys::Process::PreventCoreFiles();

	SMDiagnostic err;
	auto module = std::unique_ptr<Module>(parseIRFile(InputFile, err, context));
	if (!module) {
		err.print(argv[0], errs());
		std::exit(1);
	}

	if (auto errCode = module->materializeAll()) {
		errs() << argv[0] << ": bitcode didn't read correctly.\n";
		// errs() << "Reason: " << errCode.message() << "\n";
		std::exit(1);
	}

	auto& functionList = module->getFunctionList();

	// rename sensitive function
	for (Function& func : functionList) {
		if (func.getName() == "_Z13checkSecurityv") {
			func.setName(Twine("renamed"));
		}
	}

	// write changed IR to OutputFile file
	std::error_code EC;
  	llvm::raw_fd_ostream OS(OutputFile, EC, llvm::sys::fs::F_None);
  	WriteBitcodeToFile(*module, OS);
  	OS.flush();

	return 0;
}