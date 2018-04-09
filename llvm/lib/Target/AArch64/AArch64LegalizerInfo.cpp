//===- AArch64LegalizerInfo.cpp ----------------------------------*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
/// \file
/// This file implements the targeting of the Machinelegalizer class for
/// AArch64.
/// \todo This should be generated by TableGen.
//===----------------------------------------------------------------------===//

#include "AArch64LegalizerInfo.h"
#include "AArch64Subtarget.h"
#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/TargetOpcodes.h"
#include "llvm/CodeGen/ValueTypes.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Type.h"

using namespace llvm;
using namespace LegalizeActions;
using namespace LegalityPredicates;

AArch64LegalizerInfo::AArch64LegalizerInfo(const AArch64Subtarget &ST) {
  using namespace TargetOpcode;
  const LLT p0 = LLT::pointer(0, 64);
  const LLT s1 = LLT::scalar(1);
  const LLT s8 = LLT::scalar(8);
  const LLT s16 = LLT::scalar(16);
  const LLT s32 = LLT::scalar(32);
  const LLT s64 = LLT::scalar(64);
  const LLT s128 = LLT::scalar(128);
  const LLT s256 = LLT::scalar(256);
  const LLT s512 = LLT::scalar(512);
  const LLT v16s8 = LLT::vector(16, 8);
  const LLT v8s8 = LLT::vector(8, 8);
  const LLT v4s8 = LLT::vector(4, 8);
  const LLT v8s16 = LLT::vector(8, 16);
  const LLT v4s16 = LLT::vector(4, 16);
  const LLT v2s16 = LLT::vector(2, 16);
  const LLT v2s32 = LLT::vector(2, 32);
  const LLT v4s32 = LLT::vector(4, 32);
  const LLT v2s64 = LLT::vector(2, 64);

  getActionDefinitionsBuilder(G_IMPLICIT_DEF)
      .legalFor({p0, s1, s8, s16, s32, s64})
      .clampScalar(0, s1, s64)
      .widenScalarToNextPow2(0, 8);

  getActionDefinitionsBuilder(G_PHI)
      .legalFor({p0, s16, s32, s64})
      .clampScalar(0, s16, s64)
      .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder(G_BSWAP)
      .legalFor({s32, s64})
      .clampScalar(0, s16, s64)
      .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder({G_ADD, G_SUB, G_MUL, G_AND, G_OR, G_XOR, G_SHL})
      .legalFor({s32, s64, v2s32, v4s32, v2s64})
      .clampScalar(0, s32, s64)
      .widenScalarToNextPow2(0)
      .clampNumElements(0, v2s32, v4s32)
      .clampNumElements(0, v2s64, v2s64)
      .moreElementsToNextPow2(0);

  getActionDefinitionsBuilder(G_GEP)
      .legalFor({{p0, s64}})
      .clampScalar(1, s64, s64);

  getActionDefinitionsBuilder(G_PTR_MASK).legalFor({p0});

  getActionDefinitionsBuilder({G_LSHR, G_ASHR, G_SDIV, G_UDIV})
      .legalFor({s32, s64})
      .clampScalar(0, s32, s64)
      .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder({G_SREM, G_UREM})
      .lowerFor({s1, s8, s16, s32, s64});

  getActionDefinitionsBuilder({G_SMULO, G_UMULO})
      .lowerFor({{s64, s1}});

  getActionDefinitionsBuilder({G_SMULH, G_UMULH}).legalFor({s32, s64});

  getActionDefinitionsBuilder({G_UADDE, G_USUBE, G_SADDO, G_SSUBO})
      .legalFor({{s32, s1}, {s64, s1}});

  getActionDefinitionsBuilder({G_FADD, G_FSUB, G_FMA, G_FMUL, G_FDIV})
      .legalFor({s32, s64});

  getActionDefinitionsBuilder({G_FREM, G_FPOW}).libcallFor({s32, s64});

  getActionDefinitionsBuilder(G_INSERT)
      .unsupportedIf([=](const LegalityQuery &Query) {
        return Query.Types[0].getSizeInBits() <= Query.Types[1].getSizeInBits();
      })
      .legalIf([=](const LegalityQuery &Query) {
        const LLT &Ty0 = Query.Types[0];
        const LLT &Ty1 = Query.Types[1];
        if (Ty0 != s32 && Ty0 != s64 && Ty0 != p0)
          return false;
        return isPowerOf2_32(Ty1.getSizeInBits()) &&
               (Ty1.getSizeInBits() == 1 || Ty1.getSizeInBits() >= 8);
      })
      .clampScalar(0, s32, s64)
      .widenScalarToNextPow2(0)
      .maxScalarIf(typeInSet(0, {s32}), 1, s16)
      .maxScalarIf(typeInSet(0, {s64}), 1, s32)
      .widenScalarToNextPow2(1);

  getActionDefinitionsBuilder(G_EXTRACT)
      .unsupportedIf([=](const LegalityQuery &Query) {
        return Query.Types[0].getSizeInBits() >= Query.Types[1].getSizeInBits();
      })
      .legalIf([=](const LegalityQuery &Query) {
        const LLT &Ty0 = Query.Types[0];
        const LLT &Ty1 = Query.Types[1];
        if (Ty1 != s32 && Ty1 != s64)
          return false;
        if (Ty1 == p0)
          return true;
        return isPowerOf2_32(Ty0.getSizeInBits()) &&
               (Ty0.getSizeInBits() == 1 || Ty0.getSizeInBits() >= 8);
      })
      .clampScalar(1, s32, s64)
      .widenScalarToNextPow2(1)
      .maxScalarIf(typeInSet(1, {s32}), 0, s16)
      .maxScalarIf(typeInSet(1, {s64}), 0, s32)
      .widenScalarToNextPow2(0);

  getActionDefinitionsBuilder({G_LOAD, G_STORE})
      .legalFor(
          {{s8, p0}, {s16, p0}, {s32, p0}, {s64, p0}, {p0, p0}, {v2s32, p0}})
      .clampScalar(0, s8, s64)
      .widenScalarToNextPow2(0)
      .clampNumElements(0, v2s32, v2s32);

  // Constants
  getActionDefinitionsBuilder(G_CONSTANT)
      .legalFor({p0, s32, s64})
      .clampScalar(0, s32, s64)
      .widenScalarToNextPow2(0);
  getActionDefinitionsBuilder(G_FCONSTANT)
      .legalFor({s32, s64})
      .clampScalar(0, s32, s64);

  getActionDefinitionsBuilder(G_ICMP)
      .legalFor({{s32, s32}, {s32, s64}, {s32, p0}})
      .clampScalar(0, s32, s32)
      .clampScalar(1, s32, s64)
      .widenScalarToNextPow2(1);

  getActionDefinitionsBuilder(G_FCMP)
      .legalFor({{s32, s32}, {s32, s64}})
      .clampScalar(0, s32, s32)
      .clampScalar(1, s32, s64)
      .widenScalarToNextPow2(1);

  // Extensions
  getActionDefinitionsBuilder({G_ZEXT, G_SEXT, G_ANYEXT})
      .legalFor({s1, s8, s16, s32, s64})
      .maxScalar(0, s64)
      .widenScalarToNextPow2(0);

  // FP conversions
  getActionDefinitionsBuilder(G_FPTRUNC).legalFor(
      {{s16, s32}, {s16, s64}, {s32, s64}});
  getActionDefinitionsBuilder(G_FPEXT).legalFor(
      {{s32, s16}, {s64, s16}, {s64, s32}});

  // Conversions
  getActionDefinitionsBuilder({G_FPTOSI, G_FPTOUI})
      .legalForCartesianProduct({s32, s64})
      .clampScalar(0, s32, s64)
      .widenScalarToNextPow2(0)
      .clampScalar(1, s32, s64)
      .widenScalarToNextPow2(1);

  getActionDefinitionsBuilder({G_SITOFP, G_UITOFP})
      .legalForCartesianProduct({s32, s64})
      .clampScalar(1, s32, s64)
      .widenScalarToNextPow2(1)
      .clampScalar(0, s32, s64)
      .widenScalarToNextPow2(0);

  // Control-flow
  getActionDefinitionsBuilder(G_BRCOND).legalFor({s1, s8, s16, s32});
  getActionDefinitionsBuilder(G_BRINDIRECT).legalFor({p0});

  // Select
  getActionDefinitionsBuilder(G_SELECT)
      .legalFor({{s32, s1}, {s64, s1}, {p0, s1}})
      .clampScalar(0, s32, s64)
      .widenScalarToNextPow2(0);

  // Pointer-handling
  getActionDefinitionsBuilder(G_FRAME_INDEX).legalFor({p0});
  getActionDefinitionsBuilder(G_GLOBAL_VALUE).legalFor({p0});

  getActionDefinitionsBuilder(G_PTRTOINT)
      .legalForCartesianProduct({s1, s8, s16, s32, s64}, {p0})
      .maxScalar(0, s64)
      .widenScalarToNextPow2(0, /*Min*/ 8);

  getActionDefinitionsBuilder(G_INTTOPTR)
      .unsupportedIf([&](const LegalityQuery &Query) {
        return Query.Types[0].getSizeInBits() != Query.Types[1].getSizeInBits();
      })
      .legalFor({s64, p0});

  // Casts for 32 and 64-bit width type are just copies.
  // Same for 128-bit width type, except they are on the FPR bank.
  getActionDefinitionsBuilder(G_BITCAST)
      // FIXME: This is wrong since G_BITCAST is not allowed to change the
      // number of bits but it's what the previous code described and fixing
      // it breaks tests.
      .legalForCartesianProduct({s1, s8, s16, s32, s64, s128, v16s8, v8s8, v4s8,
                                 v8s16, v4s16, v2s16, v4s32, v2s32, v2s64});

  getActionDefinitionsBuilder(G_VASTART).legalFor({p0});

  // va_list must be a pointer, but most sized types are pretty easy to handle
  // as the destination.
  getActionDefinitionsBuilder(G_VAARG)
      .customForCartesianProduct({s8, s16, s32, s64, p0}, {p0})
      .clampScalar(0, s8, s64)
      .widenScalarToNextPow2(0, /*Min*/ 8);

  if (ST.hasLSE()) {
    getActionDefinitionsBuilder(G_ATOMIC_CMPXCHG)
        .legalForCartesianProduct({s8, s16, s32, s64}, {p0});
  }

  if (ST.hasLSE()) {
    getActionDefinitionsBuilder(G_ATOMIC_CMPXCHG_WITH_SUCCESS)
        .lowerFor({s8, s16, s32, s64});

    getActionDefinitionsBuilder(
        {G_ATOMICRMW_XCHG, G_ATOMICRMW_ADD, G_ATOMICRMW_SUB, G_ATOMICRMW_AND,
         G_ATOMICRMW_OR, G_ATOMICRMW_XOR, G_ATOMICRMW_MIN, G_ATOMICRMW_MAX,
         G_ATOMICRMW_UMIN, G_ATOMICRMW_UMAX})
        .legalForCartesianProduct({s8, s16, s32, s64}, {p0});
  }

  // Merge/Unmerge
  for (unsigned Op : {G_MERGE_VALUES, G_UNMERGE_VALUES}) {
    unsigned BigTyIdx = Op == G_MERGE_VALUES ? 0 : 1;
    unsigned LitTyIdx = Op == G_MERGE_VALUES ? 1 : 0;

    auto notValidElt = [](const LegalityQuery &Query, unsigned TypeIdx) {
      const LLT &Ty = Query.Types[TypeIdx];
      if (Ty.isVector()) {
        const LLT &EltTy = Ty.getElementType();
        if (EltTy.getSizeInBits() < 8 || EltTy.getSizeInBits() > 64)
          return true;
        if (!isPowerOf2_32(EltTy.getSizeInBits()))
          return true;
      }
      return false;
    };
    auto scalarize =
        [](const LegalityQuery &Query, unsigned TypeIdx) {
          const LLT &Ty = Query.Types[TypeIdx];
          return std::make_pair(TypeIdx, Ty.getElementType());
        };

    // FIXME: This rule is horrible, but specifies the same as what we had
    // before with the particularly strange definitions removed (e.g.
    // s8 = G_MERGE_VALUES s32, s32).
    // Part of the complexity comes from these ops being extremely flexible. For
    // example, you can build/decompose vectors with it, concatenate vectors,
    // etc. and in addition to this you can also bitcast with it at the same
    // time. We've been considering breaking it up into multiple ops to make it
    // more manageable throughout the backend.
    getActionDefinitionsBuilder(Op)
        // Break up vectors with weird elements into scalars
        .fewerElementsIf(
            [=](const LegalityQuery &Query) { return notValidElt(Query, 0); },
            [=](const LegalityQuery &Query) { return scalarize(Query, 0); })
        .fewerElementsIf(
            [=](const LegalityQuery &Query) { return notValidElt(Query, 1); },
            [=](const LegalityQuery &Query) { return scalarize(Query, 1); })
        // Clamp the big scalar to s8-s512 and make it either a power of 2, 192,
        // or 384.
        .clampScalar(BigTyIdx, s8, s512)
        .widenScalarIf(
            [=](const LegalityQuery &Query) {
              const LLT &Ty = Query.Types[BigTyIdx];
              return !isPowerOf2_32(Ty.getSizeInBits()) &&
                     Ty.getSizeInBits() % 64 != 0;
            },
            [=](const LegalityQuery &Query) {
              // Pick the next power of 2, or a multiple of 64 over 128.
              // Whichever is smaller.
              const LLT &Ty = Query.Types[BigTyIdx];
              unsigned NewSizeInBits = 1
                                       << Log2_32_Ceil(Ty.getSizeInBits() + 1);
              if (NewSizeInBits >= 256) {
                unsigned RoundedTo = alignTo<64>(Ty.getSizeInBits() + 1);
                if (RoundedTo < NewSizeInBits)
                  NewSizeInBits = RoundedTo;
              }
              return std::make_pair(BigTyIdx, LLT::scalar(NewSizeInBits));
            })
        // Clamp the little scalar to s8-s256 and make it a power of 2. It's not
        // worth considering the multiples of 64 since 2*192 and 2*384 are not
        // valid.
        .clampScalar(LitTyIdx, s8, s256)
        .widenScalarToNextPow2(LitTyIdx, /*Min*/ 8)
        // So at this point, we have s8, s16, s32, s64, s128, s192, s256, s384,
        // s512, <X x s8>, <X x s16>, <X x s32>, or <X x s64>.
        // At this point it's simple enough to accept the legal types.
        .legalIf([=](const LegalityQuery &Query) {
          const LLT &BigTy = Query.Types[BigTyIdx];
          const LLT &LitTy = Query.Types[LitTyIdx];
          if (BigTy.isVector() && BigTy.getSizeInBits() < 32)
            return false;
          if (LitTy.isVector() && LitTy.getSizeInBits() < 32)
            return false;
          return BigTy.getSizeInBits() % LitTy.getSizeInBits() == 0;
        })
        // Any vectors left are the wrong size. Scalarize them.
        .fewerElementsIf([](const LegalityQuery &Query) { return true; },
                         [](const LegalityQuery &Query) {
                           return std::make_pair(
                               0, Query.Types[0].getElementType());
                         })
        .fewerElementsIf([](const LegalityQuery &Query) { return true; },
                         [](const LegalityQuery &Query) {
                           return std::make_pair(
                               1, Query.Types[1].getElementType());
                         });
  }

  computeTables();
}

bool AArch64LegalizerInfo::legalizeCustom(MachineInstr &MI,
                                          MachineRegisterInfo &MRI,
                                          MachineIRBuilder &MIRBuilder) const {
  switch (MI.getOpcode()) {
  default:
    // No idea what to do.
    return false;
  case TargetOpcode::G_VAARG:
    return legalizeVaArg(MI, MRI, MIRBuilder);
  }

  llvm_unreachable("expected switch to return");
}

bool AArch64LegalizerInfo::legalizeVaArg(MachineInstr &MI,
                                         MachineRegisterInfo &MRI,
                                         MachineIRBuilder &MIRBuilder) const {
  MIRBuilder.setInstr(MI);
  MachineFunction &MF = MIRBuilder.getMF();
  unsigned Align = MI.getOperand(2).getImm();
  unsigned Dst = MI.getOperand(0).getReg();
  unsigned ListPtr = MI.getOperand(1).getReg();

  LLT PtrTy = MRI.getType(ListPtr);
  LLT IntPtrTy = LLT::scalar(PtrTy.getSizeInBits());

  const unsigned PtrSize = PtrTy.getSizeInBits() / 8;
  unsigned List = MRI.createGenericVirtualRegister(PtrTy);
  MIRBuilder.buildLoad(
      List, ListPtr,
      *MF.getMachineMemOperand(MachinePointerInfo(), MachineMemOperand::MOLoad,
                               PtrSize, /* Align = */ PtrSize));

  unsigned DstPtr;
  if (Align > PtrSize) {
    // Realign the list to the actual required alignment.
    auto AlignMinus1 = MIRBuilder.buildConstant(IntPtrTy, Align - 1);

    unsigned ListTmp = MRI.createGenericVirtualRegister(PtrTy);
    MIRBuilder.buildGEP(ListTmp, List, AlignMinus1->getOperand(0).getReg());

    DstPtr = MRI.createGenericVirtualRegister(PtrTy);
    MIRBuilder.buildPtrMask(DstPtr, ListTmp, Log2_64(Align));
  } else
    DstPtr = List;

  uint64_t ValSize = MRI.getType(Dst).getSizeInBits() / 8;
  MIRBuilder.buildLoad(
      Dst, DstPtr,
      *MF.getMachineMemOperand(MachinePointerInfo(), MachineMemOperand::MOLoad,
                               ValSize, std::max(Align, PtrSize)));

  unsigned SizeReg = MRI.createGenericVirtualRegister(IntPtrTy);
  MIRBuilder.buildConstant(SizeReg, alignTo(ValSize, PtrSize));

  unsigned NewList = MRI.createGenericVirtualRegister(PtrTy);
  MIRBuilder.buildGEP(NewList, DstPtr, SizeReg);

  MIRBuilder.buildStore(
      NewList, ListPtr,
      *MF.getMachineMemOperand(MachinePointerInfo(), MachineMemOperand::MOStore,
                               PtrSize, /* Align = */ PtrSize));

  MI.eraseFromParent();
  return true;
}
