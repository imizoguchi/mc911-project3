#include "llvm/Pass.h"
#include "llvm/IR/IntrinsicInst.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instruction.h"
#include "llvm/ADT/StringExtras.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/InstIterator.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/IR/User.h"
#include "llvm/IR/Instructions.h"
#include <set>
#include <queue>
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/CFG.h"

using namespace llvm;

namespace llvm {

class genKill {
public:
std::set<const Instruction*> gen;
std::set<const Instruction*> kill;
};

class beforeAfter {
public:
std::set<const Instruction*> before;
std::set<const Instruction*> after;
};

class Liveness: public FunctionPass {
private:

public:

    DenseMap<const BasicBlock*, genKill> bbGKMap;
    DenseMap<const BasicBlock*, beforeAfter> bbBAMap;
    DenseMap<const Instruction*, beforeAfter> iBAMap;
    DenseMap<const Instruction*, int> instMap;

	static char ID; 
	Liveness() : FunctionPass(ID) {}

	virtual bool runOnFunction(Function &F);


    void computeBBGenKill(Function &F);
    void computeBBBeforeAfter(Function &F);
    void computeIBeforeAfter(Function &F);
    bool isLiveOut(Instruction *i);
    void addToMap(Function &F);

    void printInAndOut(Function &F);
    void printValueSet(std::set<const Instruction *> *s);

	virtual void getAnalysisUsage(AnalysisUsage &AU) const{
		AU.setPreservesAll();
	}



};

}


