// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_LIST_H_
#define STP_BASE_CONTAINERS_LIST_H_

#include "Base/Containers/ListFwd.h"
#include "Base/Containers/Span.h"
#include "Base/Mem/Allocate.h"

namespace stp {

template<typename T>
class List {
 public:
  typedef T ItemType;
  typedef Span<T> SpanType;
  typedef MutableSpan<T> MutableSpanType;

  List() = default;
  ~List() { DestroyAndFree(data_, size_, capacity_); }

  List(List&& other) noexcept;
  List& operator=(List&& other) noexcept;

  List(const List& other) { AssignExternal(other); }
  List& operator=(const List& other);

  List(InitializerList<T> ilist) { AssignExternal(SpanType(ilist)); }
  List& operator=(InitializerList<T> ilist);

  explicit List(SpanType span) { AssignExternal(span); }
  List& operator=(SpanType span);

  explicit List(const T* data, int size) { AssignExternal(SpanType(data, size)); }

  operator SpanType() const { return ToSpan(); }
  operator MutableSpanType() { return ToSpan(); }

  ALWAYS_INLINE const T* data() const { return data_; }
  ALWAYS_INLINE T* data() { return data_; }
  ALWAYS_INLINE int size() const { return size_; }
  ALWAYS_INLINE int capacity() const { return capacity_; }

  bool IsEmpty() const { return size_ == 0; }
  void Clear() { Truncate(0); }

  void EnsureCapacity(int request);
  void ShrinkCapacity(int request);
  void ShrinkToFit() { ShrinkCapacity(size_); }

  void WillGrow(int n);

  const T& operator[](int at) const;
  T& operator[](int at);

  const T& GetFirst() const { return ToSpan().GetFirst(); }
  const T& GetLast() const { return ToSpan().GetLast(); }
  T& GetFirst() { return ToSpan().GetFirst(); }
  T& GetLast() { return ToSpan().GetLast(); }

  SpanType GetSlice(int at) const { return ToSpan().GetSlice(at); }
  SpanType GetSlice(int at, int n) const { return ToSpan().GetSlice(at, n); }
  MutableSpanType GetSlice(int at) { return ToSpan().GetSlice(at); }
  MutableSpanType GetSlice(int at, int n) { return ToSpan().GetSlice(at, n); }

  int Add(T item);
  T* AppendUninitialized(int n = 1);
  int AppendInitialized(int n = 1);
  int AddRepeat(T item, int n);
  int Append(SpanType other);

  void Insert(int at, T item);
  T* InsertUninitialized(int at, int n = 1);
  void InsertInitialized(int at, int n = 1);
  void InsertRange(int at, SpanType src);

  void RemoveLast();
  void RemoveAt(int at) { RemoveRange(at, 1); }
  void RemoveRange(int at, int n);

  void Truncate(int at);
  void RemovePrefix(int n) { RemoveRange(0, n); }
  void RemoveSuffix(int n) { Truncate(size_ - n); }

  template<typename U>
  int IndexOf(const U& item) const { return ToSpan().IndexOf(item); }
  template<typename U>
  int LastIndexOf(const U& item) const { return ToSpan().LastIndexOf(item); }
  template<typename U>
  bool Contains(const U& item) const { return ToSpan().Contains(item); }

  static List AdoptMemory(T* ptr, int size, int capacity);
  T* ReleaseMemory();

  bool IsSourceOf(SpanType span) const { return IsSourceOf(span.data()); }

  List& operator+=(T item) { Add(Move(item)); return *this; }
  List& operator+=(SpanType range) { Append(range); return *this; }

  friend void Swap(List& l, List& r) noexcept {
    Swap(l.data_, r.data_);
    Swap(l.size_, r.size_);
    Swap(l.capacity_, r.capacity_);
  }
  friend bool operator==(const List& l, const SpanType& r) { return l.ToSpan() == r; }
  friend bool operator!=(const List& l, const SpanType& r) { return l.ToSpan() != r; }
  friend int Compare(const List& l, const SpanType& r) { return Compare(l.ToSpan(), r); }

  friend const T* begin(const List& x) { return x.data_; }
  friend const T* end(const List& x) { return x.data_ + x.size_; }
  friend T* begin(List& x) { return x.data_; }
  friend T* end(List& x) { return x.data_ + x.size_; }

  friend SpanType MakeSpan(const List& x) { return x.ToSpan(); }
  friend MutableSpanType MakeSpan(List& x) { return x.ToSpan(); }

 private:
  T* data_ = nullptr;
  int size_ = 0;
  int capacity_ = 0;

  static constexpr int MaxCapacity_ = Limits<int>::Max / isizeof(T);
  // Needed by String -> null terminated conversion.
  static constexpr int CapacityIncrement_ = TIsCharacter<T>;

  static void DestroyAndFree(T* data, int size, int capacity) {
    if (data) {
      Destroy(data, size);
      Free(data);
    }
  }

  SpanType ToSpan() const { return SpanType(data_, size_); }
  MutableSpanType ToSpan() { return MutableSpanType(data_, size_); }

  void SetSizeNoGrow(int new_size);

  void AssignExternal(SpanType src);
  void AssignInternal(SpanType src);

  bool IsSourceOf(const T* ptr) const { return data_ <= ptr && ptr < data_ + size_; }

  template<typename TAction>
  int AddMany(int n, TAction&& action);
  template<typename TAction>
  void InsertMany(int at, int n, TAction&& action);

  void ResizeStorage(int new_capacity);

  void CheckGrow(int n) {
    if (MaxCapacity_ - size_ < n)
      throw LengthException();
  }

  int RecommendCapacity(int request) const {
    return (capacity_ < MaxCapacity_ / 2) ? Max(request, capacity_ << 1) : MaxCapacity_;
  }
};

template<typename T>
struct TIsZeroConstructibleTmpl<List<T>> : TTrue {};
template<typename T>
struct TIsTriviallyRelocatableTmpl<List<T>> : TIsTriviallyRelocatableTmpl<T> {};

template<typename T, int N>
inline bool operator==(const T (&lhs)[N], const List<T>& rhs) {
  return operator==(MakeSpan(lhs), MakeSpan(rhs));
}
template<typename T, int N>
inline bool operator!=(const T (&lhs)[N], const List<T>& rhs) {
  return operator!=(MakeSpan(lhs), MakeSpan(rhs));
}
template<typename T, int N>
inline int Compare(const T (&lhs)[N], const List<T>& rhs) {
  return Compare(MakeSpan(lhs), MakeSpan(rhs));
}

template<typename T, TEnableIf<TIsHashable<T>>* = nullptr>
inline HashCode Hash(const List<T>& x) { return HashContiguous(x.data(), x.size()); }

template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline void Format(TextWriter& out, const List<T>& x, const StringSpan& opts) {
  FormatContiguous(out, x.data(), x.size(), opts);
}
template<typename T, TEnableIf<TIsFormattable<T>>* = nullptr>
inline TextWriter& operator<<(TextWriter& out, const List<T>& x) {
  FormatContiguous(out, x.data(), x.size()); return out;
}

template<typename T, TEnableIf<TIsContiguousContainer<TRemoveReference<T>>>* = nullptr>
inline List<typename T::ItemType> MakeList(T&& list) {
  return List<typename T::ItemType>(Forward<T>(list));
}

inline String ToString(String s) { return s; }
inline String ToString(StringSpan s) { return String(s); }

inline String16 ToString16(String16 s) { return s; }
inline String16 ToString16(String16Span s) { return String16(s); }

BASE_EXPORT String ToString(String16Span s);
BASE_EXPORT String16 ToString16(StringSpan s);

BASE_EXPORT const char* ToNullTerminated(const List<char>& string);
BASE_EXPORT const char16_t* ToNullTerminated(const List<char16_t>& string);

#if SIZEOF_WCHAR_T == 2
inline WString ToWString(WString s) { return s; }
inline WString ToWString(WStringSpan s) { return WString(s); }

BASE_EXPORT String ToString(WStringSpan s);
BASE_EXPORT String16 ToString16(WStringSpan s);
BASE_EXPORT WString ToWString(StringSpan s);
BASE_EXPORT WString ToWString(String16Span s);
BASE_EXPORT const wchar_t* ToNullTerminated(const List<wchar_t>& string);
#endif

template<typename T>
inline List<T>::List(List&& other) noexcept
    : data_(Exchange(other.data_, nullptr)),
      size_(Exchange(other.size_, 0)),
      capacity_(Exchange(other.capacity_, 0)) {}

template<typename T>
inline List<T>& List<T>::operator=(List&& other) noexcept {
  DestroyAndFree(data_, size_, capacity_);
  data_ = Exchange(other.data_, nullptr);
  size_ = Exchange(other.size_, 0);
  capacity_ = Exchange(other.capacity_, 0);
  return *this;
}

template<typename T>
inline List<T>& List<T>::operator=(const List& other) {
  if (LIKELY(this != &other)) {
    AssignExternal(other);
  }
  return *this;
}

template<typename T>
inline List<T>& List<T>::operator=(InitializerList<T> ilist) {
  AssignExternal(SpanType(ilist));
  return *this;
}

template<typename T>
inline List<T>& List<T>::operator=(SpanType span) {
  if (TIsTriviallyDestructible<T> || !IsSourceOf(span)) {
    AssignExternal(span);
  } else {
    AssignInternal(span);
  }
  return *this;
}

template<typename T>
inline void List<T>::ResizeStorage(int new_capacity) {
  ASSERT(new_capacity >= 0 && new_capacity != capacity_);

  if (size_ && TIsTriviallyRelocatable<T>) {
    data_ = Reallocate(data_, ToUnsigned(new_capacity + CapacityIncrement_));
    capacity_ = new_capacity;
  } else {
    T* new_data = Allocate<T>(new_capacity + CapacityIncrement_);
    capacity_ = new_capacity;
    T* old_data = Exchange(data_, new_data);
    if (old_data) {
      UninitializedRelocate(new_data, old_data, size_);
      Free(old_data);
    }
  }
}

template<typename T>
inline void List<T>::EnsureCapacity(int request) {
  ASSERT(request >= size_);
  if (request > capacity_) {
    if (request > MaxCapacity_)
      throw LengthException();
    ResizeStorage(request);
  }
}

template<typename T>
inline void List<T>::ShrinkCapacity(int request) {
  ASSERT(size_ <= request);
  if (request >= capacity_)
    return;
  if (request) {
    ResizeStorage(request);
  } else {
    capacity_ = 0;
    Free(Exchange(data_, nullptr));
  }
}

template<typename T>
void List<T>::WillGrow(int n) {
  CheckGrow(n);
  int request = size_ + n;
  if (UNLIKELY(request > capacity_))
    ResizeStorage(RecommendCapacity(request));
}

template<typename T>
inline T& List<T>::operator[](int at) {
  ASSERT(0 <= at && at < size_);
  return *(data_ + at);
}

template<typename T>
inline const T& List<T>::operator[](int at) const {
  ASSERT(0 <= at && at < size_);
  return *(data_ + at);
}

template<typename T>
inline void List<T>::Truncate(int at) {
  ASSERT(0 <= at && at <= size_);
  if (TIsTriviallyDestructible<T>) {
    SetSizeNoGrow(at);
  } else {
    int old_size = size_;
    if (old_size != at) {
      Destroy(data_ + at, old_size - at);
      SetSizeNoGrow(at);
    }
  }
}

template<typename T>
inline int List<T>::Add(T item) {
  int old_size = size_;
  if (UNLIKELY(capacity_ == old_size))
    WillGrow(1);
  new(data_ + old_size) T(Move(item));
  SetSizeNoGrow(old_size + 1);
  return old_size;
}

template<typename T>
inline T* List<T>::AppendUninitialized(int n) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    WillGrow(n);
  SetSizeNoGrow(old_size + n);
  return data_ + old_size;
}

template<typename T>
inline int List<T>::AppendInitialized(int n) {
  return AddMany(n, [](T* dst, int count) { UninitializedInit(dst, count); });
}

template<typename T>
inline int List<T>::AddRepeat(T item, int n) {
  return AddMany(n, [&item](T* dst, int count) { UninitializedFill(dst, count, item); });
}

template<typename T>
inline int List<T>::Append(SpanType src) {
  ASSERT(!IsSourceOf(src));
  const T* src_d = src.data();
  return AddMany(src.size(), [src_d](T* dst, int count) { UninitializedCopy(dst, src_d, count); });
}

template<typename T>
template<typename TAction>
inline int List<T>::AddMany(int n, TAction&& action) {
  ASSERT(n >= 0);
  int old_size = size_;
  if (UNLIKELY(capacity_ - old_size < n))
    WillGrow(n);
  action(data_ + old_size, n);
  SetSizeNoGrow(old_size + n);
  return old_size;
}

template<typename T>
inline void List<T>::RemoveLast() {
  ASSERT(!IsEmpty());
  int new_size = size_ - 1;
  DestroyAt(data_ + new_size);
  SetSizeNoGrow(new_size);
}

template<typename T>
inline void List<T>::RemoveRange(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(0 <= n && n <= size_ - at);
  Destroy(data_ + at, n);
  int old_size = Exchange(size_, size_ - n);
  UninitializedRelocate(data_ + at, data_ + at + n, old_size - n - at);
}

template<typename T>
inline void List<T>::Insert(int at, T item) {
  ASSERT(0 <= at && at <= size_);

  T* old_d = data_;
  int old_size = size_;

  if (capacity_ != size_) {
    UninitializedRelocate(old_d + at + 1, old_d + at, old_size - at);
    try {
      new (old_d + at) T(Move(item));
    } catch (...) {
      UninitializedRelocate(old_d + at, old_d + at + 1, old_size - at);
      throw;
    }
    SetSizeNoGrow(old_size + 1);
  } else {
    CheckGrow(1);

    int old_capacity = capacity_;

    int new_size = old_size + 1;
    int new_capacity = RecommendCapacity(new_size);
    T* new_d = Allocate<T>(new_capacity + CapacityIncrement_);

    new(new_d + at) T(Move(item));
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      UninitializedRelocate(new_d, old_d, at);
      UninitializedRelocate(new_d + at + 1, old_d + at, old_size - at);
      Free(old_d);
    }
  }
}

template<typename T>
inline T* List<T>::InsertUninitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  InsertMany(at, n, [](T* dst) {});
  return data_ + at;
}

template<typename T>
inline void List<T>::InsertInitialized(int at, int n) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);
  InsertMany(at, n, [n](T* dst) { UninitializedInit(dst, n); });
}

template<typename T>
inline void List<T>::InsertRange(int at, SpanType src) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(!IsSourceOf(src));
  InsertMany(at, src.size(), [src](T* dst) {
    UninitializedCopy(dst, src.data(), src.size());
  });
}

template<typename T>
template<typename TAction>
inline void List<T>::InsertMany(int at, int n, TAction&& action) {
  ASSERT(0 <= at && at <= size_);
  ASSERT(n >= 0);

  T* old_d = data_;
  int old_size = size_;

  if (capacity_ - old_size >= n) {
    UninitializedRelocate(old_d + at + n, old_d + at, old_size - at);
    try {
      action(old_d + at);
    } catch (...) {
      UninitializedRelocate(old_d + at, old_d + at + n, old_size - at);
      throw;
    }
    SetSizeNoGrow(old_size + n);
  } else {
    CheckGrow(n);

    int old_capacity = capacity_;

    int new_size = old_size + n;
    int new_capacity = RecommendCapacity(new_size);
    T* new_d = Allocate<T>(new_capacity + CapacityIncrement_);

    try {
      action(new_d + at);
    } catch (...) {
      Free(new_d);
      throw;
    }
    data_ = new_d;
    size_ = new_size;
    capacity_ = new_capacity;
    if (old_capacity) {
      UninitializedRelocate(new_d, old_d, at);
      UninitializedRelocate(new_d + at + n, old_d + at, old_size - at);
      Free(old_d);
    }
  }
}

template<typename T>
inline List<T> List<T>::AdoptMemory(T* ptr, int size, int capacity) {
  ASSERT(0 <= size && size <= capacity);
  List result;
  result.data_ = ptr;
  result.size_ = size;
  result.capacity_ = capacity;
  return result;
}

template<typename T>
inline T* List<T>::ReleaseMemory() {
  size_ = 0;
  capacity_ = 0;
  return Exchange(data_, nullptr);
}

template<typename T>
inline void List<T>::SetSizeNoGrow(int new_size) {
  ASSERT(0 <= new_size && new_size <= capacity_);
  size_ = new_size;
}

template<typename T>
inline void List<T>::AssignExternal(SpanType src) {
  if (capacity_ < src.size()) {
    if constexpr (TIsTriviallyDestructible<T>) {
      Clear();
    }
    EnsureCapacity(src.size());
    CopyNonOverlapping(data_, src.data(), size_);
    UninitializedCopy(data_ + size_, src.data() + size_, src.size() - size_);
    SetSizeNoGrow(src.size());
  } else {
    Truncate(src.size());
    CopyNonOverlapping(data_, src.data(), src.size());
  }
}

template<typename T>
inline void List<T>::AssignInternal(SpanType src) {
  int start = src.data() - data_;
  Truncate(start + src.size());
  RemoveRange(0, start);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_LIST_H_