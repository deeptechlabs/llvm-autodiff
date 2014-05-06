//===-- Globals.cpp - Implement the GlobalValue & GlobalVariable class ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the GlobalValue & GlobalVariable classes for the IR
// library.
//
//===----------------------------------------------------------------------===//

#include "llvm/IR/GlobalValue.h"
#include "llvm/ADT/SmallPtrSet.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/GlobalAlias.h"
#include "llvm/IR/GlobalVariable.h"
#include "llvm/IR/LeakDetector.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/ErrorHandling.h"
using namespace llvm;

//===----------------------------------------------------------------------===//
//                            GlobalValue Class
//===----------------------------------------------------------------------===//

bool GlobalValue::isMaterializable() const {
  return getParent() && getParent()->isMaterializable(this);
}
bool GlobalValue::isDematerializable() const {
  return getParent() && getParent()->isDematerializable(this);
}
bool GlobalValue::Materialize(std::string *ErrInfo) {
  return getParent()->Materialize(this, ErrInfo);
}
void GlobalValue::Dematerialize() {
  getParent()->Dematerialize(this);
}

const DataLayout *GlobalValue::getDataLayout() const {
  return getParent()->getDataLayout();
}

/// Override destroyConstant to make sure it doesn't get called on
/// GlobalValue's because they shouldn't be treated like other constants.
void GlobalValue::destroyConstant() {
  llvm_unreachable("You can't GV->destroyConstant()!");
}

/// copyAttributesFrom - copy all additional attributes (those not needed to
/// create a GlobalValue) from the GlobalValue Src to this one.
void GlobalValue::copyAttributesFrom(const GlobalValue *Src) {
  if (!isa<GlobalAlias>(this)) {
    setAlignment(Src->getAlignment());
    setSection(Src->getSection());
  }

  setVisibility(Src->getVisibility());
  setUnnamedAddr(Src->hasUnnamedAddr());
  setDLLStorageClass(Src->getDLLStorageClass());
}

unsigned GlobalValue::getAlignment() const {
  if (auto *GA = dyn_cast<GlobalAlias>(this))
    return GA->getAliasedGlobal()->getAlignment();

  return (1u << Alignment) >> 1;
}

void GlobalValue::setAlignment(unsigned Align) {
  assert((!isa<GlobalAlias>(this)) &&
         "GlobalAlias should not have an alignment!");
  assert((Align & (Align-1)) == 0 && "Alignment is not a power of 2!");
  assert(Align <= MaximumAlignment &&
         "Alignment is greater than MaximumAlignment!");
  Alignment = Log2_32(Align) + 1;
  assert(getAlignment() == Align && "Alignment representation error!");
}

void GlobalValue::setSection(StringRef S) {
  assert(!isa<GlobalAlias>(this) && "GlobalAlias should not have a section!");
  Section = S;
}

bool GlobalValue::isDeclaration() const {
  // Globals are definitions if they have an initializer.
  if (const GlobalVariable *GV = dyn_cast<GlobalVariable>(this))
    return GV->getNumOperands() == 0;

  // Functions are definitions if they have a body.
  if (const Function *F = dyn_cast<Function>(this))
    return F->empty();

  // Aliases are always definitions.
  assert(isa<GlobalAlias>(this));
  return false;
}
  
//===----------------------------------------------------------------------===//
// GlobalVariable Implementation
//===----------------------------------------------------------------------===//

GlobalVariable::GlobalVariable(Type *Ty, bool constant, LinkageTypes Link,
                               Constant *InitVal,
                               const Twine &Name, ThreadLocalMode TLMode,
                               unsigned AddressSpace,
                               bool isExternallyInitialized)
  : GlobalValue(PointerType::get(Ty, AddressSpace),
                Value::GlobalVariableVal,
                OperandTraits<GlobalVariable>::op_begin(this),
                InitVal != nullptr, Link, Name),
    isConstantGlobal(constant), threadLocalMode(TLMode),
    isExternallyInitializedConstant(isExternallyInitialized) {
  if (InitVal) {
    assert(InitVal->getType() == Ty &&
           "Initializer should be the same type as the GlobalVariable!");
    Op<0>() = InitVal;
  }

  LeakDetector::addGarbageObject(this);
}

GlobalVariable::GlobalVariable(Module &M, Type *Ty, bool constant,
                               LinkageTypes Link, Constant *InitVal,
                               const Twine &Name,
                               GlobalVariable *Before, ThreadLocalMode TLMode,
                               unsigned AddressSpace,
                               bool isExternallyInitialized)
  : GlobalValue(PointerType::get(Ty, AddressSpace),
                Value::GlobalVariableVal,
                OperandTraits<GlobalVariable>::op_begin(this),
                InitVal != nullptr, Link, Name),
    isConstantGlobal(constant), threadLocalMode(TLMode),
    isExternallyInitializedConstant(isExternallyInitialized) {
  if (InitVal) {
    assert(InitVal->getType() == Ty &&
           "Initializer should be the same type as the GlobalVariable!");
    Op<0>() = InitVal;
  }
  
  LeakDetector::addGarbageObject(this);
  
  if (Before)
    Before->getParent()->getGlobalList().insert(Before, this);
  else
    M.getGlobalList().push_back(this);
}

void GlobalVariable::setParent(Module *parent) {
  if (getParent())
    LeakDetector::addGarbageObject(this);
  Parent = parent;
  if (getParent())
    LeakDetector::removeGarbageObject(this);
}

void GlobalVariable::removeFromParent() {
  getParent()->getGlobalList().remove(this);
}

void GlobalVariable::eraseFromParent() {
  getParent()->getGlobalList().erase(this);
}

void GlobalVariable::replaceUsesOfWithOnConstant(Value *From, Value *To,
                                                 Use *U) {
  // If you call this, then you better know this GVar has a constant
  // initializer worth replacing. Enforce that here.
  assert(getNumOperands() == 1 &&
         "Attempt to replace uses of Constants on a GVar with no initializer");

  // And, since you know it has an initializer, the From value better be
  // the initializer :)
  assert(getOperand(0) == From &&
         "Attempt to replace wrong constant initializer in GVar");

  // And, you better have a constant for the replacement value
  assert(isa<Constant>(To) &&
         "Attempt to replace GVar initializer with non-constant");

  // Okay, preconditions out of the way, replace the constant initializer.
  this->setOperand(0, cast<Constant>(To));
}

void GlobalVariable::setInitializer(Constant *InitVal) {
  if (!InitVal) {
    if (hasInitializer()) {
      Op<0>().set(nullptr);
      NumOperands = 0;
    }
  } else {
    assert(InitVal->getType() == getType()->getElementType() &&
           "Initializer type must match GlobalVariable type");
    if (!hasInitializer())
      NumOperands = 1;
    Op<0>().set(InitVal);
  }
}

/// copyAttributesFrom - copy all additional attributes (those not needed to
/// create a GlobalVariable) from the GlobalVariable Src to this one.
void GlobalVariable::copyAttributesFrom(const GlobalValue *Src) {
  assert(isa<GlobalVariable>(Src) && "Expected a GlobalVariable!");
  GlobalValue::copyAttributesFrom(Src);
  const GlobalVariable *SrcVar = cast<GlobalVariable>(Src);
  setThreadLocalMode(SrcVar->getThreadLocalMode());
}


//===----------------------------------------------------------------------===//
// GlobalAlias Implementation
//===----------------------------------------------------------------------===//

GlobalAlias::GlobalAlias(Type *Ty, LinkageTypes Link,
                         const Twine &Name, Constant* aliasee,
                         Module *ParentModule)
  : GlobalValue(Ty, Value::GlobalAliasVal, &Op<0>(), 1, Link, Name) {
  LeakDetector::addGarbageObject(this);

  if (aliasee)
    assert(aliasee->getType() == Ty && "Alias and aliasee types should match!");
  Op<0>() = aliasee;

  if (ParentModule)
    ParentModule->getAliasList().push_back(this);
}

void GlobalAlias::setParent(Module *parent) {
  if (getParent())
    LeakDetector::addGarbageObject(this);
  Parent = parent;
  if (getParent())
    LeakDetector::removeGarbageObject(this);
}

void GlobalAlias::removeFromParent() {
  getParent()->getAliasList().remove(this);
}

void GlobalAlias::eraseFromParent() {
  getParent()->getAliasList().erase(this);
}

void GlobalAlias::setAliasee(Constant *Aliasee) {
  assert((!Aliasee || Aliasee->getType() == getType()) &&
         "Alias and aliasee types should match!");
  
  setOperand(0, Aliasee);
}

static GlobalValue *getAliaseeGV(GlobalAlias *GA) {
  Constant *C = GA->getAliasee();
  assert(C && "Must alias something");

  if (GlobalValue *GV = dyn_cast<GlobalValue>(C))
    return GV;

  ConstantExpr *CE = cast<ConstantExpr>(C);
  assert((CE->getOpcode() == Instruction::BitCast ||
          CE->getOpcode() == Instruction::AddrSpaceCast ||
          CE->getOpcode() == Instruction::GetElementPtr) &&
         "Unsupported aliasee");

  return cast<GlobalValue>(CE->getOperand(0));
}

GlobalValue *GlobalAlias::getAliasedGlobal() {
  SmallPtrSet<GlobalValue*, 3> Visited;

  GlobalAlias *GA = this;

  for (;;) {
    GlobalValue *GV = getAliaseeGV(GA);
    if (!Visited.insert(GV))
      return nullptr;

    // Iterate over aliasing chain.
    GA = dyn_cast<GlobalAlias>(GV);
    if (!GA)
      return GV;
  }
}
