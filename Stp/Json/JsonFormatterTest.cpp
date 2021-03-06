// Copyright 2017 Polonite Authors. All rights reserved.
// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Json/JsonFormatter.h"

#include "Base/Debug/Log.h"
#include "Base/Test/GTest.h"
#include "Base/Text/Utf.h"

namespace stp {

TEST(JsonFormatterTest, BasicTypes) {
  EXPECT_EQ("null", formattableToString(JsonValue()));
  EXPECT_EQ("{}", formattableToString(JsonObject()));
  EXPECT_EQ("[]", formattableToString(JsonArray()));
  EXPECT_EQ("42", formattableToString(JsonValue(42)));
  EXPECT_EQ("true", formattableToString(JsonValue(true)));
  EXPECT_EQ("false", formattableToString(JsonValue(false)));

  // Test Real values should always have a decimal or an 'e'.
  EXPECT_EQ("1.0", formattableToString(JsonValue(1.0)));

  // Test Real values in the the range (-1, 1) must have leading zeros
  EXPECT_EQ("0.2", formattableToString(JsonValue(0.2)));
  EXPECT_EQ("-0.8", formattableToString(JsonValue(-0.8)));

  EXPECT_EQ("\"foo\"", formattableToString(JsonValue("foo")));
}

TEST(JsonFormatterTest, NestedTypes) {
  // Writer unittests like empty array/object nesting.
  JsonObject root;
  JsonArray array;
  JsonObject object;
  object.SetWithPath("inner int", 10);
  array.add(move(object));
  array.add(JsonArray());
  array.add(true);
  root.SetWithPath("array", move(array));

  // Test the pretty-printer.
  EXPECT_EQ("{\"array\":[{\"inner int\":10},[],true]}", formattableToString(root));

  EXPECT_EQ("{\n"
            "   \"array\": [ {\n"
            "      \"inner int\": 10\n"
            "   }, [  ], true ]\n"
            "}",
            formattableToString(root, "P"));
}

TEST(JsonFormatterTest, KeysWithPeriods) {
  JsonObject period_dict;
  period_dict.set("a.b", 3);
  period_dict.set("c", 2);
  JsonObject period_dict2;
  period_dict2.set("g.h.i.j", 1);
  period_dict.set("d.e.f", move(period_dict2));
  EXPECT_EQ("{\"a.b\":3,\"c\":2,\"d.e.f\":{\"g.h.i.j\":1}}", formattableToString(period_dict));

  JsonObject period_dict3;
  period_dict3.SetWithPath("a.b", 2);
  period_dict3.set("a.b", 1);
  EXPECT_EQ("{\"a\":{\"b\":2},\"a.b\":1}", formattableToString(period_dict3));
}

TEST(JsonFormatterTest, DoublesAsInts) {
  // Test allowing a double with no fractional part to be written as an integer.
  JsonValue double_value(1e10);
  EXPECT_EQ("10000000000", formattableToString(double_value, "I"));
}

TEST(JsonFormatterTest, EscapeUTF8) {
  const struct {
    StringSpan to_escape;
    StringSpan escaped;
  } cases[] = {
    {"\b\001aZ\"\\wee", "\\b\\u0001aZ\\\"\\\\wee"},
    {"a\b\f\n\r\t\v\1\\.\"z", "a\\b\\f\\n\\r\\t\\u000B\\u0001\\\\.\\\"z"},
    {"b\x0f\xff\xf0\xff!",  // \xf0\xff is not a valid UTF-8 unit.
        "b\\u000F\\uFFFD\\uFFFD\\uFFFD!"},
    {"Hello\xe2\x80\xa8world", "Hello\\u2028world"},
    {"\xe2\x80\xa9purple", "\\u2029purple"},
  };

  for (const auto& test : cases) {
    auto in = test.to_escape;
    auto escaped = test.escaped;

    String out;
    StringWriter os(&out);
    JsonFormatter::Escape(os, in, true);
    EXPECT_EQ(escaped, out);
    EXPECT_TRUE(Utf8::Validate(out));

    out.clear();
    JsonFormatter::Escape(os, in, true);
    EXPECT_EQ(escaped, out);
    EXPECT_TRUE(Utf8::Validate(out));
  }

  String in = cases[0].to_escape;
  String out;
  StringWriter os(&out);
  JsonFormatter::Escape(os, in, true);
  EXPECT_TRUE(Utf8::Validate(out));

  // now try with a NULL in the string
  String null_prepend = "test";
  null_prepend.add('\0');
  in = null_prepend + in;
  String expected = "test\\u0000";
  expected += cases[0].escaped;
  out.clear();
  JsonFormatter::Escape(os, in, true);
  EXPECT_EQ(expected, out);
  EXPECT_TRUE(Utf8::Validate(out));
}

} // namespace stp
