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
#include "folly/json.h"
#include "velox/functions/Macros.h"
#include "velox/functions/UDFOutputString.h"

namespace facebook::velox::functions {

template <typename T>
struct JsonFormatFunction {
  VELOX_DEFINE_FUNCTION_TYPES(T);

  FOLLY_ALWAYS_INLINE bool call(
      out_type<Varchar>& result,
      const arg_type<Varchar>& json) {
    const folly::StringPiece& jsonStringPiece = json;

    folly::dynamic jsonObj;
    std::string jsonString;
    try {
      jsonObj = folly::parseJson(jsonStringPiece);
      jsonString = folly::toJson(jsonObj);
    } catch (const std::exception& e) {
      VELOX_USER_FAIL(e.what());
    }

    result.resize(jsonString.size());
    if (!jsonString.empty()) {
      std::memcpy(result.data(), jsonString.data(), jsonString.size());
    }

    return true;
  }
};

} // namespace facebook::velox::functions
