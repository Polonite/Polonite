// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_CONTAINERS_ARRAY_H_
#define STP_BASE_CONTAINERS_ARRAY_H_

#include "Base/Containers/Span.h"
#include "Base/Type/Common.h"

namespace stp {

template<typename T, int N>
struct Array {
  typedef T ItemType;
  typedef Span<T> SpanType;
  typedef MutableSpan<T> MutableSpanType;

  T data_[N];

  constexpr operator Span<T>() const { return toSpan(); }
  constexpr operator MutableSpan<T>() { return toSpan(); }

  ALWAYS_INLINE constexpr const T* data() const { return data_; }
  ALWAYS_INLINE constexpr T* data() { return data_; }
  ALWAYS_INLINE constexpr int size() const { return N; }

  constexpr const T& operator[](int at) const;
  constexpr T& operator[](int at);

  constexpr const T& first() const { return toSpan().first(); }
  constexpr const T& last() const { return toSpan().last(); }
  constexpr T& first() { return toSpan().first(); }
  constexpr T& last() { return toSpan().last(); }

  constexpr SpanType slice(int at) const { return toSpan().slice(at); }
  constexpr SpanType slice(int at, int n) const { return toSpan().slice(at, n); }
  constexpr MutableSpanType slice(int at) { return toSpan().slice(at); }
  constexpr MutableSpanType slice(int at, int n) { return toSpan().slice(at, n); }

  template<typename U>
  int indexOf(const U& item) const { return toSpan().indexOf(item); }
  template<typename U>
  int lastIndexOf(const U& item) const { return toSpan().lastIndexOf(item); }
  template<typename U>
  bool contains(const U& item) const { return toSpan().contains(item); }

  friend constexpr void swap(Array& lhs, Array& rhs) { swap(lhs.data_, rhs.data_); }
  friend constexpr bool operator==(const Array& lhs, SpanType rhs) { return operator==(lhs.toSpan(), rhs); }
  friend constexpr bool operator!=(const Array& lhs, SpanType rhs) { return operator==(lhs.toSpan(), rhs); }
  friend int compare(const Array& lhs, SpanType rhs) { return compare(lhs.toSpan(), rhs); }

  friend constexpr const T* begin(const Array& x) { return x.data_; }
  friend constexpr const T* end(const Array& x) { return x.data_ + N; }
  friend constexpr T* begin(Array& x) { return x.data_; }
  friend constexpr T* end(Array& x) { return x.data_ + N; }

  constexpr SpanType toSpan() const { return SpanType(data_, N); }
  constexpr MutableSpanType toSpan() { return MutableSpanType(data_, N); }
};

template<typename T, int N>
struct TIsZeroConstructibleTmpl<Array<T, N>> : TIsZeroConstructibleTmpl<T> {};
template<typename T, int N>
struct TIsTriviallyRelocatableTmpl<Array<T, N>> : TIsTriviallyRelocatableTmpl<T> {};

template<typename T = void, typename... TElements>
constexpr auto makeArray(TElements&&... elements) {
  using DT = TConditional<TIsVoid<T>, TCommon<TElements...>, T>;
  return Array<DT, sizeof...(TElements)> { { forward<TElements>(elements)... } };
}

template<typename T, int N>
inline BufferSpan makeBufferSpan(const Array<T, N>& array) {
  return makeBufferSpan(array.toSpan());
}
template<typename T, int N>
inline MutableBufferSpan makeBufferSpan(Array<T, N>& array) {
  return makeBufferSpan(array.toSpan());
}

template<typename T, int N>
constexpr const T& Array<T, N>::operator[](int at) const {
  ASSERT(0 <= at && at < N);
  return *(data_ + at);
}

template<typename T, int N>
constexpr T& Array<T, N>::operator [](int at) {
  ASSERT(0 <= at && at < N);
  return *(data_ + at);
}

} // namespace stp

#endif // STP_BASE_CONTAINERS_ARRAY_H_
