//===-- llvm/CFG.h - CFG definitions and useful classes ----------*- C++ -*--=//
//
// This file contains the class definitions useful for operating on the control
// flow graph.
//
// Currently it contains functionality for these three applications:
//
//  1. Iterate over the predecessors of a basic block:
//     pred_iterator, pred_const_iterator, pred_begin, pred_end
//  2. Iterate over the successors of a basic block:
//     succ_iterator, succ_const_iterator, succ_begin, succ_end
//  3. Iterate over the basic blocks of a method in depth first ordering or 
//     reverse depth first order.  df_iterator, df_const_iterator, 
//     df_begin, df_end.  df_begin takes an arg to specify reverse or not.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_CFG_H
#define LLVM_CFG_H

#include <set>
#include <stack>
#include "llvm/Method.h"
#include "llvm/BasicBlock.h"
#include "llvm/InstrTypes.h"

//===----------------------------------------------------------------------===//
//                                Interface
//===----------------------------------------------------------------------===//

//===--------------------------------------------------------------------===//
// Predecessor iterator code
//===--------------------------------------------------------------------===//
// 
// This is used to figure out what basic blocks we could be coming from.
//

// Forward declare iterator class template...
template <class _Ptr, class _USE_iterator> class PredIterator;

typedef PredIterator<BasicBlock*, BasicBlock::use_iterator> pred_iterator;
typedef PredIterator<const BasicBlock*, 
		     BasicBlock::use_const_iterator> pred_const_iterator;

inline pred_iterator       pred_begin(      BasicBlock *BB);
inline pred_const_iterator pred_begin(const BasicBlock *BB);
inline pred_iterator       pred_end  (      BasicBlock *BB);
inline pred_const_iterator pred_end  (const BasicBlock *BB);


//===--------------------------------------------------------------------===//
// Successor iterator code
//===--------------------------------------------------------------------===//
// 
// This is used to figure out what basic blocks we could be going to...
//

// Forward declare iterator class template...
template <class _Term, class _BB> class SuccIterator;

typedef SuccIterator<TerminatorInst*, BasicBlock*> succ_iterator;
typedef SuccIterator<const TerminatorInst*, 
		     const BasicBlock*> succ_const_iterator;

inline succ_iterator       succ_begin(      BasicBlock *BB);
inline succ_const_iterator succ_begin(const BasicBlock *BB);
inline succ_iterator       succ_end  (      BasicBlock *BB);
inline succ_const_iterator succ_end  (const BasicBlock *BB);


//===--------------------------------------------------------------------===//
// Depth First CFG iterator code
//===--------------------------------------------------------------------===//
// 
// This is used to visit basic blocks in a method in either depth first, or 
// reverse depth first ordering, depending on the value passed to the df_begin
// method.
//

// Forward declare iterator class template...
template<class BBType, class SuccItTy> class DFIterator;

typedef DFIterator<BasicBlock, succ_iterator> df_iterator;
typedef DFIterator<const BasicBlock, 
		   succ_const_iterator> df_const_iterator;

inline df_iterator       df_begin(      Method *BB, bool Reverse = false);
inline df_const_iterator df_begin(const Method *BB, bool Reverse = false);
inline df_iterator       df_end  (      Method *BB);
inline df_const_iterator df_end  (const Method *BB);

inline df_iterator       df_begin(      BasicBlock *BB, bool Reverse = false);
inline df_const_iterator df_begin(const BasicBlock *BB, bool Reverse = false);
inline df_iterator       df_end  (      BasicBlock *BB);
inline df_const_iterator df_end  (const BasicBlock *BB);



//===----------------------------------------------------------------------===//
//                                Implementation
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Basic Block Predecessor Iterator
//

template <class _Ptr,  class _USE_iterator> // Predecessor Iterator
class PredIterator {
  const _Ptr BB;
  _USE_iterator It;
public:
  typedef PredIterator<_Ptr,_USE_iterator> _Self;
  
  typedef bidirectional_iterator_tag iterator_category;
  typedef _Ptr pointer;
  
  inline void advancePastConstPool() {
    // Loop to ignore constant pool references
    while (It != BB->use_end() && 
	   (((*It)->getValueType() != Value::InstructionVal) ||
	    !(((Instruction*)(*It))->isTerminator())))
      ++It;
  }
  
  inline PredIterator(_Ptr bb) : BB(bb), It(bb->use_begin()) {
    advancePastConstPool();
  }
  inline PredIterator(_Ptr bb, bool) : BB(bb), It(bb->use_end()) {}
  
  inline bool operator==(const _Self& x) const { return It == x.It; }
  inline bool operator!=(const _Self& x) const { return !operator==(x); }
  
  inline pointer operator*() const { 
    assert ((*It)->getValueType() == Value::InstructionVal);
    return ((Instruction *)(*It))->getParent(); 
  }
  inline pointer *operator->() const { return &(operator*()); }
  
  inline _Self& operator++() {   // Preincrement
    ++It; advancePastConstPool();
    return *this; 
  }
  
  inline _Self operator++(int) { // Postincrement
    _Self tmp = *this; ++*this; return tmp; 
  }
  
  inline _Self& operator--() { --It; return *this; }  // Predecrement
  inline _Self operator--(int) { // Postdecrement
    _Self tmp = *this; --*this; return tmp;
  }
};

inline pred_iterator       pred_begin(      BasicBlock *BB) {
  return pred_iterator(BB);
}
inline pred_const_iterator pred_begin(const BasicBlock *BB) {
  return pred_const_iterator(BB);
}
inline pred_iterator       pred_end(      BasicBlock *BB) {
  return pred_iterator(BB,true);
}
inline pred_const_iterator pred_end(const BasicBlock *BB) {
  return pred_const_iterator(BB,true);
}


//===----------------------------------------------------------------------===//
// Basic Block Successor Iterator
//

template <class _Term, class _BB>           // Successor Iterator
class SuccIterator {
  const _Term Term;
  unsigned idx;
public:
  typedef SuccIterator<_Term, _BB> _Self;
  typedef forward_iterator_tag iterator_category;
  typedef _BB pointer;
  
  inline SuccIterator(_Term T) : Term(T), idx(0) {}         // begin iterator
  inline SuccIterator(_Term T, bool) 
    : Term(T), idx(Term->getNumSuccessors()) {}             // end iterator
  
  inline bool operator==(const _Self& x) const { return idx == x.idx; }
  inline bool operator!=(const _Self& x) const { return !operator==(x); }
  
  inline pointer operator*() const { return Term->getSuccessor(idx); }
  inline pointer *operator->() const { return &(operator*()); }
  
  inline _Self& operator++() { ++idx; return *this; } // Preincrement
  inline _Self operator++(int) { // Postincrement
    _Self tmp = *this; ++*this; return tmp; 
  }
  
  inline _Self& operator--() { --idx; return *this; }  // Predecrement
  inline _Self operator--(int) { // Postdecrement
    _Self tmp = *this; --*this; return tmp;
  }
};

inline succ_iterator       succ_begin(      BasicBlock *BB) {
  return succ_iterator(BB->getTerminator());
}
inline succ_const_iterator succ_begin(const BasicBlock *BB) {
  return succ_const_iterator(BB->getTerminator());
}
inline succ_iterator       succ_end(      BasicBlock *BB) {
  return succ_iterator(BB->getTerminator(),true);
}
inline succ_const_iterator succ_end(const BasicBlock *BB) {
  return succ_const_iterator(BB->getTerminator(),true);
}


//===----------------------------------------------------------------------===//
// Depth First Iterator
//

template<class BBType, class SuccItTy>
class DFIterator {            // BasicBlock Depth First Iterator
  set<BBType *>   Visited;    // All of the blocks visited so far...
  // VisitStack - Used to maintain the ordering.  Top = current block
  // First element is basic block pointer, second is the 'next child' to visit
  stack<pair<BBType *, SuccItTy> > VisitStack;
  const bool Reverse;         // Iterate over children before self?
public:
  typedef DFIterator<BBType, SuccItTy> _Self;

  typedef forward_iterator_tag iterator_category;
  typedef BBType *pointer;

  inline DFIterator(BBType *BB, bool reverse) : Reverse(reverse) {
    Visited.insert(BB);
    VisitStack.push(make_pair(BB, succ_begin(BB)));
    if (Reverse) reverseEnterNode();
  }
  inline DFIterator() { /* End is when stack is empty */ }

  inline bool operator==(const _Self& x) const { 
    return VisitStack == x.VisitStack;
  }
  inline bool operator!=(const _Self& x) const { return !operator==(x); }

  inline pointer operator*() const { 
    return VisitStack.top().first;
  }

  // This is a nonstandard operator-> that dereferences the pointer an extra
  // time... so that you can actually call methods ON the BasicBlock, because
  // the contained type is a pointer.  This allows BBIt->getTerminator() f.e.
  //
  inline BBType *operator->() const { return operator*(); }

  void reverseEnterNode() {
    pair<BBType *, SuccItTy> &Top = VisitStack.top();
    BBType *BB    = Top.first;
    SuccItTy &It  = Top.second;
    for (; It != succ_end(BB); ++It) {
      BBType *Child = *It;
      if (!Visited.count(Child)) {
	Visited.insert(Child);
	VisitStack.push(make_pair(Child, succ_begin(Child)));
	reverseEnterNode();
	return;
      }
    }
  }

  inline _Self& operator++() {   // Preincrement
    if (Reverse) {               // Reverse Depth First Iterator
      if (VisitStack.top().second == succ_end(VisitStack.top().first))
	VisitStack.pop();
      if (!VisitStack.empty())
	reverseEnterNode();
    } else {                     // Normal Depth First Iterator
      do {
	pair<BBType *, SuccItTy> &Top = VisitStack.top();
	BBType *BB    = Top.first;
	SuccItTy &It  = Top.second;

	while (It != succ_end(BB)) {
	  BBType *Next = *It++;
	  if (!Visited.count(Next)) {  // Has our next sibling been visited?
	    // No, do it now.
	    Visited.insert(Next);
	    VisitStack.push(make_pair(Next, succ_begin(Next)));
	    return *this;
	  }
	}
	
	// Oops, ran out of successors... go up a level on the stack.
	VisitStack.pop();
      } while (!VisitStack.empty());
    }
    return *this; 
  }

  inline _Self operator++(int) { // Postincrement
    _Self tmp = *this; ++*this; return tmp; 
  }
};

inline df_iterator df_begin(Method *M, bool Reverse = false) {
  return df_iterator(M->getBasicBlocks().front(), Reverse);
}

inline df_const_iterator df_begin(const Method *M, bool Reverse = false) {
  return df_const_iterator(M->getBasicBlocks().front(), Reverse);
}
inline df_iterator       df_end(Method*) { 
  return df_iterator(); 
}
inline df_const_iterator df_end(const Method*) {
  return df_const_iterator();
}

inline df_iterator df_begin(BasicBlock *BB, bool Reverse = false) { 
  return df_iterator(BB, Reverse);
}
inline df_const_iterator df_begin(const BasicBlock *BB, bool Reverse = false) { 
  return df_const_iterator(BB, Reverse);
}

inline df_iterator       df_end(BasicBlock*) { 
  return df_iterator(); 
}
inline df_const_iterator df_end(const BasicBlock*) {
  return df_const_iterator();
}

#endif
