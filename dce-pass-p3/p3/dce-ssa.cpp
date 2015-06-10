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
#include "llvm/ADT/SmallVector.h"
#include "llvm/IR/CFG.h"

#define DEBUG 0

using namespace llvm;

namespace {

	struct DCESSA : public FunctionPass {
		static char ID;
		DCESSA() : FunctionPass(ID) {}

		virtual bool runOnFunction(Function &F) {

			bool changed = false;

			std::set<const Value*> instructionSet;
			
			for(inst_iterator I = inst_begin(F), E = inst_end(F); I != E; ++I) {

				// errs() << "\nInstr: " << *I << "\n";
				// for (const User *U : (&*I)->users()) {
				// 	errs() << "\tAdd USED to set: " << *U << "\n";
				// }
				if(I->use_empty() && !I->isTerminator())  {
					instructionSet.insert(&*I);
					if(DEBUG) errs() << "\tAdd to set: " << *I << "\n";
				}

				// for(Value::use_iterator i = I->use_begin(), ie = I->use_end(); i!=ie; ++i) {
				// 	Use *use = &*i;
				// 	Value *value = use->get();
				// 	errs() << "\tAdd USE to set: " << *value << "\n";
				// 	instructionSet.insert(value);
				// }
			}

			while(!instructionSet.empty()) {
				// errs() << "\nWITH EMPTY SET and 1st:\n" << **(instructionSet.begin()) << "\n";
				const Value *value = &**(instructionSet.begin());
				bool empty = true;

				// Remove value from set of variables
				instructionSet.erase(instructionSet.begin());

				// Iterate over all users of value
				// to check if it's used by someone
				for (const User *U : value->users()) {
					empty = false;
					break;
				}

				if(DEBUG) errs() << "\tPopping from set with " << empty << ": " << *value << "\n";

  				// If not used
				if(empty) {
					if (Instruction *S = (Instruction*)value) {
						if(!S->mayHaveSideEffects()) {
							if(DEBUG) errs() << "\t\tTrying to remove:\t" << *S << "\n";
							if(DEBUG) errs() << "\t\tDoesn't have side effects\n";

							// Iterate over all variables used
							// by value that was removed
							for(Value::use_iterator it = S->use_begin(), ie = S->use_end(); it!=ie; ++it) {
								// Remove S from the list of uses of usedValue
								Use *use = &*it;
								Value *usedValue = use->get();

								if(DEBUG) errs() << "\t\tadding to set:\t" << *usedValue << "\n";

								// add usedValue to variableSet
								// instructionSet.insert(usedValue);
							}
							S->eraseFromParent();
							changed = true;
						}
					}
				}
			}
			return changed;
		}
	};
}

char DCESSA::ID = 1;

static RegisterPass<DCESSA> X("dce-ssa", "Dead-code Elimination with SSA", false, false);