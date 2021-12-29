/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "velox/functions/prestosql/tests/FunctionBaseTest.h"

namespace facebook::velox::functions::prestosql {

namespace {

class JsonFormatTest : public functions::test::FunctionBaseTest {
 public:
  std::optional<std::string> json_format(std::optional<std::string> json) {
    return evaluateOnce<std::string>("json_format(c0)", json);
  }
};

TEST_F(JsonFormatTest, simple) {
  // Scalars.
  EXPECT_EQ(json_format(R"(1)").value(), "1");
  EXPECT_EQ(json_format(R"(123456)").value(), "123456");
  EXPECT_EQ(json_format(R"("hello")").value(), "\"hello\"");
  EXPECT_EQ(json_format(R"(1.1)").value(), "1.1");
  EXPECT_EQ(json_format(R"("")").value(), "\"\"");

  // Simple lists.
  EXPECT_EQ(json_format(R"([1, 2, 3])").value(), "[1,2,3]");

  // Simple maps.
  EXPECT_EQ(json_format(R"({"k1":"v1"})").value(), "{\"k1\":\"v1\"}");

  // Nested
  EXPECT_EQ(
      json_format(R"({"k1":{"k2": 999}})").value(), "{\"k1\":{\"k2\":999}}");
  EXPECT_EQ(json_format(R"({"k1":[0,1,2]})").value(), "{\"k1\":[0,1,2]}");
}

TEST_F(JsonFormatTest, utf8) {
  EXPECT_EQ(
      json_format(R"({"k1":"I \u2665 UTF-8"})").value(),
      u8"{\"k1\":\"I \u2665 UTF-8\"}");
  EXPECT_EQ(
      json_format(u8"{\"k1\":\"I \u2665 UTF-8\"}").value(),
      u8"{\"k1\":\"I \u2665 UTF-8\"}");

  EXPECT_EQ(
      json_format(u8"{\"k1\":\"I \U0001D11E playing in G-clef\"}").value(),
      u8"{\"k1\":\"I \U0001D11E playing in G-clef\"}");
}

} // namespace

} // namespace facebook::velox::functions::prestosql
