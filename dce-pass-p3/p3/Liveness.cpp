#include  "Liveness.h"
#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

/* This implementation is based on the example given by the instructor at:
 * http://www.ic.unicamp.br/~maxiwell/cursos/mc911/proj3/Liveness.cpp
 *
 * Methods that are used "as is" in the example are marked.
 */

// NOT MODIFIED
void Liveness::addToMap(Function &F) {
  static int id = 1;
  for (inst_iterator i = inst_begin(F), E = inst_end(F); i != E; ++i, ++id){
    instMap.insert(std::make_pair(&*i, id));
  }
}

bool Liveness::isLiveOut(Instruction *i)
{
  beforeAfter s = iBAMap.lookup(&*i);
  return (
  s.after.count(&*i) ||
  i->mayHaveSideEffects() ||
  isa<TerminatorInst>(&*i) ||
  isa<DbgInfoIntrinsic>(&*i) ||
  isa<LandingPadInst>(&*i) );
}

void Liveness::printValueSet(std::set<const Instruction *> *s) {
  errs() << "[";
  for (std::set<const Instruction *>::iterator i = s->begin(), e = s->end(); i != e; ++i) {
    errs() << (**i).getName() << ", ";
  }
  errs() << "]";
}

void Liveness::printInAndOut(Function &F) {

  errs() << "Func: " << F.getName() << '\n';

  for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
    errs() << b->getName() << '\n';
    
    for (BasicBlock::iterator i = b->begin(), ie = b->end(); i != ie; ++i) {
      beforeAfter bf = iBAMap.lookup(i);
      errs() << "IN ";
      printValueSet(&(bf.before));
      errs() << "\n" << *i;
      errs() << "\nOUT ";
      printValueSet(&(bf.after));
      errs() << "\n";
    }
  }
}

// NOT MODIFIED
void Liveness::computeBBGenKill(Function &F) 
{
  for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
    genKill s;
    for (BasicBlock::iterator i = b->begin(), e = b->end(); i != e; ++i) {
      // The GEN set is the set of upwards-exposed uses:
      // pseudo-registers that are used in the block before being
      // defined. (Those will be the pseudo-registers that are defined
      // in other blocks, or are defined in the current block and used
      // in a phi function at the start of this block.) 
      unsigned n = i->getNumOperands();
      for (unsigned j = 0; j < n; j++) {
        Value *v = i->getOperand(j);
        if (isa<Instruction>(v)) {
          Instruction *op = cast<Instruction>(v);
        if (!s.kill.count(op))
          s.gen.insert(op);
        }
      }
      // For the KILL set, you can use the set of all instructions
      // that are in the block (which safely includes all of the
      // pseudo-registers assigned to in the block).
      s.kill.insert(&*i);

    }
    bbGKMap.insert(std::make_pair(&*b, s));
  }
}

// Using book's algorithm 10.4
void Liveness::computeBBBeforeAfter(Function &F){

  bool changed = true;
  while(changed) {

    changed = false;

    for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {     

      beforeAfter ba = bbBAMap.lookup(&*b);
      beforeAfter oldBA;

      oldBA.after = ba.after;
      oldBA.before = ba.before;

      genKill gk = bbGKMap.lookup(&*b);

      /* Will calculate before (IN) set as (after - KILL) + GEN
       * equivalent to IN = (OUT - DEF) + USE
       */
      ba.before.clear();

      // Perform: after - KILL
      std::set_difference(ba.after.begin(), ba.after.end(), gk.kill.begin(), gk.kill.end(),
                std::inserter(ba.before, ba.before.end()));

      // Includes GEN
      ba.before.insert(gk.gen.begin(), gk.gen.end());

      // after set (OUT) is the union of all successors before sets (INs)
      std::set<const Instruction*> a;

      // For each successor
      for (succ_iterator SI = succ_begin(b), E = succ_end(b); SI != E; ++SI) {
        std::set<const Instruction*> s(bbBAMap.lookup(*SI).before);
        a.insert(s.begin(), s.end());
      }

      ba.after = a;

      // Continue until no changes occur in before & after sets
      if(!(ba.after == oldBA.after && ba.before == oldBA.before)){ 
        changed = true;
      }

      // update
      bbBAMap.erase(&*b);
      bbBAMap.insert(std::make_pair(&*b, ba));
    }
  }
}

// NOT MODIFIED
void Liveness::computeIBeforeAfter(Function &F)
{
  for (Function::iterator b = F.begin(), e = F.end(); b != e; ++b) {
    BasicBlock::iterator i = --b->end();
    std::set<const Instruction*> liveAfter(bbBAMap.lookup(b).after);
    std::set<const Instruction*> liveBefore(liveAfter);

    while (true) {
      // before = after - KILL + GEN
      liveBefore.erase(i);

      unsigned n = i->getNumOperands();
      for (unsigned j = 0; j < n; j++) {
        Value *v = i->getOperand(j);
        if (isa<Instruction>(v))
          liveBefore.insert(cast<Instruction>(v));
      }

      beforeAfter ba;
      ba.before = liveBefore;
      ba.after = liveAfter;
      iBAMap.insert(std::make_pair(&*i, ba));

      liveAfter = liveBefore;
      if (i == b->begin())
        break;
      --i;
    }
  }
}

bool Liveness::runOnFunction(Function &F) {

  addToMap(F);

  computeBBGenKill(F);
  computeBBBeforeAfter(F);
  computeIBeforeAfter(F);

  return false;
}

char Liveness::ID = 0;
RegisterPass<Liveness> X("liveness", "Liveness Pass", false, true);