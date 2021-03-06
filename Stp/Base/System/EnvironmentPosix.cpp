// Copyright 2017 Polonite Authors. All rights reserved.
// Distributed under MIT license that can be found in the LICENSE file.

#include "Base/System/Environment.h"

#include "Base/FileSystem/FilePath.h"
#include "Base/Text/Wtf.h"
#include "Base/Thread/Lock.h"

#include <stdlib.h>

namespace stp {

BasicLock Environment::g_lock_ = BASIC_LOCK_INITIALIZER;

bool Environment::tryGet(StringSpan name, String& out_value) {
  return tryGet(toNullTerminated(String(name)), out_value);
}

bool Environment::tryGet(const char* name, String& out_value) {
  #if HAVE_UTF8_NATIVE_VALIDATION
  if (!tryGetNative(name, out_value))
    return false;
  ASSERT(Utf8::Validate(out_value));
  #else
  String mbvalue;
  if (!tryGetNative(name, mbvalue))
    return false;
  out_value.clear();
  appendWtf(out_value, mbvalue);
  #endif
  return true;
}

bool Environment::tryGetNative(const char* name, String& out_value) {
  AutoLock auto_lock(borrow(g_lock_));
  const char* mbvalue_cstr = ::getenv(name);
  if (!mbvalue_cstr)
    return false;
  out_value = makeSpanFromNullTerminated(mbvalue_cstr);
  return true;
}

bool Environment::tryGet(StringSpan name, FilePath& out_path) {
  return tryGet(toNullTerminated(String(name)), out_path);
}

bool Environment::tryGet(const char* name, FilePath& out_path) {
  return tryGetNative(name, out_path.chars());
}

bool Environment::Has(StringSpan name) {
  return Has(toNullTerminated(String(name)));
}

bool Environment::Has(const char* name) {
  ASSERT(name);
  AutoLock auto_lock(borrow(g_lock_));
  return ::getenv(name) != nullptr;
}

bool Environment::TrySet(StringSpan name, StringSpan value) {
  return TrySet(toNullTerminated(String(name)), toNullTerminated(String(value)));
}

bool Environment::TrySet(const char* name, const char* value) {
  AutoLock auto_lock(borrow(g_lock_));
  return ::setenv(name, value, 1) == 0;
}

bool Environment::TrySet(StringSpan name, const FilePath& path) {
  return TrySet(toNullTerminated(String(name)), toNullTerminated(path));
}

bool Environment::TryUnset(StringSpan name) {
  return TryUnset(toNullTerminated(String(name)));
}

bool Environment::TryUnset(const char* name) {
  AutoLock auto_lock(borrow(g_lock_));
  return ::unsetenv(name) == 0;
}
} // namespace stp
