// Copyright 2007, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
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
//
// This file tests the internal utilities.

#include "Base/Test/Internal/GMockGeneratedInternalUtils.h"

#include "Base/Test/GTest.h"
#include "Base/Test/Internal/GMockInternalUtils.h"

namespace {

using ::testing::tuple;
using ::testing::Matcher;
using ::testing::internal::CompileAssertTypesEqual;
using ::testing::internal::MatcherTuple;
using ::testing::internal::Function;
using ::testing::internal::IgnoredValue;

// Tests the MatcherTuple template struct.

TEST(MatcherTupleTest, ForSize0) {
  CompileAssertTypesEqual<tuple<>, MatcherTuple<tuple<> >::type>();
}

TEST(MatcherTupleTest, ForSize1) {
  CompileAssertTypesEqual<tuple<Matcher<int> >,
                          MatcherTuple<tuple<int> >::type>();
}

TEST(MatcherTupleTest, ForSize2) {
  CompileAssertTypesEqual<tuple<Matcher<int>, Matcher<char> >,
                          MatcherTuple<tuple<int, char> >::type>();
}

TEST(MatcherTupleTest, ForSize5) {
  CompileAssertTypesEqual<tuple<Matcher<int>, Matcher<char>, Matcher<bool>,
                                Matcher<double>, Matcher<char*> >,
                          MatcherTuple<tuple<int, char, bool, double, char*>
                                      >::type>();
}

// Tests the Function template struct.

TEST(FunctionTest, Nullary) {
  typedef Function<int()> F;  
  CompileAssertTypesEqual<int, F::Result>();
  CompileAssertTypesEqual<tuple<>, F::ArgumentTuple>();
  CompileAssertTypesEqual<tuple<>, F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(), F::MakeResultVoid>();
  CompileAssertTypesEqual<IgnoredValue(), F::MakeResultIgnoredValue>();
}

TEST(FunctionTest, Unary) {
  typedef Function<int(bool)> F;  
  CompileAssertTypesEqual<int, F::Result>();
  CompileAssertTypesEqual<bool, F::Argument1>();
  CompileAssertTypesEqual<tuple<bool>, F::ArgumentTuple>();
  CompileAssertTypesEqual<tuple<Matcher<bool> >, F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(bool), F::MakeResultVoid>();  
  CompileAssertTypesEqual<IgnoredValue(bool),  
      F::MakeResultIgnoredValue>();
}

TEST(FunctionTest, Binary) {
  typedef Function<int(bool, const long&)> F;  
  CompileAssertTypesEqual<int, F::Result>();
  CompileAssertTypesEqual<bool, F::Argument1>();
  CompileAssertTypesEqual<const long&, F::Argument2>();  
  CompileAssertTypesEqual<tuple<bool, const long&>, F::ArgumentTuple>();  
  CompileAssertTypesEqual<tuple<Matcher<bool>, Matcher<const long&> >,  
                          F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(bool, const long&), F::MakeResultVoid>();  
  CompileAssertTypesEqual<IgnoredValue(bool, const long&),  
      F::MakeResultIgnoredValue>();
}

TEST(FunctionTest, LongArgumentList) {
  typedef Function<char(bool, int, char*, int&, const long&)> F;  
  CompileAssertTypesEqual<char, F::Result>();
  CompileAssertTypesEqual<bool, F::Argument1>();
  CompileAssertTypesEqual<int, F::Argument2>();
  CompileAssertTypesEqual<char*, F::Argument3>();
  CompileAssertTypesEqual<int&, F::Argument4>();
  CompileAssertTypesEqual<const long&, F::Argument5>();  
  CompileAssertTypesEqual<tuple<bool, int, char*, int&, const long&>,  
                          F::ArgumentTuple>();
  CompileAssertTypesEqual<tuple<Matcher<bool>, Matcher<int>, Matcher<char*>,
                                Matcher<int&>, Matcher<const long&> >,  
                          F::ArgumentMatcherTuple>();
  CompileAssertTypesEqual<void(bool, int, char*, int&, const long&),  
                          F::MakeResultVoid>();
  CompileAssertTypesEqual<
      IgnoredValue(bool, int, char*, int&, const long&),  
      F::MakeResultIgnoredValue>();
}

}  // Unnamed namespace
