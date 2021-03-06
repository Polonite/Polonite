// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_MEMORY_MALLOCPTR_H_
#define STP_BASE_MEMORY_MALLOCPTR_H_

#include "Base/Debug/Assert.h"
#include "Base/Memory/Allocate.h"
#include "Base/Type/Variable.h"

namespace stp {

template<class T>
class MallocPtr {
  DISALLOW_COPY_AND_ASSIGN(MallocPtr);
 public:
  static_assert(!TIsArray<T>, "C arrays disallowed, use List class instead");

  MallocPtr() = default;
  ~MallocPtr() { if (ptr_) ::free(ptr_); }

  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  MallocPtr(MallocPtr<U>&& u) noexcept : ptr_(u.leakPtr()) {}
  template<class U, TEnableIf<!TIsArray<U> && TIsConvertibleTo<U*, T*>>* = nullptr>
  MallocPtr& operator=(MallocPtr<U>&& u) noexcept { reset(u.leakPtr()); return *this; }

  MallocPtr(nullptr_t) noexcept {}
  MallocPtr& operator=(nullptr_t) noexcept { reset(); return *this; }

  explicit MallocPtr(T* ptr) noexcept : ptr_(ptr) { ASSERT(ptr_ != nullptr); }
  [[nodiscard]] T* leakPtr() noexcept { return exchange(ptr_, nullptr); }

  void reset(T* new_ptr = nullptr) noexcept {
    T* tmp = exchange(ptr_, new_ptr);
    if (tmp)
      ::free(tmp);
  }

  T& operator*() const noexcept { ASSERT(ptr_); return *ptr_; }
  T* operator->() const noexcept { ASSERT(ptr_); return ptr_; }

  ALWAYS_INLINE T* get() const noexcept { return ptr_; }

  explicit operator bool() const noexcept { return ptr_ != nullptr; }

  static MallocPtr create(int size_in_bytes);
  static MallocPtr tryCreate(int size_in_bytes) noexcept;

  friend void swap(MallocPtr& l, MallocPtr& r) noexcept { swap(l.ptr_, r.ptr_); }

  friend bool operator==(const MallocPtr& l, const MallocPtr& r) noexcept { return l.ptr_ == r.ptr_; }
  friend bool operator!=(const MallocPtr& l, const MallocPtr& r) noexcept { return l.ptr_ != r.ptr_; }

  friend bool operator==(const MallocPtr& l, nullptr_t) noexcept { return !l.ptr_; }
  friend bool operator!=(const MallocPtr& l, nullptr_t) noexcept { return !!l.ptr_; }
  friend bool operator==(nullptr_t, const MallocPtr& r) noexcept { return !r.ptr_; }
  friend bool operator!=(nullptr_t, const MallocPtr& r) noexcept { return !!r.ptr_; }

 private:
  T* ptr_ = nullptr;
};

template<class T> struct TIsZeroConstructibleTmpl<MallocPtr<T>> : TTrue {};
template<class T> struct TIsTriviallyRelocatableTmpl<MallocPtr<T>> : TTrue {};
template<class T> struct TIsTriviallyEqualityComparableTmpl<MallocPtr<T>> : TTrue {};

template<class T> inline MallocPtr<T> MallocPtr<T>::create(int size_in_bytes) {
  return (T*)allocateMemory(size_in_bytes);
}

template<class T> inline MallocPtr<T> MallocPtr<T>::tryCreate(int size_in_bytes) noexcept {
  return (T*)tryAllocateMemory(size_in_bytes);
}

// Helper to transfer ownership of a raw pointer to a MallocPtr<T>.
template<class T> inline MallocPtr<T> makeMallocPtr(T* ptr) noexcept {
  return MallocPtr<T>(ptr);
}

} // namespace stp

#endif // STP_BASE_MEMORY_MALLOCPTR_H_
