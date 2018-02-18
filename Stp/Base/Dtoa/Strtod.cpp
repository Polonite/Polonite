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

#include "Base/Dtoa/Strtod.h"

#include "Base/Dtoa/Bignum.h"
#include "Base/Dtoa/CachedPowers.h"
#include "Base/Dtoa/Double.h"

#include <stdarg.h>
#include <limits.h>

namespace stp {

namespace dtoa {

// 2^53 = 9007199254740992.
// Any integer with at most 15 decimal digits will hence fit into a double
// (which has a 53bit significand) without loss of precision.
static const int MaxExactDoubleIntegerDecimalDigits = 15;
// 2^64 = 18446744073709551616 > 10^19
static const int MaxUInt64DecimalDigits = 19;

// Max double: 1.7976931348623157 x 10^308
// Min non-zero double: 4.9406564584124654 x 10^-324
// Any x >= 10^309 is interpreted as +infinity.
// Any x <= 10^-324 is interpreted as 0.
// Note that 2.5e-324 (despite being smaller than the min double) will be read
// as non-zero (equal to the min non-zero double).
static const int MaxDecimalPower = 309;
static const int MinDecimalPower = -324;

// 2^64 = 18446744073709551616
static const uint64_t MaxUInt64 = UINT64_2PART_C(0xFFFFFFFF, FFFFFFFF);


static const double ExactPowersOfTen[] = {
  1.0,  // 10^0
  10.0,
  100.0,
  1000.0,
  10000.0,
  100000.0,
  1000000.0,
  10000000.0,
  100000000.0,
  1000000000.0,
  10000000000.0,  // 10^10
  100000000000.0,
  1000000000000.0,
  10000000000000.0,
  100000000000000.0,
  1000000000000000.0,
  10000000000000000.0,
  100000000000000000.0,
  1000000000000000000.0,
  10000000000000000000.0,
  100000000000000000000.0,  // 10^20
  1000000000000000000000.0,
  // 10^22 = 0x21e19e0c9bab2400000 = 0x878678326eac9 * 2^22
  10000000000000000000000.0
};
static constexpr int ExactPowersOfTenSize = isizeofArray(ExactPowersOfTen);

// Maximum number of significant digits in the decimal representation.
// In fact the value is 772 , but to give us some margin we round up to 780.
static const int MaxSignificantDecimalDigits = 780;

static Vector<const char> TrimLeadingZeros(Vector<const char> buffer) {
  for (int i = 0; i < buffer.length(); i++) {
    if (buffer[i] != '0') {
      return buffer.SubVector(i, buffer.length());
    }
  }
  return Vector<const char>(buffer.start(), 0);
}


static Vector<const char> TrimTrailingZeros(Vector<const char> buffer) {
  for (int i = buffer.length() - 1; i >= 0; --i) {
    if (buffer[i] != '0') {
      return buffer.SubVector(0, i + 1);
    }
  }
  return Vector<const char>(buffer.start(), 0);
}


static void TrimToMaxSignificantDigits(Vector<const char> buffer,
                                       int exponent,
                                       char* significant_buffer,
                                       int* significant_exponent) {
  for (int i = 0; i < MaxSignificantDecimalDigits - 1; ++i) {
    significant_buffer[i] = buffer[i];
  }
  // The input buffer has been trimmed. Therefore the last digit must be
  // different from '0'.
  ASSERT(buffer[buffer.length() - 1] != '0');
  // Set the last digit to be non-zero. This is sufficient to guarantee
  // correct rounding.
  significant_buffer[MaxSignificantDecimalDigits - 1] = '1';
  *significant_exponent =
      exponent + (buffer.length() - MaxSignificantDecimalDigits);
}

// Reads digits from the buffer and converts them to a uint64.
// Reads in as many digits as fit into a uint64.
// When the string starts with "1844674407370955161" no further digit is read.
// Since 2^64 = 18446744073709551616 it would still be possible read another
// digit if it was less or equal than 6, but this would complicate the code.
static uint64_t ReadUint64(Vector<const char> buffer,
                           int* number_of_read_digits) {
  uint64_t result = 0;
  int i = 0;
  while (i < buffer.length() && result <= (MaxUInt64 / 10 - 1)) {
    int digit = buffer[i++] - '0';
    ASSERT(0 <= digit && digit <= 9);
    result = 10 * result + digit;
  }
  *number_of_read_digits = i;
  return result;
}


// Reads a DiyFp from the buffer.
// The returned DiyFp is not necessarily normalized.
// If remaining_decimals is zero then the returned DiyFp is accurate.
// Otherwise it has been rounded and has error of at most 1/2 ulp.
static void ReadDiyFp(Vector<const char> buffer,
                      DiyFp* result,
                      int* remaining_decimals) {
  int read_digits;
  uint64_t significand = ReadUint64(buffer, &read_digits);
  if (buffer.length() == read_digits) {
    *result = DiyFp(significand, 0);
    *remaining_decimals = 0;
  } else {
    // Round the significand.
    if (buffer[read_digits] >= '5') {
      significand++;
    }
    // Compute the binary exponent.
    int exponent = 0;
    *result = DiyFp(significand, exponent);
    *remaining_decimals = buffer.length() - read_digits;
  }
}


static bool DoubleStrtod(Vector<const char> trimmed,
                         int exponent,
                         double* result) {
  #if !defined(DOUBLE_CONVERSION_CORRECT_DOUBLE_OPERATIONS)
  // On x86 the floating-point stack can be 64 or 80 bits wide. If it is
  // 80 bits wide (as is the case on Linux) then double-rounding occurs and the
  // result is not accurate.
  // We know that Windows32 uses 64 bits and is therefore accurate.
  // Note that the ARM simulator is compiled for 32bits. It therefore exhibits
  // the same problem.
  return false;
  #endif
  if (trimmed.length() <= MaxExactDoubleIntegerDecimalDigits) {
    int read_digits;
    // The trimmed input fits into a double.
    // If the 10^exponent (resp. 10^-exponent) fits into a double too then we
    // can compute the result-double simply by multiplying (resp. dividing) the
    // two numbers.
    // This is possible because IEEE guarantees that floating-point operations
    // return the best possible approximation.
    if (exponent < 0 && -exponent < ExactPowersOfTenSize) {
      // 10^-exponent fits into a double.
      *result = static_cast<double>(ReadUint64(trimmed, &read_digits));
      ASSERT(read_digits == trimmed.length());
      *result /= ExactPowersOfTen[-exponent];
      return true;
    }
    if (0 <= exponent && exponent < ExactPowersOfTenSize) {
      // 10^exponent fits into a double.
      *result = static_cast<double>(ReadUint64(trimmed, &read_digits));
      ASSERT(read_digits == trimmed.length());
      *result *= ExactPowersOfTen[exponent];
      return true;
    }
    int remaining_digits = MaxExactDoubleIntegerDecimalDigits - trimmed.length();
    if ((0 <= exponent) &&
        (exponent - remaining_digits < ExactPowersOfTenSize)) {
      // The trimmed string was short and we can multiply it with
      // 10^remaining_digits. As a result the remaining exponent now fits
      // into a double too.
      *result = static_cast<double>(ReadUint64(trimmed, &read_digits));
      ASSERT(read_digits == trimmed.length());
      *result *= ExactPowersOfTen[remaining_digits];
      *result *= ExactPowersOfTen[exponent - remaining_digits];
      return true;
    }
  }
  return false;
}


// Returns 10^exponent as an exact DiyFp.
// The given exponent must be in the range [1; DecimalExponentDistance[.
static DiyFp AdjustmentPowerOfTen(int exponent) {
  ASSERT(0 < exponent);
  ASSERT(exponent < PowersOfTenCache::DecimalExponentDistance);
  // Simply hardcode the remaining powers for the given decimal exponent
  // distance.
  ASSERT(PowersOfTenCache::DecimalExponentDistance == 8);
  switch (exponent) {
    case 1: return DiyFp(UINT64_2PART_C(0xa0000000, 00000000), -60);
    case 2: return DiyFp(UINT64_2PART_C(0xc8000000, 00000000), -57);
    case 3: return DiyFp(UINT64_2PART_C(0xfa000000, 00000000), -54);
    case 4: return DiyFp(UINT64_2PART_C(0x9c400000, 00000000), -50);
    case 5: return DiyFp(UINT64_2PART_C(0xc3500000, 00000000), -47);
    case 6: return DiyFp(UINT64_2PART_C(0xf4240000, 00000000), -44);
    case 7: return DiyFp(UINT64_2PART_C(0x98968000, 00000000), -40);
    default:
      UNREACHABLE(return DiyFp(0, 0));
  }
}


// If the function returns true then the result is the correct double.
// Otherwise it is either the correct double or the double that is just below
// the correct double.
static bool DiyFpStrtod(Vector<const char> buffer,
                        int exponent,
                        double* result) {
  DiyFp input;
  int remaining_decimals;
  ReadDiyFp(buffer, &input, &remaining_decimals);
  // Since we may have dropped some digits the input is not accurate.
  // If remaining_decimals is different than 0 than the error is at most
  // .5 ulp (unit in the last place).
  // We don't want to deal with fractions and therefore keep a common
  // denominator.
  const int DenominatorLog = 3;
  const int Denominator = 1 << DenominatorLog;
  // Move the remaining decimals into the exponent.
  exponent += remaining_decimals;
  int64_t error = (remaining_decimals == 0 ? 0 : Denominator / 2);

  int old_e = input.e();
  input.Normalize();
  error <<= old_e - input.e();

  ASSERT(exponent <= PowersOfTenCache::MaxDecimalExponent);
  if (exponent < PowersOfTenCache::MinDecimalExponent) {
    *result = 0.0;
    return true;
  }
  DiyFp cached_power;
  int cached_decimal_exponent;
  PowersOfTenCache::GetCachedPowerForDecimalExponent(exponent,
                                                     &cached_power,
                                                     &cached_decimal_exponent);

  if (cached_decimal_exponent != exponent) {
    int adjustment_exponent = exponent - cached_decimal_exponent;
    DiyFp adjustment_power = AdjustmentPowerOfTen(adjustment_exponent);
    input.Multiply(adjustment_power);
    if (MaxUInt64DecimalDigits - buffer.length() >= adjustment_exponent) {
      // The product of input with the adjustment power fits into a 64 bit
      // integer.
      ASSERT(DiyFp::SignificandSize == 64);
    } else {
      // The adjustment power is exact. There is hence only an error of 0.5.
      error += Denominator / 2;
    }
  }

  input.Multiply(cached_power);
  // The error introduced by a multiplication of a*b equals
  //   error_a + error_b + error_a*error_b/2^64 + 0.5
  // Substituting a with 'input' and b with 'cached_power' we have
  //   error_b = 0.5  (all cached powers have an error of less than 0.5 ulp),
  //   error_ab = 0 or 1 / Denominator > error_a*error_b/ 2^64
  int error_b = Denominator / 2;
  int error_ab = (error == 0 ? 0 : 1);  // We round up to 1.
  int fixed_error = Denominator / 2;
  error += error_b + error_ab + fixed_error;

  old_e = input.e();
  input.Normalize();
  error <<= old_e - input.e();

  // See if the double's significand changes if we add/subtract the error.
  int order_of_magnitude = DiyFp::SignificandSize + input.e();
  int effective_significand_size =
      Double::SignificandSizeForOrderOfMagnitude(order_of_magnitude);
  int precision_digits_count =
      DiyFp::SignificandSize - effective_significand_size;
  if (precision_digits_count + DenominatorLog >= DiyFp::SignificandSize) {
    // This can only happen for very small denormals. In this case the
    // half-way multiplied by the denominator exceeds the range of an uint64.
    // Simply shift everything to the right.
    int shift_amount = (precision_digits_count + DenominatorLog) - DiyFp::SignificandSize + 1;
    input.set_f(input.f() >> shift_amount);
    input.set_e(input.e() + shift_amount);
    // We add 1 for the lost precision of error, and Denominator for
    // the lost precision of input.f().
    error = (error >> shift_amount) + 1 + Denominator;
    precision_digits_count -= shift_amount;
  }
  // We use uint64_ts now. This only works if the DiyFp uses uint64_ts too.
  ASSERT(DiyFp::SignificandSize == 64);
  ASSERT(precision_digits_count < 64);
  uint64_t one64 = 1;
  uint64_t precision_bits_mask = (one64 << precision_digits_count) - 1;
  uint64_t precision_bits = input.f() & precision_bits_mask;
  uint64_t half_way = one64 << (precision_digits_count - 1);
  precision_bits *= Denominator;
  half_way *= Denominator;
  DiyFp rounded_input(input.f() >> precision_digits_count,
                      input.e() + precision_digits_count);
  if (precision_bits >= half_way + error) {
    rounded_input.set_f(rounded_input.f() + 1);
  }
  // If the last_bits are too close to the half-way case than we are too
  // inaccurate and round down. In this case we return false so that we can
  // fall back to a more precise algorithm.

  *result = Double(rounded_input).value();
  if (half_way - error < precision_bits && precision_bits < half_way + error) {
    // Too imprecise. The caller will have to fall back to a slower version.
    // However the returned number is guaranteed to be either the correct
    // double, or the next-lower double.
    return false;
  }
  return true;
}


// Returns the correct double for the buffer*10^exponent.
// The variable guess should be a close guess that is either the correct double
// or its lower neighbor (the nearest double less than the correct one).
// Preconditions:
//   buffer.length() + exponent <= MaxDecimalPower + 1
//   buffer.length() + exponent > MinDecimalPower
//   buffer.length() <= MaxDecimalSignificantDigits
static double BignumStrtod(Vector<const char> buffer,
                           int exponent,
                           double guess) {
  if (guess == Double::Infinity())
    return guess;

  DiyFp upper_boundary = Double(guess).UpperBoundary();

  ASSERT(buffer.length() + exponent <= MaxDecimalPower + 1);
  ASSERT(buffer.length() + exponent > MinDecimalPower);
  ASSERT(buffer.length() <= MaxSignificantDecimalDigits);
  // Make sure that the Bignum will be able to hold all our numbers.
  // Our Bignum implementation has a separate field for exponents. Shifts will
  // consume at most one bigit (< 64 bits).
  // ln(10) == 3.3219...
  ASSERT(((MaxDecimalPower + 1) * 333 / 100) < Bignum::MaxSignificantBits);
  Bignum input;
  Bignum boundary;
  input.AssignDecimalString(buffer);
  boundary.AssignUInt64(upper_boundary.f());
  if (exponent >= 0) {
    input.MultiplyByPowerOfTen(exponent);
  } else {
    boundary.MultiplyByPowerOfTen(-exponent);
  }
  if (upper_boundary.e() > 0) {
    boundary.ShiftLeft(upper_boundary.e());
  } else {
    input.ShiftLeft(-upper_boundary.e());
  }
  int comparison = Bignum::compare(input, boundary);
  if (comparison < 0)
    return guess;
  if (comparison > 0)
    return Double(guess).NextDouble();
  if ((Double(guess).Significand() & 1) == 0) {
    // Round towards even.
    return guess;
  }
  return Double(guess).NextDouble();
}


double Strtod(Vector<const char> buffer, int exponent) {
  Vector<const char> left_trimmed = TrimLeadingZeros(buffer);
  Vector<const char> trimmed = TrimTrailingZeros(left_trimmed);
  exponent += left_trimmed.length() - trimmed.length();
  if (trimmed.length() == 0) return 0.0;
  if (trimmed.length() > MaxSignificantDecimalDigits) {
    char significant_buffer[MaxSignificantDecimalDigits];
    int significant_exponent;
    TrimToMaxSignificantDigits(trimmed, exponent,
                               significant_buffer, &significant_exponent);
    return Strtod(Vector<const char>(significant_buffer,
                                     MaxSignificantDecimalDigits),
                  significant_exponent);
  }
  if (exponent + trimmed.length() - 1 >= MaxDecimalPower)
    return Double::Infinity();
  if (exponent + trimmed.length() <= MinDecimalPower)
    return 0.0;

  double guess;
  if (DoubleStrtod(trimmed, exponent, &guess) ||
      DiyFpStrtod(trimmed, exponent, &guess)) {
    return guess;
  }
  return BignumStrtod(trimmed, exponent, guess);
}

} // namespace dtoa

} // namespace stp
