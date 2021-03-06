// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef STP_BASE_MATH_SAFECONVERSIONS_H_
#define STP_BASE_MATH_SAFECONVERSIONS_H_

#include "Base/Debug/Assert.h"
#include "Base/Math/SafeConversionsImpl.h"

namespace stp {

// Convenience function that returns true if the supplied value is in range
// for the destination type.
template<typename TDst, typename TSrc>
constexpr bool isValueInRangeForNumericType(TSrc value) {
  return detail::dstRangeRelationToSrcRange<TDst>(value).isValid();
}

// assertedCast<> is analogous to static_cast<> for numeric types,
// except that it asserts that the specified numeric conversion will not
// overflow or underflow. NaN source will always trigger a ASSERT.
template<typename TDst, typename TSrc,
         TEnableIf<TIsArithmetic<TDst> && TIsArithmetic<TSrc>>* = nullptr>
constexpr TDst assertedCast(TSrc value) {
  ASSERT(isValueInRangeForNumericType<TDst>(value));
  return static_cast<TDst>(value);
}

// saturatedCast<> is analogous to static_cast<> for numeric types, except
// that the specified numeric conversion will saturate by default rather than
// overflow or underflow, and NaN assignment to an integer will return 0.
// All boundary condition behaviors can be overriden with a custom handler.
template<typename TDst, typename TSrc, TEnableIf<TIsFloatingPoint<TDst>>* = nullptr>
constexpr TDst saturatedCast(TSrc x) {
  return static_cast<TDst>(x);
}

template<typename TDst, typename TSrc, TEnableIf<TIsInteger<TDst>>* = nullptr>
constexpr TDst saturatedCast(TSrc x) {
  auto constraint = detail::dstRangeRelationToSrcRange<TDst, TSrc>(x);
  // For some reason clang generates much better code when the branch is
  // structured exactly this way, rather than a sequence of checks.
  return !constraint.isOverflowFlagSet()
      ? (!constraint.isUnderflowFlagSet() ? static_cast<TDst>(x) : Limits<TDst>::Min)
      // Skip this check for integral Src, which cannot be NaN.
      : (TIsInteger<TSrc> || !constraint.isUnderflowFlagSet()
          ? Limits<TDst>::Max : static_cast<TDst>(0));
}

// strictCast<> is analogous to static_cast<> for numeric types, except that
// it will cause a compile failure if the destination type is not large enough
// to contain any value in the source type. It performs no runtime checking.
template<typename TDst, typename TSrc>
constexpr TDst strictCast(TSrc value) {
  static_assert(TIsArithmetic<TSrc>, "argument must be numeric");
  static_assert(TIsArithmetic<TDst>, "result must be numeric");
  static_assert((detail::StaticDstRangeRelationToSrcRange<TDst, TSrc>::Value == detail::NumericRangeContained),
                "the numeric conversion is out of range for this type");
  return static_cast<TDst>(value);
}

} // namespace stp

#endif // STP_BASE_MATH_SAFECONVERSIONS_H_
