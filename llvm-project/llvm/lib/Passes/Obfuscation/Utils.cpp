/**
 * @file Utils.cpp
 * @author SsageParuders
 * @brief 本代码参考原OLLVM项目:https://github.com/obfuscator-llvm/obfuscator
 *        感谢地球人前辈的指点
 * @version 0.1
 * @date 2022-07-14
 *
 * @copyright Copyright (c) 2022
 *
 */
#include "Utils.h"
#include "llvm/IR/IntrinsicInst.h"

using namespace llvm;
using std::vector;

LLVMContext *CONTEXT = nullptr;

/**
 * @brief 参考资料:https://www.jianshu.com/p/0567346fd5e8
 *        作用是读取llvm.global.annotations中的annotation值 从而实现过滤函数 只对单独某功能开启PASS
 * @param f
 * @return std::string
 */
std::string llvm::readAnnotate(Function *f){
    std::string annotation = "";
    /* Get annotation variable */
    GlobalVariable *glob=f->getParent()->getGlobalVariable( "llvm.global.annotations" );
    if ( glob != NULL ){
        /* Get the array */
        if ( ConstantArray * ca = dyn_cast<ConstantArray>( glob->getInitializer() ) ){
            for ( unsigned i = 0; i < ca->getNumOperands(); ++i ){
                /* Get the struct */
                if ( ConstantStruct * structAn = dyn_cast<ConstantStruct>( ca->getOperand( i ) ) ){
                    if (Function *target = dyn_cast<Function>(
                        structAn->getOperand(0)->stripPointerCasts())) {
                        //llvm::errs() << "Found ca[" + i + "] funcname: " + target->getName() + "\n";
                        if (target->getName() == f->getName()) {
                            if (GlobalVariable *annoVar = dyn_cast<GlobalVariable>( structAn->getOperand(1) ->stripPointerCasts())) {
                                if (ConstantDataSequential *data = dyn_cast<ConstantDataSequential>( annoVar->getInitializer())) {
                                    if (data->isString()) {
                                        //llvm::errs() << "  " + data->getAsString() + "\n";
                                        annotation += data->getAsString().str();
                                    }
                                }
                            }
                        
                        }
                    }
                }
            }
        }
    }
    return(annotation);
}


/**
 * @brief 用于判断是否开启混淆
 * 
 * @param flag 
 * @param f 
 * @param attribute 
 * @return true 
 * @return false 
 */
bool llvm::toObfuscate(bool flag, Function *f, std::string const &attribute) { //取自原版ollvm项目
    std::string attr = attribute;
    std::string attrNo = "no" + attr;
    // Check if declaration
    if (f->isDeclaration()) {
        return false;
    }
    // Check external linkage
    if (f->hasAvailableExternallyLinkage() != 0) {
        return false;
    }
    // We have to check the nofla flag first
    // Because .find("fla") is true for a string like "fla" or
    // "nofla"
    if (readAnnotate(f).find(attrNo) != std::string::npos || f->hasFnAttribute(attrNo)) { // 是否禁止开启XXX
        llvm::errs() << "[OLLVM] " + f->getName() + " disable(" + attr + ")\n";
        return false;
    }
    // If fla annotations
    if (readAnnotate(f).find(attr) != std::string::npos ||  f->hasFnAttribute(attr)) { // 是否开启XXX
        llvm::errs() << "[OLLVM] " + f->getName() + " enable(" + attr + ")\n";
        return true;
    }
    // If fla flag is set
    if (flag == true) { // 开启PASS
        return true;
    }
    return false;
}

/**
 * @brief 修复PHI指令和逃逸变量
 * 
 * @param F 
 */
void llvm::fixStack(Function &F) {
    vector<PHINode*> origPHI;
    vector<Instruction*> origReg;
    BasicBlock &entryBB = F.getEntryBlock();
    for(BasicBlock &BB : F){
        for(Instruction &I : BB){
            if(PHINode *PN = dyn_cast<PHINode>(&I)){
                origPHI.push_back(PN);
            }else if(!(isa<AllocaInst>(&I) && I.getParent() == &entryBB) 
                && I.isUsedOutsideOfBlock(&BB)){
                origReg.push_back(&I);
            }
        }
    }
    for(PHINode *PN : origPHI){
        DemotePHIToStack(PN, entryBB.getTerminator());
    }
    for(Instruction *I : origReg){
        DemoteRegToStack(*I, entryBB.getTerminator());
    }
}

/**
 * @brief 
 * 
 * @param Func 
 */
void llvm::FixFunctionConstantExpr(Function *Func) {
  // Replace ConstantExpr with equal instructions
  // Otherwise replacing on Constant will crash the compiler
  for (BasicBlock &BB : *Func) {
    FixBasicBlockConstantExpr(&BB);
  }
}
/**
 * @brief 
 * 
 * @param BB 
 */
void llvm::FixBasicBlockConstantExpr(BasicBlock *BB) {
  // Replace ConstantExpr with equal instructions
  // Otherwise replacing on Constant will crash the compiler
  // Things to note:
  // - Phis must be placed at BB start so CEs must be placed prior to current BB
  assert(!BB->empty() && "BasicBlock is empty!");
  assert((BB->getParent() != NULL) && "BasicBlock must be in a Function!");
  Instruction *FunctionInsertPt = &*(BB->getParent()->getEntryBlock().getFirstInsertionPt());
  // Instruction* LocalBBInsertPt=&*(BB.getFirstInsertionPt());
  for (Instruction &I : *BB) {
    if (isa<LandingPadInst>(I) || isa<FuncletPadInst>(I)) {
        continue;
    }
    for (unsigned i = 0; i < I.getNumOperands(); i++) {
      if (ConstantExpr *C = dyn_cast<ConstantExpr>(I.getOperand(i))) {
        Instruction *InsertPt = &I;
        IRBuilder<NoFolder> IRB(InsertPt);
        if (isa<PHINode>(I)) {
          IRB.SetInsertPoint(FunctionInsertPt);
        }
        Instruction *Inst = IRB.Insert(C->getAsInstruction());
        I.setOperand(i, Inst);
      }
    }
  }
}

/**
 * @brief 随机字符串
 * 
 * @param len 
 * @return string 
 */
string llvm::rand_str(int len){
    string str;
    char c = 'O';
    int idx;
    for (idx = 0; idx < len; idx++){
        
        switch ((rand() % 3)){
            case 1:
                c = 'O';
                break;
            case 2:
                c = '0';
                break;
            default:
                c = 'o';
                break;
		}
        str.push_back(c);
    }
    return str;
}

void llvm::LowerConstantExpr(Function &F) {
    SmallPtrSet<Instruction *, 8> WorkList;

    for (inst_iterator It = inst_begin(F), E = inst_end(F); It != E; ++It) {
        Instruction *I = &*It;

        if (isa<LandingPadInst>(I) || isa<CatchPadInst>(I) ||
            isa<CatchSwitchInst>(I) || isa<CatchReturnInst>(I))
                continue;
        if (auto *II = dyn_cast<IntrinsicInst>(I)) {
                if (II->getIntrinsicID() == Intrinsic::eh_typeid_for) {
        continue;
                }
        }

        for (unsigned int i = 0; i < I->getNumOperands(); ++i) {
                if (isa<ConstantExpr>(I->getOperand(i)))
        WorkList.insert(I);
        }
    }

    while (!WorkList.empty()) {
        auto It = WorkList.begin();
        Instruction *I = *It;
        WorkList.erase(*It);

        if (PHINode *PHI = dyn_cast<PHINode>(I)) {
                for (unsigned int i = 0; i < PHI->getNumIncomingValues(); ++i) {
        Instruction *TI = PHI->getIncomingBlock(i)->getTerminator();
        if (ConstantExpr *CE =
                dyn_cast<ConstantExpr>(PHI->getIncomingValue(i))) {
          Instruction *NewInst = CE->getAsInstruction();
          NewInst->insertBefore(TI);
          PHI->setIncomingValue(i, NewInst);
          WorkList.insert(NewInst);
        }
                }
        } else {
                for (unsigned int i = 0; i < I->getNumOperands(); ++i) {
        if (ConstantExpr *CE = dyn_cast<ConstantExpr>(I->getOperand(i))) {
          Instruction *NewInst = CE->getAsInstruction();
          NewInst->insertBefore(I);
          I->replaceUsesOfWith(CE, NewInst);
          WorkList.insert(NewInst);
        }
                }
        }
    }
}