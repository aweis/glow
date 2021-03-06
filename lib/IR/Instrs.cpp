/**
 * Copyright (c) 2017-present, Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "glow/IR/Instrs.h"
#include "glow/IR/IR.h"
#include "glow/Support/Support.h"

#include "llvm/Support/Casting.h"

#include <cassert>

using namespace glow;
using llvm::cast;
using llvm::isa;

//===----------------------------------------------------------------------===//
//                      Instruction textual printers
//===----------------------------------------------------------------------===//

const char *WeightVar::getMutabilityStr(MutabilityKind kind) {
  const char *names[] = {"const", "mutable", nullptr};
  return names[static_cast<int>(kind)];
}

const char *WeightVar::getMutabilityStr() const {
  return getMutabilityStr(mut_);
}

void WeightVar::dump(llvm::raw_ostream &os) const {
  os << "%" << getName() << " = WeightVar ";
  os << *getType() << " " << getMutabilityStr();
}

//===----------------------------------------------------------------------===//
//                       Instruction verification
//===----------------------------------------------------------------------===//

void CopyInst::verify() const {
  auto *dest = getDest();
  auto *src = getSrc();
  (void)dest;
  (void)src;
  assert(dest->getType() == src->getType() && "Invalid type.");
  // The operands of the copy instruction must be variables.
  assert(isa<AllocActivationInst>(dest) || isa<WeightVar>(dest) ||
         isa<TensorViewInst>(dest));
  assert(isa<AllocActivationInst>(src) || isa<WeightVar>(src) ||
         isa<TensorViewInst>(src));
}

void TensorViewInst::verify() const {
  assert(getSrc()->getType()->size() >= getType()->size() &&
         "TensorView view size should be no larger than Src size");
  assert(getSrc()->getElementType() == getType()->getElementType() &&
         "TensorView view element type should be the same as Src type");
}

void AllocActivationInst::verify() const {
  unsigned numDealloc = 0;
  for (const Use &U : getUsers()) {
    numDealloc += isa<DeallocActivationInst>(U.get());
  }

  // Make sure that there is exactly one user is a deallocation.
  assert(numDealloc == 1 && "Invalid number of tensor deallocation");
}

void DeallocActivationInst::verify() const {
  // The operand of this instruction needs to be an AllocActivationInst.
  assert(isa<AllocActivationInst>(getSrc()) && "Invalid operand");
}

void InsertTensorInst::verify() const {
  assert(getSrc()->getElementType() == getDest()->getElementType() &&
         "InsertTensor dest element type should be the same as Src type.");
  assert(getCount() > 0 && "Count must be non-zero.");
  assert(getAxis() >= 0 && getAxis() < getDest()->dims().size() &&
         "Axis must fit inside Dest dims.");
}

void QuantizeInst::verify() const {
  assert((getDest()->getElementType() == ElemKind::Int8QTy ||
          getDest()->getElementType() == ElemKind::Int32QTy) &&
         "Invalid type");
  assert((getSrc()->getElementType() == ElemKind::FloatTy ||
          getSrc()->getElementType() == ElemKind::Float16Ty) &&
         "Invalid type");
  assert(getSrc()->dims() == getDest()->dims() && "Invalid shape");
}
