// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_LINKEDLIST_H_
#define STP_BASE_CONTAINERS_LINKEDLIST_H_

#include "Base/Debug/Assert.h"

namespace stp {

template<typename T>
class LinkedList;

template<typename T>
class LinkedListNode {
 public:
  constexpr LinkedListNode() : prev_(nullptr), next_(nullptr) {}

  constexpr LinkedListNode(LinkedListNode* prev, LinkedListNode* next)
      : prev_(prev), next_(next) {}

  // Insert |this| into the linked list, before |e|.
  void InsertBefore(LinkedListNode<T>* e);

  // Insert |this| into the linked list, after |e|.
  void InsertAfter(LinkedListNode<T>* e);

  // Remove |this| from the linked list.
  void RemoveFromList();

  ALWAYS_INLINE LinkedListNode* prev() const { return prev_; }
  ALWAYS_INLINE LinkedListNode* next() const { return next_; }

 private:
  friend class LinkedList<T>;

  ALWAYS_INLINE const T* that() const { return static_cast<const T*>(this); }
  ALWAYS_INLINE T* that() { return static_cast<T*>(this); }

  LinkedListNode* prev_;
  LinkedListNode* next_;

  DISALLOW_COPY_AND_ASSIGN(LinkedListNode);
};

// Represents a circular, doubly linked list.
template<typename T>
class LinkedList {
 public:
  using NodeType = LinkedListNode<T>;

  // The "root" node is self-referential, and forms the basis of a circular
  // list (root_.next() will point back to the start of the list,
  // and root_.prev() wraps around to the end of the list).
  constexpr LinkedList() : root_(root(), root()) {}
  ~LinkedList() { Clear(); }

  ALWAYS_INLINE NodeType* root() const { return const_cast<NodeType*>(&root_); }

  ALWAYS_INLINE bool IsEmpty() const { return root_.next() == root(); }

  void Clear();

  // Unlike clear it does not invalidates next/prev pointers of items.
  // Useful in cases when all items were already deleted.
  void Reset() { root_ = NodeType(root(), root()); }

  T* GetFirst() const;
  T* GetLast() const;

  void InsertBefore(NodeType* before, T* e) { e->InsertBefore(before); }
  void InsertAfter(NodeType* after, T* e) { e->InsertAfter(after); }

  void Append(T* e) { InsertBefore(root(), e); }
  void Prepend(T* e) { InsertAfter(root(), e); }

  void Remove(T* e) { e->RemoveFromList(); }

  void RemoveFirst() { Remove(GetFirst()); }
  void RemoveLast() { Remove(GetLast()); }

  bool Contains(const T& value) const { return Find(value) != nullptr; }

  T* Find(const T& value) const;
  T* FindLast(const T& value) const;

  // Really slow, that's because it's not named size().
  int Count() const;

  class Iterator {
   public:
    explicit Iterator(NodeType* ptr) : ptr_(ptr) {}

    T& operator*() const { return *ptr_->that(); }
    T* operator->() const { return ptr_->that(); }

    Iterator& operator++() {
      ptr_ = ptr_->next();
      return *this;
    }

    T* get() const { return ptr_->that(); }

    bool operator==(const Iterator& other) const { return ptr_ == other.ptr_; }
    bool operator!=(const Iterator& other) const { return !operator==(other); }

   private:
    NodeType* ptr_;
  };

  // Only for range-based for-loop.
  ALWAYS_INLINE Iterator begin() const { return Iterator(root_.next()); }
  ALWAYS_INLINE Iterator end() const { return Iterator(const_cast<NodeType*>(&root_)); }

  friend void Swap(LinkedList& lhs, LinkedList& rhs) {
    Swap(lhs.root_, rhs.root_);
  }

 private:
  NodeType root_;

  DISALLOW_COPY_AND_ASSIGN(LinkedList);
};

template<typename T>
inline void LinkedListNode<T>::InsertBefore(LinkedListNode<T>* e) {
  ASSERT(!prev_ && !next_);
  this->next_ = e;
  this->prev_ = e->prev_;
  e->prev_->next_ = this;
  e->prev_ = this;
}

template<typename T>
inline void LinkedListNode<T>::InsertAfter(LinkedListNode<T>* e) {
  ASSERT(!prev_ && !next_);
  this->next_ = e->next_;
  this->prev_ = e;
  e->next_->prev_ = this;
  e->next_ = this;
}

template<typename T>
inline void LinkedListNode<T>::RemoveFromList() {
  ASSERT(prev_ && next_);
  this->prev_->next_ = this->next_;
  this->next_->prev_ = this->prev_;
  this->next_ = nullptr;
  this->prev_ = nullptr;
}

template<typename T>
inline void LinkedList<T>::Clear() {
  while (!IsEmpty())
    GetFirst()->RemoveFromList();
}

template<typename T>
inline T* LinkedList<T>::GetFirst() const {
  ASSERT(!IsEmpty());
  return root_.next()->that();
}

template<typename T>
inline T* LinkedList<T>::GetLast() const {
  ASSERT(!IsEmpty());
  return root_.prev()->that();
}

template<typename T>
inline T* LinkedList<T>::Find(const T& value) const {
  for (NodeType* n = root_.next(), e = root(); n != e; n = n->next()) {
    if (*n->that() == value)
      return n->that();
  }
  return nullptr;
}

template<typename T>
inline T* LinkedList<T>::FindLast(const T& value) const {
  for (NodeType* n = root_.prev(), e = root(); n != e; n = n->prev()) {
    if (*n->that() == value)
      return n->that();
  }
  return nullptr;
}

template<typename T>
inline int LinkedList<T>::Count() const {
  int n = 0;
  for (auto* it = root_.next(); it != &root_; ++it)
    ++n;
  return n;
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_LINKEDLIST_H_