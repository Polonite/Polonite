// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#ifndef STP_BASE_TEXT_CODEC_TEXTCODEC_H_
#define STP_BASE_TEXT_CODEC_TEXTCODEC_H_

#include "Base/Containers/BufferSpan.h"
#include "Base/Error/Exception.h"
#include "Base/Text/AsciiChar.h"
#include "Base/Text/StringSpan.h"
#include "Base/Text/Utf.h"
#include "Base/Type/HashableFwd.h"

namespace stp {

struct TextConversionContext {
  union State {
    State() { memset(this, 0, sizeof(TextConversionContext)); }
    void* ptr;
    byte_t bytes[8];
  };
  State state;
  bool exception_on_fallback = false;
  bool did_fallback = false;

  void MaybeThrow(bool saw_error);
};

class BASE_EXPORT TextConversionFallbackException final : public Exception {
 public:
  StringSpan GetName() const noexcept override;
};

struct TextCodecVtable {
  using Context = TextConversionContext;

  int (*decode)(Context&, BufferSpan, MutableStringSpan, bool);
  int (*decode16)(Context&, BufferSpan, MutableString16Span, bool);
  int (*count_chars)(const Context&, BufferSpan);
  int (*count_chars16)(const Context&, BufferSpan);

  int (*encode)(Context&, StringSpan, MutableBufferSpan);
  int (*encode16)(Context&, String16Span, MutableBufferSpan);
  int (*count_bytes)(const Context&, StringSpan);
  int (*count_bytes16)(const Context&, String16Span);
};

struct TextCodec {
  const TextCodecVtable* vtable = nullptr;

  StringSpan name;
  Span<StringSpan> aliases;

  int iana_codepage = 0;
  int windows_codepage = 0;

  bool single_byte = false;
};

namespace detail {

BASE_EXPORT extern const TextCodec UndefinedTextCodec;
BASE_EXPORT extern const TextCodec AsciiCodec;
BASE_EXPORT extern const TextCodec Cp1252Codec;
BASE_EXPORT extern const TextCodec Latin1Codec;
BASE_EXPORT extern const TextCodec Latin2Codec;
BASE_EXPORT extern const TextCodec Latin3Codec;
BASE_EXPORT extern const TextCodec Latin4Codec;
BASE_EXPORT extern const TextCodec Utf16BECodec;
BASE_EXPORT extern const TextCodec Utf16LECodec;
BASE_EXPORT extern const TextCodec Utf16Codec;
BASE_EXPORT extern const TextCodec Utf32BECodec;
BASE_EXPORT extern const TextCodec Utf32LECodec;
BASE_EXPORT extern const TextCodec Utf32Codec;
BASE_EXPORT extern const TextCodec Utf8Codec;

} // namespace detail

class BASE_EXPORT TextEncoding {
 public:
  constexpr TextEncoding() noexcept : codec_(detail::UndefinedTextCodec) {}
  constexpr TextEncoding(const TextCodec* codec) noexcept : codec_(*codec) {}

  // Name and aliases are specified in IANA character sets:
  // https://www.iana.org/assignments/character-sets/character-sets.xhtml
  // |name| is standard IANA name for character set.
  StringSpan GetName() const { return codec_.name; }
  Span<StringSpan> GetAliases() const { return codec_.aliases; }

  int GetIanaCodepage() { return codec_.iana_codepage; }
  int GetWindowsCodepage() const { return codec_.windows_codepage; }

  bool CanDecode() const { return codec_.vtable->decode != nullptr; }
  bool CanEncode() const { return codec_.vtable->encode != nullptr; }

  // True if each unit takes single byte and ASCII compatible.
  bool IsSingleByte() const { return codec_.single_byte; }

  bool IsValid() const { return &codec_ != &detail::UndefinedTextCodec; }

  const TextCodecVtable& GetVtable() const { return *codec_.vtable; }

  friend bool operator==(const TextEncoding& l, const TextEncoding& r) { return &l == &r; }
  friend bool operator!=(const TextEncoding& l, const TextEncoding& r) { return !operator==(l, r); }
  friend HashCode Hash(const TextEncoding& codec) { return codec.HashImpl(); }

  static bool AreNamesMatching(StringSpan lhs, StringSpan rhs) noexcept;

 private:
  const TextCodec& codec_;

  HashCode HashImpl() const noexcept;
};

BASE_EXPORT String ToString(BufferSpan buffer, TextEncoding codec);
BASE_EXPORT String16 ToString16(BufferSpan buffer, TextEncoding codec);

class BuiltinTextEncodings {
  STATIC_ONLY(BuiltinTextEncodings);
 public:
  static TextEncoding Ascii() { return &detail::AsciiCodec; }
  static TextEncoding Cp1252() { return &detail::Cp1252Codec; }
  static TextEncoding Latin1() { return &detail::Latin1Codec; }
  static TextEncoding Latin2() { return &detail::Latin2Codec; }
  static TextEncoding Latin3() { return &detail::Latin3Codec; }
  static TextEncoding Latin4() { return &detail::Latin4Codec; }

  static TextEncoding Utf8() { return &detail::Utf8Codec; }
  static TextEncoding Utf16() { return &detail::Utf16Codec; }
  static TextEncoding Utf16BE() { return &detail::Utf16BECodec; }
  static TextEncoding Utf16LE() { return &detail::Utf16LECodec; }
  static TextEncoding Utf32() { return &detail::Utf32Codec; }
  static TextEncoding Utf32BE() { return &detail::Utf32BECodec; }
  static TextEncoding Utf32LE() { return &detail::Utf32LECodec; }
};

class TextCodecBuilder {
 public:
  constexpr TextCodecBuilder() = default;

  constexpr TextCodecBuilder(StringSpan name, const TextCodecVtable& vtable) {
    codec_.name = name;
    codec_.vtable = &vtable;
  }

  constexpr operator TextCodec() const { return codec_; }

  constexpr void SetAliases(Span<StringSpan> aliases) { codec_.aliases = aliases; }
  constexpr void SetIanaCodepage(int cp) { codec_.iana_codepage = cp; }
  constexpr void SetWindowsCodepage(int cp) { codec_.windows_codepage = cp; }

 private:
  TextCodec codec_;
};

constexpr TextCodecBuilder BuildTextCodec(StringSpan name, const TextCodecVtable& vtable) {
  return TextCodecBuilder(name, vtable);
}

inline void TextConversionContext::MaybeThrow(bool saw_error) {
  if (saw_error) {
    did_fallback = true;
    if (exception_on_fallback)
      throw TextConversionFallbackException();
  }
}

} // namespace stp

#endif // STP_BASE_TEXT_CODEC_TEXTCODEC_H_
