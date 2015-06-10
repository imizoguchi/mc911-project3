#include "Liveness.h"

using namespace llvm;

namespace {

	struct DCELiveness : public FunctionPass {
		static char ID;
		DCELiveness() : FunctionPass(ID) {}

		virtual bool runOnFunction(Function &F) {
			Liveness &L = getAnalysis<Liveness>();

			bool changed = false;

			// Create a queue with all elements that are dead-code
			std::queue < inst_iterator > removeQueue;
			for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i) {
				if(!L.isLiveOut(&*i)) {
					removeQueue.push(i);
				}
			}

			while(!removeQueue.empty())
			{
				changed = true;
				inst_iterator it = removeQueue.front();
				(it)->eraseFromParent();
				removeQueue.pop();
			}

			return changed;
		}


		virtual void getAnalysisUsage(AnalysisUsage &AU) const {
			AU.addRequired<Liveness>();
		}
	};
}

char DCELiveness::ID = 1;

static RegisterPass<DCELiveness> X("dce-liveness", "Dead-code Elimination with Liveness Analysis", false, false);