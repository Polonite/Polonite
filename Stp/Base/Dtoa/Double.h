// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright 2010 the V8 project authors. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//     * Neither the name of Google Inc. nor the names of its
//       contributors may be used to endorse or promote products derived
//       from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

#ifndef DOUBLE_CONVERSION_DOUBLE_H_
#define DOUBLE_CONVERSION_DOUBLE_H_

#include "Base/Dtoa/DiyFp.h"
#include "Base/Type/Variable.h"

namespace stp {

namespace dtoa {

// We assume that doubles and uint64_t have the same endianness.
static uint64_t DoubleToUInt64(double d) {
  return bitCast<uint64_t>(d);
}
static double UInt64ToDouble(uint64_t d64) {
  return bitCast<double>(d64);
}

// Helper functions for doubles.
class Double {
 public:
  static const uint64_t SignMask = UINT64_2PART_C(0x80000000, 00000000);
  static const uint64_t ExponentMask = UINT64_2PART_C(0x7FF00000, 00000000);
  static const uint64_t SignificandMask = UINT64_2PART_C(0x000FFFFF, FFFFFFFF);
  static const uint64_t HiddenBit = UINT64_2PART_C(0x00100000, 00000000);
  static const int PhysicalSignificandSize = 52;  // Excludes the hidden bit.
  static const int SignificandSize = 53;

  Double() : d64_(0) {}
  explicit Double(double d) : d64_(DoubleToUInt64(d)) {}
  explicit Double(uint64_t d64) : d64_(d64) {}
  explicit Double(DiyFp diy_fp) : d64_(DiyFpToUint64(diy_fp)) {}

  // The value encoded by this Double must be greater or equal to +0.0.
  // It must not be special (infinity, or NaN).
  DiyFp AsDiyFp() const {
    ASSERT(Sign() > 0);
    ASSERT(!IsSpecial());
    return DiyFp(Significand(), Exponent());
  }

  // The value encoded by this Double must be strictly greater than 0.
  DiyFp AsNormalizedDiyFp() const {
    ASSERT(value() > 0);
    uint64_t f = Significand();
    int e = Exponent();

    // The current double could be a denormal.
    while ((f & HiddenBit) == 0) {
      f <<= 1;
      e--;
    }
    // Do the final shifts in one go.
    f <<= DiyFp::SignificandSize - SignificandSize;
    e -= DiyFp::SignificandSize - SignificandSize;
    return DiyFp(f, e);
  }

  // Returns the double's bit as uint64.
  uint64_t AsUint64() const { return d64_; }

  // Returns the next greater double. Returns +infinity on input +infinity.
  double NextDouble() const {
    if (d64_ == InfinityBits)
      return Double(InfinityBits).value();
    if (Sign() < 0 && Significand() == 0) {
      // -0.0
      return 0.0;
    }
    if (Sign() < 0) {
      return Double(d64_ - 1).value();
    } else {
      return Double(d64_ + 1).value();
    }
  }

  int Exponent() const {
    if (IsDenormal())
      return DenormalExponentBits;

    uint64_t d64 = AsUint64();
    int biased_e =
        static_cast<int>((d64 & ExponentMask) >> PhysicalSignificandSize);
    return biased_e - ExponentBiasBits;
  }

  uint64_t Significand() const {
    uint64_t d64 = AsUint64();
    uint64_t significand = d64 & SignificandMask;
    if (!IsDenormal()) {
      return significand + HiddenBit;
    } else {
      return significand;
    }
  }

  // Returns true if the double is a denormal.
  bool IsDenormal() const {
    uint64_t d64 = AsUint64();
    return (d64 & ExponentMask) == 0;
  }

  // We consider denormals not to be special.
  // Hence only Infinity and NaN are special.
  bool IsSpecial() const {
    uint64_t d64 = AsUint64();
    return (d64 & ExponentMask) == ExponentMask;
  }

  bool isNaN() const {
    uint64_t d64 = AsUint64();
    return ((d64 & ExponentMask) == ExponentMask) &&
           ((d64 & SignificandMask) != 0);
  }

  bool IsInfinite() const {
    uint64_t d64 = AsUint64();
    return ((d64 & ExponentMask) == ExponentMask) &&
           ((d64 & SignificandMask) == 0);
  }

  int Sign() const {
    uint64_t d64 = AsUint64();
    return (d64 & SignMask) == 0 ? 1 : -1;
  }

  // Precondition: the value encoded by this Double must be greater or equal
  // than +0.0.
  DiyFp UpperBoundary() const {
    ASSERT(Sign() > 0);
    return DiyFp(Significand() * 2 + 1, Exponent() - 1);
  }

  // Computes the two boundaries of this.
  // The bigger boundary (m_plus) is normalized. The lower boundary has the same
  // exponent as m_plus.
  // Precondition: the value encoded by this Double must be greater than 0.
  void NormalizedBoundaries(DiyFp* out_m_minus, DiyFp* out_m_plus) const {
    ASSERT(value() > 0);
    DiyFp v = this->AsDiyFp();
    bool significand_is_zero = (v.f() == HiddenBit);
    DiyFp m_plus = DiyFp::Normalize(DiyFp((v.f() << 1) + 1, v.e() - 1));
    DiyFp m_minus;
    if (significand_is_zero && v.e() != DenormalExponentBits) {
      // The boundary is closer. Think of v = 1000e10 and v- = 9999e9.
      // Then the boundary (== (v - v-)/2) is not just at a distance of 1e9 but
      // at a distance of 1e8.
      // The only exception is for the smallest normal: the largest denormal is
      // at the same distance as its successor.
      // Note: denormals have the same exponent as the smallest normals.
      m_minus = DiyFp((v.f() << 2) - 1, v.e() - 2);
    } else {
      m_minus = DiyFp((v.f() << 1) - 1, v.e() - 1);
    }
    m_minus.set_f(m_minus.f() << (m_minus.e() - m_plus.e()));
    m_minus.set_e(m_plus.e());
    *out_m_plus = m_plus;
    *out_m_minus = m_minus;
  }

  double value() const { return UInt64ToDouble(d64_); }

  // Returns the significand size for a given order of magnitude.
  // If v = f*2^e with 2^p-1 <= f <= 2^p then p+e is v's order of magnitude.
  // This function returns the number of significant binary digits v will have
  // once it's encoded into a double. In almost all cases this is equal to
  // SignificandSize. The only exceptions are denormals. They start with
  // leading zeroes and their effective significand-size is hence smaller.
  static int SignificandSizeForOrderOfMagnitude(int order) {
    if (order >= (DenormalExponentBits + SignificandSize)) {
      return SignificandSize;
    }
    if (order <= DenormalExponentBits)
      return 0;
    return order - DenormalExponentBits;
  }

  static double Infinity() { return Double(InfinityBits).value(); }

  static double NaN() { return Double(NaNBits).value(); }

 private:
  static const int ExponentBiasBits = 0x3FF + PhysicalSignificandSize;
  static const int DenormalExponentBits = -ExponentBiasBits + 1;
  static const int MaxExponentBits = 0x7FF - ExponentBiasBits;
  static const uint64_t InfinityBits = UINT64_2PART_C(0x7FF00000, 00000000);
  static const uint64_t NaNBits = UINT64_2PART_C(0x7FF80000, 00000000);

  const uint64_t d64_;

  static uint64_t DiyFpToUint64(DiyFp diy_fp) {
    uint64_t significand = diy_fp.f();
    int exponent = diy_fp.e();
    while (significand > HiddenBit + SignificandMask) {
      significand >>= 1;
      exponent++;
    }
    if (exponent >= MaxExponentBits) {
      return InfinityBits;
    }
    if (exponent < DenormalExponentBits) {
      return 0;
    }
    while (exponent > DenormalExponentBits && (significand & HiddenBit) == 0) {
      significand <<= 1;
      exponent--;
    }
    uint64_t biased_exponent;
    if (exponent == DenormalExponentBits && (significand & HiddenBit) == 0) {
      biased_exponent = 0;
    } else {
      biased_exponent = static_cast<uint64_t>(exponent + ExponentBiasBits);
    }
    return (significand & SignificandMask) |
           (biased_exponent << PhysicalSignificandSize);
  }
};

} // namespace dtoa

} // namespace stp

#endif // DOUBLE_CONVERSION_DOUBLE_H_
