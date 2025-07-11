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

#include <gtest/gtest.h>
#include <array>
#include "velox/common/base/tests/GTestUtils.h"
#include "velox/functions/prestosql/tests/utils/FunctionBaseTest.h"

using facebook::velox::functions::test::FunctionBaseTest;

class GeometryFunctionsTest : public FunctionBaseTest {
 public:
  // A set of geometries such that:
  // 0, 1: Within (1, 0: Contains)
  // 0, 2: Touches
  // 1, 2: Overlaps
  // 0, 3: Touches
  // 1, 3: Crosses
  // 1, 4: Touches
  // 1, 5: Touches
  // 2, 3: Contains
  // 2, 4: Crosses
  // 2, 5: Crosses
  // 3, 4: Crosses
  // 3, 5: Touches
  // 4, 5: Contains
  // 1, 6: Contains
  // 2, 6: Contains
  // 1, 7: Touches
  // 2, 7: Contains
  // 3, 6: Contains
  // 3, 7: Contains
  // 4, 7: Contains
  // 5, 7: Touches
  static constexpr std::array<std::string_view, 8> kRelationGeometriesWKT = {
      "POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))", // 0
      "POLYGON ((0 0, 0 2, 2 2, 2 0, 0 0))", // 1
      "POLYGON ((1 0, 1 1, 3 1, 3 0, 1 0))", // 2
      "LINESTRING (1 0.5, 2.5 0.5)", // 3
      "LINESTRING (2 0, 2 2)", // 4
      "LINESTRING (2 0.5, 2 2)", // 5
      "POINT (1.5 0.5)", // 6
      "POINT (2 0.5)" // 7
  };

  void assertRelation(
      std::string_view relation,
      std::optional<std::string_view> leftWkt,
      std::optional<std::string_view> rightWkt,
      bool expected) {
    std::optional<bool> actual = evaluateOnce<bool>(
        fmt::format(
            "{}(ST_GeometryFromText(c0), ST_GeometryFromText(c1))", relation),
        leftWkt,
        rightWkt);
    if (leftWkt.has_value() && rightWkt.has_value()) {
      EXPECT_TRUE(actual.has_value());
      EXPECT_EQ(actual.value(), expected);
    } else {
      EXPECT_FALSE(actual.has_value());
    }
  };

  void assertOverlay(
      std::string_view overlay,
      std::optional<std::string_view> leftWkt,
      std::optional<std::string_view> rightWkt,
      std::optional<std::string_view> expectedWkt) {
    // We are forced to make expectedWkt optional based on type signature, but
    // we always want to supply a value.
    std::optional<bool> actual = evaluateOnce<bool>(
        fmt::format(
            "ST_Equals({}(ST_GeometryFromText(c0), ST_GeometryFromText(c1)), ST_GeometryFromText(c2))",
            overlay),
        leftWkt,
        rightWkt,
        expectedWkt);
    if (leftWkt.has_value() && rightWkt.has_value()) {
      assert(expectedWkt.has_value());
      EXPECT_TRUE(actual.has_value());
      EXPECT_TRUE(actual.value());
    } else {
      EXPECT_FALSE(actual.has_value());
    }
  }
};

TEST_F(GeometryFunctionsTest, wktAndWkb) {
  const auto wktRoundTrip = [&](const std::optional<std::string>& a) {
    return evaluateOnce<std::string>("ST_AsText(ST_GeometryFromText(c0))", a);
  };

  const auto wktToWkb = [&](const std::optional<std::string>& wkt) {
    return evaluateOnce<std::string>(
        "to_hex(ST_AsBinary(ST_GeometryFromText(c0)))", wkt);
  };

  const auto wkbToWkT = [&](const std::optional<std::string>& wkb) {
    return evaluateOnce<std::string>(
        "ST_AsText(ST_GeomFromBinary(from_hex(c0)))", wkb);
  };

  const auto wkbRoundTrip = [&](const std::optional<std::string>& wkt) {
    return evaluateOnce<std::string>(
        "to_hex(ST_AsBinary(ST_GeomFromBinary(from_hex(c0))))", wkt);
  };

  const std::vector<std::string> wkts = {
      "POINT (1 2)",
      "LINESTRING (0 0, 10 10)",
      "POLYGON ((0 0, 0 5, 5 5, 5 0, 0 0))",
      "POLYGON ((0 0, 0 5, 5 5, 5 0, 0 0), (1 1, 4 1, 4 4, 1 4, 1 1))",
      "MULTIPOINT (1 2, 3 4)",
      "MULTILINESTRING ((0 0, 1 1), (2 2, 3 3))",
      "MULTIPOLYGON (((0 0, 0 1, 1 1, 1 0, 0 0)), ((2 2, 2 3, 3 3, 3 2, 2 2)))",
      "GEOMETRYCOLLECTION (POINT (1 2), LINESTRING (3 4, 5 6))"};

  const std::vector<std::string> wkbs = {
      "0101000000000000000000F03F0000000000000040",
      "0102000000020000000000000000000000000000000000000000000000000024400000000000002440",
      "010300000001000000050000000000000000000000000000000000000000000000000000000000000000001440000000000000144000000000000014400000000000001440000000000000000000000000000000000000000000000000",
      "01030000000200000005000000000000000000000000000000000000000000000000000000000000000000144000000000000014400000000000001440000000000000144000000000000000000000000000000000000000000000000005000000000000000000F03F000000000000F03F0000000000001040000000000000F03F00000000000010400000000000001040000000000000F03F0000000000001040000000000000F03F000000000000F03F",
      "0104000000020000000101000000000000000000F03F0000000000000040010100000000000000000008400000000000001040",
      "01050000000200000001020000000200000000000000000000000000000000000000000000000000F03F000000000000F03F0102000000020000000000000000000040000000000000004000000000000008400000000000000840",
      "01060000000200000001030000000100000005000000000000000000000000000000000000000000000000000000000000000000F03F000000000000F03F000000000000F03F000000000000F03F000000000000000000000000000000000000000000000000010300000001000000050000000000000000000040000000000000004000000000000000400000000000000840000000000000084000000000000008400000000000000840000000000000004000000000000000400000000000000040",
      "0107000000020000000101000000000000000000F03F00000000000000400102000000020000000000000000000840000000000000104000000000000014400000000000001840"};

  const std::vector<std::string> bigEndianWkbs = {
      "00000000013FF00000000000004000000000000000",
      "0000000002000000020000000000000000000000000000000040240000000000004024000000000000",
      "000000000300000001000000050000000000000000000000000000000000000000000000004014000000000000401400000000000040140000000000004014000000000000000000000000000000000000000000000000000000000000",
      "000000000300000002000000050000000000000000000000000000000000000000000000004014000000000000401400000000000040140000000000004014000000000000000000000000000000000000000000000000000000000000000000053ff00000000000003ff000000000000040100000000000003ff0000000000000401000000000000040100000000000003ff000000000000040100000000000003ff00000000000003ff0000000000000",
      "00000000040000000200000000013ff00000000000004000000000000000000000000140080000000000004010000000000000",
      "000000000500000002000000000200000002000000000000000000000000000000003ff00000000000003ff00000000000000000000002000000024000000000000000400000000000000040080000000000004008000000000000",
      "000000000600000002000000000300000001000000050000000000000000000000000000000000000000000000003ff00000000000003ff00000000000003ff00000000000003ff0000000000000000000000000000000000000000000000000000000000000000000000300000001000000054000000000000000400000000000000040000000000000004008000000000000400800000000000040080000000000004008000000000000400000000000000040000000000000004000000000000000",
      "00000000070000000200000000013ff000000000000040000000000000000000000002000000024008000000000000401000000000000040140000000000004018000000000000",
  };

  for (size_t i = 0; i < wkts.size(); i++) {
    assert(i < wkbs.size() && i < bigEndianWkbs.size());
    EXPECT_EQ(wkts[i], wktRoundTrip(wkts[i]));
    EXPECT_EQ(wkbs[i], wktToWkb(wkts[i]));
    EXPECT_EQ(wkts[i], wkbToWkT(wkbs[i]));
    EXPECT_EQ(wkbs[i], wkbRoundTrip(wkbs[i]));

    EXPECT_EQ(wkbs[i], wkbRoundTrip(bigEndianWkbs[i]));
    EXPECT_EQ(wkts[i], wkbToWkT(bigEndianWkbs[i]));
  }

  const std::vector<std::string> emptyGeometryWkts = {
      "POINT EMPTY",
      "LINESTRING EMPTY",
      "POLYGON EMPTY",
      "MULTIPOINT EMPTY",
      "MULTILINESTRING EMPTY",
      "MULTIPOLYGON EMPTY",
      "GEOMETRYCOLLECTION EMPTY"};

  const std::vector<std::string> emptyGeometryWkbs = {
      "0101000000000000000000F87F000000000000F87F",
      "010200000000000000",
      "010300000000000000",
      "010400000000000000",
      "010500000000000000",
      "010600000000000000",
      "010700000000000000"};

  for (size_t i = 0; i < emptyGeometryWkts.size(); i++) {
    assert(i < emptyGeometryWkbs.size());
    EXPECT_EQ(wktRoundTrip(emptyGeometryWkts[i]), emptyGeometryWkts[i]);
    EXPECT_EQ(emptyGeometryWkbs[i], wktToWkb(emptyGeometryWkts[i]));
    EXPECT_EQ(emptyGeometryWkts[i], wkbToWkT(emptyGeometryWkbs[i]));
    EXPECT_EQ(emptyGeometryWkbs[i], wkbRoundTrip(emptyGeometryWkbs[i]));
  }

  // WKT invalid cases
  VELOX_ASSERT_USER_THROW(
      wktRoundTrip(""), "Expected word but encountered end of stream");
  VELOX_ASSERT_USER_THROW(
      wktRoundTrip("RANDOM_TEXT"), "Unknown type: 'RANDOM_TEXT'");
  VELOX_ASSERT_USER_THROW(
      wktRoundTrip("LINESTRING (1 1)"),
      "point array must contain 0 or >1 elements");
  VELOX_ASSERT_USER_THROW(
      wktRoundTrip("LINESTRING ()"), "Expected number but encountered ')'");
  VELOX_ASSERT_USER_THROW(
      wktRoundTrip("POLYGON ((0 0, 0 0))"),
      "Invalid number of points in LinearRing found 2 - must be 0 or >= 4");
  VELOX_ASSERT_USER_THROW(
      wktRoundTrip("POLYGON ((0 0, 0 1, 1 1, 1 0))"),
      "Points of LinearRing do not form a closed linestring");

  // WKB invalid cases
  // Empty
  VELOX_ASSERT_USER_THROW(wkbRoundTrip(""), "Unexpected EOF parsing WKB");

  // Random bytes
  VELOX_ASSERT_USER_THROW(wkbRoundTrip("ABCDEF"), "Unexpected EOF parsing WKB");

  // Unrecognized geometry type
  VELOX_ASSERT_USER_THROW(
      wkbRoundTrip("0109000000000000000000F03F0000000000000040"),
      "Unknown WKB type 9");

  // Point with missing y
  VELOX_ASSERT_USER_THROW(
      wkbRoundTrip("0101000000000000000000F03F"), "Unexpected EOF parsing WKB");

  // LineString with only one point
  VELOX_ASSERT_THROW(
      wkbRoundTrip("010200000001000000000000000000F03F000000000000F03F"),
      "point array must contain 0 or >1 elements");

  // Polygon with unclosed LinString
  VELOX_ASSERT_THROW(
      wkbRoundTrip(
          "01030000000100000004000000000000000000000000000000000000000000000000000000000000000000F03F000000000000F03F000000000000F03F000000000000F03F0000000000000000"),
      "Points of LinearRing do not form a closed linestring");

  VELOX_ASSERT_THROW(
      wkbRoundTrip(
          "010300000001000000020000000000000000000000000000000000000000000000000000000000000000000000"),
      "Invalid number of points in LinearRing found 2 - must be 0 or >= 4");
}

// Relationship predicates

TEST_F(GeometryFunctionsTest, testStRelate) {
  const auto assertStRelate =
      [&](std::optional<std::string_view> leftWkt,
          std::optional<std::string_view> rightWkt,
          std::optional<std::string_view> relateCondition,
          bool expected) {
        std::optional<bool> actual = evaluateOnce<bool>(
            "ST_Relate(ST_GeometryFromText(c0), ST_GeometryFromText(c1), c2)",
            leftWkt,
            rightWkt,
            relateCondition);
        if (leftWkt.has_value() && rightWkt.has_value() &&
            relateCondition.has_value()) {
          EXPECT_TRUE(actual.has_value());
          EXPECT_EQ(actual.value(), expected);
        } else {
          EXPECT_FALSE(actual.has_value());
        }
      };

  assertStRelate(
      "LINESTRING (0 0, 3 3)", "LINESTRING (1 1, 4 1)", "****T****", false);
  assertStRelate(
      "POLYGON ((2 0, 2 1, 3 1, 2 0))",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "****T****",
      true);
  assertStRelate(
      "POLYGON ((2 0, 2 1, 3 1, 2 0))",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "T********",
      false);
  assertStRelate(std::nullopt, std::nullopt, std::nullopt, false);
}

TEST_F(GeometryFunctionsTest, testStContains) {
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[1],
      kRelationGeometriesWKT[0],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[2],
      kRelationGeometriesWKT[3],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[4],
      kRelationGeometriesWKT[5],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[1],
      kRelationGeometriesWKT[6],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[2],
      kRelationGeometriesWKT[6],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[2],
      kRelationGeometriesWKT[7],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[3],
      kRelationGeometriesWKT[6],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[3],
      kRelationGeometriesWKT[7],
      true);
  assertRelation(
      "ST_Contains",
      kRelationGeometriesWKT[4],
      kRelationGeometriesWKT[7],
      true);

  assertRelation("ST_Contains", std::nullopt, "POINT (25 25)", false);
  assertRelation("ST_Contains", "POINT (20 20)", "POINT (25 25)", false);
  assertRelation(
      "ST_Contains", "MULTIPOINT (20 20, 25 25)", "POINT (25 25)", true);
  assertRelation(
      "ST_Contains", "LINESTRING (20 20, 30 30)", "POINT (25 25)", true);
  assertRelation(
      "ST_Contains",
      "LINESTRING (20 20, 30 30)",
      "MULTIPOINT (25 25, 31 31)",
      false);
  assertRelation(
      "ST_Contains",
      "LINESTRING (20 20, 30 30)",
      "LINESTRING (25 25, 27 27)",
      true);
  assertRelation(
      "ST_Contains",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 4 4), (2 1, 6 1))",
      false);
  assertRelation(
      "ST_Contains",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      "POLYGON ((1 1, 1 2, 2 2, 2 1, 1 1))",
      true);
  assertRelation(
      "ST_Contains",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      "POLYGON ((-1 -1, -1 2, 2 2, 2 -1, -1 -1))",
      false);
  assertRelation(
      "ST_Contains",
      "MULTIPOLYGON (((0 0, 0 2, 2 2, 2 0, 0 0)), ((2 2, 2 4, 4 4, 4 2, 2 2)))",
      "POLYGON ((2 2, 2 3, 3 3, 3 2, 2 2))",
      true);
  assertRelation(
      "ST_Contains",
      "LINESTRING (20 20, 30 30)",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      false);
  assertRelation(
      "ST_Contains",
      "LINESTRING EMPTY",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      false);
  assertRelation(
      "ST_Contains", "LINESTRING (20 20, 30 30)", "POLYGON EMPTY", false);

  VELOX_ASSERT_USER_THROW(
      assertRelation(
          "ST_Contains",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT (1 1)",
          false),
      "Failed to check geometry contains: TopologyException: side location conflict at 1 2. This can occur if the input geometry is invalid.");
}

TEST_F(GeometryFunctionsTest, testStCrosses) {
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[1], kRelationGeometriesWKT[3], true);
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[3], kRelationGeometriesWKT[1], true);
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[2], kRelationGeometriesWKT[4], true);
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[4], kRelationGeometriesWKT[2], true);
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[2], kRelationGeometriesWKT[5], true);
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[5], kRelationGeometriesWKT[2], true);
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[3], kRelationGeometriesWKT[4], true);
  assertRelation(
      "ST_Crosses", kRelationGeometriesWKT[4], kRelationGeometriesWKT[3], true);

  assertRelation("ST_Crosses", std::nullopt, "POINT (25 25)", false);
  assertRelation("ST_Crosses", "POINT (20 20)", "POINT (25 25)", false);
  assertRelation(
      "ST_Crosses", "LINESTRING (20 20, 30 30)", "POINT (25 25)", false);
  assertRelation(
      "ST_Crosses",
      "LINESTRING (20 20, 30 30)",
      "MULTIPOINT (25 25, 31 31)",
      true);
  assertRelation(
      "ST_Crosses", "LINESTRING(0 0, 1 1)", "LINESTRING (1 0, 0 1)", true);
  assertRelation(
      "ST_Crosses",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "POLYGON ((2 2, 2 5, 5 5, 5 2, 2 2))",
      false);
  assertRelation(
      "ST_Crosses",
      "MULTIPOLYGON (((0 0, 0 2, 2 2, 2 0, 0 0)), ((2 2, 2 4, 4 4, 4 2, 2 2)))",
      "POLYGON ((2 2, 2 3, 3 3, 3 2, 2 2))",
      false);
  assertRelation(
      "ST_Crosses",
      "LINESTRING (-2 -2, 6 6)",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      true);
  assertRelation("ST_Crosses", "POINT (20 20)", "POINT (20 20)", false);
  assertRelation(
      "ST_Crosses",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      false);
  assertRelation(
      "ST_Crosses",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      "LINESTRING (0 0, 0 4, 4 4, 4 0)",
      false);

  VELOX_ASSERT_USER_THROW(
      assertRelation(
          "ST_Crosses",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT (1 1)",
          false),
      "Failed to check geometry crosses: TopologyException: side location conflict at 1 2. This can occur if the input geometry is invalid.");
}

TEST_F(GeometryFunctionsTest, testStDisjoint) {
  assertRelation("ST_Disjoint", std::nullopt, "POINT (150 150)", true);
  assertRelation("ST_Disjoint", "POINT (50 100)", "POINT (150 150)", true);
  assertRelation(
      "ST_Disjoint", "MULTIPOINT (50 100, 50 200)", "POINT (50 100)", false);
  assertRelation(
      "ST_Disjoint", "LINESTRING (0 0, 0 1)", "LINESTRING (1 1, 1 0)", true);
  assertRelation(
      "ST_Disjoint", "LINESTRING (2 1, 1 2)", "LINESTRING (3 1, 1 3)", true);
  assertRelation(
      "ST_Disjoint", "LINESTRING (1 1, 3 3)", "LINESTRING (3 1, 1 3)", false);
  assertRelation(
      "ST_Disjoint",
      "LINESTRING (50 100, 50 200)",
      "LINESTRING (20 150, 100 150)",
      false);
  assertRelation(
      "ST_Disjoint",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      false);
  assertRelation(
      "ST_Disjoint",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((4 4, 4 5, 5 5, 5 4, 4 4))",
      true);

  VELOX_ASSERT_USER_THROW(
      assertRelation(
          "ST_Disjoint",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT (1 1)",
          false),
      "Failed to check geometry disjoint: TopologyException: side location conflict at 1 2. This can occur if the input geometry is invalid.");
}

TEST_F(GeometryFunctionsTest, testStEquals) {
  for (const auto& leftWkt : kRelationGeometriesWKT) {
    for (const auto& rightWkt : kRelationGeometriesWKT) {
      assertRelation("ST_Equals", leftWkt, rightWkt, leftWkt == rightWkt);
    }
  }

  assertRelation("ST_Equals", std::nullopt, "POINT (150 150)", false);
  assertRelation("ST_Equals", "POINT (50 100)", "POINT (150 150)", false);
  assertRelation(
      "ST_Equals", "MULTIPOINT (50 100, 50 200)", "POINT (50 100)", false);
  assertRelation(
      "ST_Equals", "LINESTRING (0 0, 0 1)", "LINESTRING (1 1, 1 0)", false);
  assertRelation(
      "ST_Equals", "LINESTRING (0 0, 2 2)", "LINESTRING (0 0, 2 2)", true);
  assertRelation(
      "ST_Equals",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      false);
  assertRelation(
      "ST_Equals",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((3 3, 3 1, 1 1, 1 3, 3 3))",
      true);
  assertRelation(
      "ST_Equals",
      "MULTIPOLYGON (((1 1, 1 3, 3 3, 3 1, 1 1)), ((0 0, 0 2, 2 2, 2 0, 0 0)))",
      "POLYGON ((0 1, 3 1, 3 3, 0 3, 0 1))",
      false);
  // Invalid geometries.  This test might have to change when upgrading GEOS.
  assertRelation(
      "ST_Equals",
      "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
      "LINESTRING (0 0, 1 1, 1 0, 0 1)",
      false);
}

TEST_F(GeometryFunctionsTest, testStIntersects) {
  assertRelation("ST_Intersects", std::nullopt, "POINT (150 150)", false);
  assertRelation("ST_Intersects", "POINT (50 100)", "POINT (150 150)", false);
  assertRelation(
      "ST_Intersects", "MULTIPOINT (50 100, 50 200)", "POINT (50 100)", true);
  assertRelation(
      "ST_Intersects", "LINESTRING (0 0, 0 1)", "LINESTRING (1 1, 1 0)", false);
  assertRelation(
      "ST_Intersects",
      "LINESTRING (50 100, 50 200)",
      "LINESTRING (20 150, 100 150)",
      true);
  assertRelation(
      "ST_Intersects",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      true);
  assertRelation(
      "ST_Intersects",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((4 4, 4 5, 5 5, 5 4, 4 4))",
      false);
  assertRelation(
      "ST_Intersects",
      "MULTIPOLYGON (((1 1, 1 3, 3 3, 3 1, 1 1)), ((0 0, 0 2, 2 2, 2 0, 0 0)))",
      "POLYGON ((0 1, 3 1, 3 3, 0 3, 0 1))",
      true);
  assertRelation(
      "ST_Intersects",
      "POLYGON ((16.5 54, 16.5 54.1, 16.51 54.1, 16.8 54, 16.5 54))",
      "LINESTRING (16.6 53, 16.6 56)",
      true);
  assertRelation(
      "ST_Intersects",
      "POLYGON ((16.5 54, 16.5 54.1, 16.51 54.1, 16.8 54, 16.5 54))",
      "LINESTRING (16.6667 54.05, 16.8667 54.05)",
      false);
  assertRelation(
      "ST_Intersects",
      "POLYGON ((16.5 54, 16.5 54.1, 16.51 54.1, 16.8 54, 16.5 54))",
      "LINESTRING (16.6667 54.25, 16.8667 54.25)",
      false);

  VELOX_ASSERT_USER_THROW(
      assertRelation(
          "ST_Intersects",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT (1 1)",
          false),
      "Failed to check geometry intersects: TopologyException: side location conflict at 1 2. This can occur if the input geometry is invalid.");
}

TEST_F(GeometryFunctionsTest, testStOverlaps) {
  assertRelation(
      "ST_Overlaps",
      kRelationGeometriesWKT[1],
      kRelationGeometriesWKT[2],
      true);
  assertRelation(
      "ST_Overlaps",
      kRelationGeometriesWKT[2],
      kRelationGeometriesWKT[1],
      true);

  assertRelation("ST_Overlaps", std::nullopt, "POINT (150 150)", false);
  assertRelation("ST_Overlaps", "POINT (50 100)", "POINT (150 150)", false);
  assertRelation("ST_Overlaps", "POINT (50 100)", "POINT (50 100)", false);
  assertRelation(
      "ST_Overlaps", "MULTIPOINT (50 100, 50 200)", "POINT (50 100)", false);
  assertRelation(
      "ST_Overlaps", "LINESTRING (0 0, 0 1)", "LINESTRING (1 1, 1 0)", false);
  assertRelation(
      "ST_Overlaps",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      true);
  assertRelation(
      "ST_Overlaps",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "POLYGON ((3 3, 3 5, 5 5, 5 3, 3 3))",
      true);
  assertRelation(
      "ST_Overlaps",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      false);
  assertRelation(
      "ST_Overlaps",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "LINESTRING (1 1, 4 4)",
      false);
  assertRelation(
      "ST_Overlaps",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((4 4, 4 5, 5 5, 5 4, 4 4))",
      false);

  VELOX_ASSERT_USER_THROW(
      assertRelation(
          "ST_Overlaps",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT (1 1)",
          false),
      "Failed to check geometry overlaps: TopologyException: side location conflict at 1 2. This can occur if the input geometry is invalid.");
}

TEST_F(GeometryFunctionsTest, testStTouches) {
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[0], kRelationGeometriesWKT[2], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[2], kRelationGeometriesWKT[0], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[0], kRelationGeometriesWKT[3], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[3], kRelationGeometriesWKT[0], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[1], kRelationGeometriesWKT[4], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[4], kRelationGeometriesWKT[1], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[1], kRelationGeometriesWKT[5], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[5], kRelationGeometriesWKT[1], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[3], kRelationGeometriesWKT[5], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[5], kRelationGeometriesWKT[3], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[1], kRelationGeometriesWKT[7], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[7], kRelationGeometriesWKT[1], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[5], kRelationGeometriesWKT[7], true);
  assertRelation(
      "ST_Touches", kRelationGeometriesWKT[7], kRelationGeometriesWKT[5], true);

  assertRelation("ST_Touches", std::nullopt, "POINT (150 150)", false);
  assertRelation("ST_Touches", "POINT (50 100)", "POINT (150 150)", false);
  assertRelation(
      "ST_Touches", "MULTIPOINT (50 100, 50 200)", "POINT (50 100)", false);
  assertRelation(
      "ST_Touches",
      "LINESTRING (50 100, 50 200)",
      "LINESTRING (20 150, 100 150)",
      false);
  assertRelation(
      "ST_Touches",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      false);
  assertRelation(
      "ST_Touches", "POINT (1 2)", "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))", true);
  assertRelation(
      "ST_Touches",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((4 4, 4 5, 5 5, 5 4, 4 4))",
      false);
  assertRelation(
      "ST_Touches",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "LINESTRING (0 0, 1 1)",
      true);
  assertRelation(
      "ST_Touches",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((3 3, 3 5, 5 5, 5 3, 3 3))",
      true);

  VELOX_ASSERT_USER_THROW(
      assertRelation(
          "ST_Touches",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT (1 1)",
          false),
      "Failed to check geometry touches: TopologyException: side location conflict at 1 2. This can occur if the input geometry is invalid.");
}

TEST_F(GeometryFunctionsTest, testStWithin) {
  // 0, 1: Within (1, 0: Contains)
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[0], kRelationGeometriesWKT[1], true);
  // 2, 3: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[3], kRelationGeometriesWKT[2], true);
  // 4, 5: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[5], kRelationGeometriesWKT[4], true);
  // 1, 6: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[6], kRelationGeometriesWKT[1], true);
  // 2, 6: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[6], kRelationGeometriesWKT[2], true);
  // 2, 7: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[7], kRelationGeometriesWKT[2], true);
  // 3, 6: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[6], kRelationGeometriesWKT[3], true);
  // 3, 7: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[7], kRelationGeometriesWKT[3], true);
  // 4, 7: Contains
  assertRelation(
      "ST_Within", kRelationGeometriesWKT[7], kRelationGeometriesWKT[4], true);

  assertRelation("ST_Within", std::nullopt, "POINT (150 150)", false);
  assertRelation("ST_Within", "POINT (50 100)", "POINT (150 150)", false);
  assertRelation(
      "ST_Within", "POINT (50 100)", "MULTIPOINT (50 100, 50 200)", true);
  assertRelation(
      "ST_Within",
      "LINESTRING (50 100, 50 200)",
      "LINESTRING (50 50, 50 250)",
      true);
  assertRelation(
      "ST_Within",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      false);
  assertRelation(
      "ST_Within", "POINT (3 2)", "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))", true);
  assertRelation(
      "ST_Within",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      true);
  assertRelation(
      "ST_Within",
      "LINESTRING (1 1, 3 3)",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      true);
  assertRelation(
      "ST_Within",
      "MULTIPOLYGON (((1 1, 1 3, 3 3, 3 1, 1 1)), ((0 0, 0 2, 2 2, 2 0, 0 0)))",
      "POLYGON ((0 1, 3 1, 3 3, 0 3, 0 1))",
      false);
  assertRelation(
      "ST_Within",
      "POLYGON ((1 1, 1 5, 5 5, 5 1, 1 1))",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      false);

  VELOX_ASSERT_USER_THROW(
      assertRelation(
          "ST_Within",
          "POINT (0 0)",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          false),
      "Failed to check geometry within: TopologyException: side location conflict at 1 2. This can occur if the input geometry is invalid.");
}

// Overlay operations

TEST_F(GeometryFunctionsTest, testStDifference) {
  assertOverlay("ST_Difference", std::nullopt, std::nullopt, std::nullopt);
  assertOverlay(
      "ST_Difference", "POINT (50 100)", "POINT (150 150)", "POINT (50 100)");
  assertOverlay(
      "ST_Difference",
      "MULTIPOINT (50 100, 50 200)",
      "POINT (50 100)",
      "POINT (50 200)");
  assertOverlay(
      "ST_Difference",
      "LINESTRING (50 100, 50 200)",
      "LINESTRING (50 50, 50 150)",
      "LINESTRING (50 150, 50 200)");
  assertOverlay(
      "ST_Difference",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((2 1, 4 1), (3 3, 7 3))",
      "MULTILINESTRING ((1 1, 2 1), (4 1, 5 1), (2 4, 4 4))");
  assertOverlay(
      "ST_Difference",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "POLYGON ((2 2, 2 5, 5 5, 5 2, 2 2))",
      "POLYGON ((1 4, 2 4, 2 2, 4 2, 4 1, 1 1, 1 4))");
  assertOverlay(
      "ST_Difference",
      "MULTIPOLYGON (((1 1, 1 3, 3 3, 3 1, 1 1)), ((0 0, 0 1, 1 1, 1 0, 0 0)))",
      "POLYGON ((0 1, 3 1, 3 3, 0 3, 0 1))",
      "POLYGON ((0 1, 1 1, 1 0, 0 0, 0 1))");

  VELOX_ASSERT_USER_THROW(
      assertOverlay(
          "ST_Difference",
          "LINESTRING (0 0, 1 1, 1 0, 0 1)",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT EMPTY"),
      "Failed to compute geometry difference: TopologyException: Input geom 1 is invalid: Self-intersection at 1 2");
}

TEST_F(GeometryFunctionsTest, testStIntersection) {
  assertOverlay("ST_Intersection", std::nullopt, std::nullopt, std::nullopt);
  assertOverlay(
      "ST_Intersection", "POINT (50 100)", "POINT (150 150)", "POINT EMPTY");
  assertOverlay(
      "ST_Intersection",
      "MULTIPOINT (50 100, 50 200)",
      "POINT (50 100)",
      "POINT (50 100)");
  assertOverlay(
      "ST_Intersection",
      "LINESTRING (50 100, 50 200)",
      "LINESTRING (20 150, 100 150)",
      "POINT (50 150)");
  assertOverlay(
      "ST_Intersection",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      "GEOMETRYCOLLECTION (LINESTRING (3 4, 4 4), POINT (5 1))");
  assertOverlay(
      "ST_Intersection",
      "POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1))",
      "POLYGON ((4 4, 4 5, 5 5, 5 4, 4 4))",
      "POLYGON EMPTY");
  assertOverlay(
      "ST_Intersection",
      "MULTIPOLYGON (((1 1, 1 3, 3 3, 3 1, 1 1)), ((0 0, 0 1, 1 1, 1 0, 0 0)))",
      "POLYGON ((0 1, 3 1, 3 3, 0 3, 0 1))",
      "GEOMETRYCOLLECTION (POLYGON ((1 3, 3 3, 3 1, 1 1, 1 3)), LINESTRING (0 1, 1 1))");
  assertOverlay(
      "ST_Intersection",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "LINESTRING (2 0, 2 3)",
      "LINESTRING (2 1, 2 3)");
  assertOverlay(
      "ST_Intersection",
      "POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))",
      "LINESTRING (0 0, 1 -1, 1 2)",
      "GEOMETRYCOLLECTION (LINESTRING (1 1, 1 0), POINT (0 0))");

  VELOX_ASSERT_USER_THROW(
      assertOverlay(
          "ST_Intersection",
          "LINESTRING (0 0, 1 1, 1 0, 0 1)",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT EMPTY"),
      "Failed to compute geometry intersection: TopologyException: Input geom 1 is invalid: Self-intersection at 1 2");
}

TEST_F(GeometryFunctionsTest, testStSymDifference) {
  assertOverlay("ST_SymDifference", std::nullopt, std::nullopt, std::nullopt);
  assertOverlay(
      "ST_SymDifference",
      "POINT (50 100)",
      "POINT (50 150)",
      "MULTIPOINT (50 100, 50 150)");
  assertOverlay(
      "ST_SymDifference",
      "MULTIPOINT (50 100, 60 200)",
      "MULTIPOINT (60 200, 70 150)",
      "MULTIPOINT (50 100, 70 150)");
  assertOverlay(
      "ST_SymDifference",
      "LINESTRING (50 100, 50 200)",
      "LINESTRING (50 50, 50 150)",
      "MULTILINESTRING ((50 150, 50 200), (50 50, 50 100))");
  assertOverlay(
      "ST_SymDifference",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 4 4))",
      "MULTILINESTRING ((3 4, 6 4), (5 0, 5 4))",
      "MULTILINESTRING ((1 1, 5 1), (2 4, 3 4), (4 4, 5 4), (5 4, 6 4), (5 0, 5 1), (5 1, 5 4))");
  assertOverlay(
      "ST_SymDifference",
      "POLYGON ((1 1, 1 4, 4 4, 4 1, 1 1))",
      "POLYGON ((2 2, 2 5, 5 5, 5 2, 2 2))",
      "MULTIPOLYGON (((1 4, 2 4, 2 2, 4 2, 4 1, 1 1, 1 4)), ((4 4, 2 4, 2 5, 5 5, 5 2, 4 2, 4 4)))");
  assertOverlay(
      "ST_SymDifference",
      "MULTIPOLYGON (((0 0, 0 2, 2 2, 2 0, 0 0)), ((2 2, 2 4, 4 4, 4 2, 2 2)))",
      "POLYGON ((0 0, 0 3, 3 3, 3 0, 0 0))",
      "MULTIPOLYGON (((0 2, 0 3, 2 3, 2 2, 0 2)), ((2 2, 3 2, 3 0, 2 0, 2 2)), ((2 4, 4 4, 4 2, 3 2, 3 3, 2 3, 2 4)))");

  VELOX_ASSERT_USER_THROW(
      assertOverlay(
          "ST_SymDifference",
          "LINESTRING (0 0, 1 1, 1 0, 0 1)",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT EMPTY"),
      "Failed to compute geometry symdifference: TopologyException: Input geom 1 is invalid: Self-intersection at 1 2");
}

TEST_F(GeometryFunctionsTest, testStUnion) {
  std::array<std::string_view, 7> emptyWkts = {
      "POINT EMPTY",
      "MULTIPOINT EMPTY",
      "LINESTRING EMPTY",
      "MULTILINESTRING EMPTY",
      "POLYGON EMPTY",
      "MULTIPOLYGON EMPTY",
      "GEOMETRYCOLLECTION EMPTY"};
  std::array<std::string_view, 7> simpleWkts = {
      "POINT (1 2)",
      "MULTIPOINT (1 2, 3 4)",
      "LINESTRING (0 0, 2 2, 4 4)",
      "MULTILINESTRING ((0 0, 2 2, 4 4), (5 5, 7 7, 9 9))",
      "POLYGON ((0 1, 1 1, 1 0, 0 0, 0 1))",
      "MULTIPOLYGON (((1 1, 1 3, 3 3, 3 1, 1 1)), ((2 4, 2 6, 6 6, 6 4, 2 4)))",
      "GEOMETRYCOLLECTION (LINESTRING (0 5, 5 5), POLYGON ((1 1, 1 3, 3 3, 3 1, 1 1)))"};

  // empty geometry
  for (std::string_view emptyWkt : emptyWkts) {
    for (std::string_view simpleWkt : simpleWkts) {
      assertOverlay("ST_Union", emptyWkt, simpleWkt, simpleWkt);
    }
  }

  // self union
  for (std::string_view simpleWkt : simpleWkts) {
    assertOverlay("ST_Union", simpleWkt, simpleWkt, simpleWkt);
  }

  assertOverlay("ST_Union", std::nullopt, std::nullopt, std::nullopt);

  // touching union
  assertOverlay(
      "ST_Union",
      "POINT (1 2)",
      "MULTIPOINT (1 2, 3 4)",
      "MULTIPOINT (1 2, 3 4)");
  assertOverlay(
      "ST_Union",
      "MULTIPOINT (1 2)",
      "MULTIPOINT (1 2, 3 4)",
      "MULTIPOINT (1 2, 3 4)");
  assertOverlay(
      "ST_Union",
      "LINESTRING (0 1, 1 2)",
      "LINESTRING (1 2, 3 4)",
      "LINESTRING (0 1, 1 2, 3 4)");
  assertOverlay(
      "ST_Union",
      "MULTILINESTRING ((0 0, 2 2, 4 4), (5 5, 7 7, 9 9))",
      "MULTILINESTRING ((5 5, 7 7, 9 9), (11 11, 13 13, 15 15))",
      "MULTILINESTRING ((0 0, 2 2, 4 4), (5 5, 7 7, 9 9), (11 11, 13 13, 15 15))");
  assertOverlay(
      "ST_Union",
      "POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0))",
      "POLYGON ((1 0, 2 0, 2 1, 1 1, 1 0))",
      "POLYGON ((0 0, 0 1, 1 1, 2 1, 2 0, 1 0, 0 0))");
  assertOverlay(
      "ST_Union",
      "MULTIPOLYGON (((0 0, 0 1, 1 1, 1 0, 0 0)))",
      "MULTIPOLYGON (((1 0, 2 0, 2 1, 1 1, 1 0)))",
      "POLYGON ((0 0, 0 1, 1 1, 2 1, 2 0, 1 0, 0 0))");
  assertOverlay(
      "ST_Union",
      "GEOMETRYCOLLECTION (POLYGON ((0 0, 0 1, 1 1, 1 0, 0 0)), POINT (1 2))",
      "GEOMETRYCOLLECTION (POLYGON ((1 0, 2 0, 2 1, 1 1, 1 0)), MULTIPOINT ((1 2), (3 4)))",
      "GEOMETRYCOLLECTION (POINT (1 2), POINT (3 4), POLYGON ((0 0, 0 1, 1 1, 2 1, 2 0, 1 0, 0 0)))");

  // within union
  assertOverlay(
      "ST_Union",
      "MULTIPOINT (20 20, 25 25)",
      "POINT (25 25)",
      "MULTIPOINT (20 20, 25 25)");
  assertOverlay(
      "ST_Union",
      "LINESTRING (20 20, 30 30)",
      "POINT (25 25)",
      "LINESTRING (20 20, 30 30)");
  assertOverlay(
      "ST_Union",
      "LINESTRING (20 20, 30 30)",
      "LINESTRING (25 25, 27 27)",
      "LINESTRING (20 20, 25 25, 27 27, 30 30)");
  assertOverlay(
      "ST_Union",
      "POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0))",
      "POLYGON ((1 1, 1 2, 2 2, 2 1, 1 1))",
      "POLYGON ((0 4, 4 4, 4 0, 0 0, 0 4))");
  assertOverlay(
      "ST_Union",
      "MULTIPOLYGON (((0 0, 0 2, 2 2, 2 0, 0 0)), ((2 2, 2 4, 4 4, 4 2, 2 2)))",
      "POLYGON ((2 2, 2 3, 3 3, 3 2, 2 2))",
      "MULTIPOLYGON (((2 2, 2 3, 2 4, 4 4, 4 2, 3 2, 2 2)), ((0 0, 0 2, 2 2, 2 0, 0 0)))");
  assertOverlay(
      "ST_Union",
      "GEOMETRYCOLLECTION (POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0)), MULTIPOINT (20 20, 25 25))",
      "GEOMETRYCOLLECTION (POLYGON ((1 1, 1 2, 2 2, 2 1, 1 1)), POINT (25 25))",
      "GEOMETRYCOLLECTION (MULTIPOINT (20 20, 25 25), POLYGON ((0 0, 0 4, 4 4, 4 0, 0 0)))");

  // overlap union
  assertOverlay(
      "ST_Union",
      "LINESTRING (1 1, 3 1)",
      "LINESTRING (2 1, 4 1)",
      "LINESTRING (1 1, 2 1, 3 1, 4 1)");
  assertOverlay(
      "ST_Union",
      "MULTILINESTRING ((1 1, 3 1))",
      "MULTILINESTRING ((2 1, 4 1))",
      "LINESTRING (1 1, 2 1, 3 1, 4 1)");
  assertOverlay(
      "ST_Union",
      "POLYGON ((1 1, 3 1, 3 3, 1 3, 1 1))",
      "POLYGON ((2 2, 4 2, 4 4, 2 4, 2 2))",
      "POLYGON ((1 1, 1 3, 2 3, 2 4, 4 4, 4 2, 3 2, 3 1, 1 1))");
  assertOverlay(
      "ST_Union",
      "MULTIPOLYGON (((1 1, 3 1, 3 3, 1 3, 1 1)))",
      "MULTIPOLYGON (((2 2, 4 2, 4 4, 2 4, 2 2)))",
      "POLYGON ((1 1, 1 3, 2 3, 2 4, 4 4, 4 2, 3 2, 3 1, 1 1))");
  assertOverlay(
      "ST_Union",
      "GEOMETRYCOLLECTION (POLYGON ((1 1, 3 1, 3 3, 1 3, 1 1)), LINESTRING (1 1, 3 1))",
      "GEOMETRYCOLLECTION (POLYGON ((2 2, 4 2, 4 4, 2 4, 2 2)), LINESTRING (2 1, 4 1))",
      "GEOMETRYCOLLECTION (LINESTRING (3 1, 4 1), POLYGON ((1 1, 1 3, 2 3, 2 4, 4 4, 4 2, 3 2, 3 1, 2 1, 1 1)))");

  VELOX_ASSERT_USER_THROW(
      assertOverlay(
          "ST_Union",
          "LINESTRING (0 0, 1 1, 1 0, 0 1)",
          "MULTIPOLYGON ( ((0 0, 0 2, 2 2, 2 0, 0 0)), ((1 1, 1 3, 3 3, 3 1, 1 1)) )",
          "POINT EMPTY"),
      "Failed to compute geometry union: TopologyException: Input geom 1 is invalid: Self-intersection at 1 2");
}

TEST_F(GeometryFunctionsTest, testStArea) {
  const auto testStAreaFunc = [&](std::optional<std::string> wkt,
                                  std::optional<double> expectedArea) {
    std::optional<double> result =
        evaluateOnce<double>("ST_Area(ST_GeometryFromText(c0))", wkt);

    if (wkt.has_value()) {
      ASSERT_TRUE(result.has_value());
      ASSERT_TRUE(expectedArea.has_value());
      ASSERT_EQ(result.value(), expectedArea.value());
    } else {
      ASSERT_FALSE(result.has_value());
    }
  };

  testStAreaFunc("POLYGON ((2 2, 2 6, 6 6, 6 2, 2 2))", 16.0);
  testStAreaFunc("POLYGON EMPTY", 0.0);
  testStAreaFunc("LINESTRING (1 4, 2 5)", 0.0);
  testStAreaFunc("LINESTRING EMPTY", 0.0);
  testStAreaFunc("POINT (1 4)", 0.0);
  testStAreaFunc("POINT EMPTY", 0.0);
  testStAreaFunc("GEOMETRYCOLLECTION EMPTY", 0.0);

  // Test basic geometry collection. Area is the area of the polygon.
  testStAreaFunc(
      "GEOMETRYCOLLECTION (POINT (8 8), LINESTRING (5 5, 6 6), POLYGON ((1 1, 3 1, 3 4, 1 4, 1 1)))",
      6.0);

  // Test overlapping geometries. Area is the sum of the individual elements
  testStAreaFunc(
      "GEOMETRYCOLLECTION (POLYGON ((0 0, 2 0, 2 2, 0 2, 0 0)), POLYGON ((1 1, 3 1, 3 3, 1 3, 1 1)))",
      8.0);

  // Test nested geometry collection
  testStAreaFunc(
      "GEOMETRYCOLLECTION (POLYGON ((0 0, 2 0, 2 2, 0 2, 0 0)), POLYGON ((1 1, 3 1, 3 3, 1 3, 1 1)), GEOMETRYCOLLECTION (POINT (8 8), LINESTRING (5 5, 6 6), POLYGON ((1 1, 3 1, 3 4, 1 4, 1 1))))",
      14.0);
}
