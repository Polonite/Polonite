// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TYPE_PARSEINTEGER_H_
#define STP_BASE_TYPE_PARSEINTEGER_H_

#include "Base/Containers/Span.h"
#include "Base/Type/Limits.h"
#include "Base/Type/Parsable.h"
#include "Base/Type/Sign.h"

namespace stp {

template<int TBase, typename TChar, TEnableIf<TIsCharacter<TChar>>* = nullptr>
constexpr bool tryParseDigit(TChar c, uint8_t& digit) {
  if constexpr (TBase <= 10) {
    if ('0' <= c && c < '0' + TBase) {
      digit = static_cast<uint8_t>(c - '0');
      return true;
    }
    return false;
  } else {
    if (c >= '0' && c <= '9') {
      digit = static_cast<uint8_t>(c - '0');
    } else if (c >= 'a' && c < 'a' + TBase - 10) {
      digit = static_cast<uint8_t>(c - 'a' + 10);
    } else if (c >= 'A' && c < 'A' + TBase - 10) {
      digit = static_cast<uint8_t>(c - 'A' + 10);
    } else {
      return false;
    }
    return true;
  }
}

enum class ParseIntegerErrorCode {
  Ok,
  FormatError,
  OverflowError,
};

namespace detail {

template<typename TInteger, int TBase>
class IntegerParser {
 public:
  static constexpr TInteger Min = Limits<TInteger>::Min;
  static constexpr TInteger Max = Limits<TInteger>::Max;

  static constexpr ParseIntegerErrorCode invoke(StringSpan input, TInteger& out_value) {
    const char* it = begin(input);
    const char* it_end = end(input);

    auto result = ParseIntegerErrorCode::Ok;
    if (it != it_end && *it == '-') {
      if constexpr (!TIsSigned<TInteger>)
        result = ParseIntegerErrorCode::FormatError;
      else
        result = Negative::invoke(it + 1, it_end, out_value);
    } else {
      if (it != it_end && *it == '+')
        ++it;
      result = Positive::invoke(it, it_end, out_value);
    }
    return result;
  }

 private:
  // TSignHelper provides:
  //  - a static function, checkBounds, that determines whether the next digit
  //    causes an overflow/underflow
  //  - a static function, increment, that appends the next digit appropriately
  //    according to the sign of the number being parsed.
  template<typename TSignHelper>
  class Base {
   public:
    static constexpr ParseIntegerErrorCode invoke(const char* begin, const char* end, TInteger& output) {
      output = 0;
      if (begin == end)
        return ParseIntegerErrorCode::FormatError;

      if constexpr (TBase == 16) {
        if (end - begin > 2 && *begin == '0' &&
            (*(begin + 1) == 'x' || *(begin + 1) == 'X')) {
          begin += 2;
        }
      }

      for (const char* current = begin; current != end; ++current) {
        uint8_t new_digit = 0;

        if (!tryParseDigit<TBase>(*current, new_digit))
          return ParseIntegerErrorCode::FormatError;

        if (current != begin) {
          if (!TSignHelper::checkBounds(output, new_digit))
            return ParseIntegerErrorCode::OverflowError;

          output *= TBase;
        }

        TSignHelper::increment(new_digit, output);
      }
      return ParseIntegerErrorCode::Ok;
    }
  };

  class Positive : public Base<Positive> {
   public:
    static constexpr bool checkBounds(TInteger& output, uint8_t new_digit) {
      if (output > static_cast<TInteger>(Max / TBase) ||
          (output == static_cast<TInteger>(Max / TBase) &&
           new_digit > Max % TBase)) {
        return false;
      }
      return true;
    }
    static constexpr void increment(uint8_t amount, TInteger& output) {
      output += amount;
    }
  };

  class Negative : public Base<Negative> {
   public:
    static constexpr bool checkBounds(TInteger& output, uint8_t new_digit) {
      if (output < Min / TBase ||
          (output == Min / TBase && new_digit > 0 - Min % TBase)) {
        return false;
      }
      return true;
    }
    static constexpr void increment(uint8_t amount, TInteger& output) {
      output -= amount;
    }
  };
};

} // namespace detail

template<typename T, TEnableIf<TIsInteger<T>>*>
constexpr ParseIntegerErrorCode tryParse(StringSpan input, T& output) {
  return detail::IntegerParser<T, 10>::invoke(input, output);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr ParseIntegerErrorCode tryParseHex(StringSpan input, T& output) {
  return detail::IntegerParser<T, 16>::invoke(input, output);
}

template<typename T, TEnableIf<TIsInteger<T>>* = nullptr>
constexpr ParseIntegerErrorCode tryParseOctal(StringSpan input, T& output) {
  return detail::IntegerParser<T, 8>::invoke(input, output);
}

} // namespace stp

#endif // STP_BASE_TYPE_PARSEINTEGER_H_
