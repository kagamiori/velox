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

#include <string>
#include "velox/functions/Registerer.h"
#include "velox/functions/prestosql/GeometryFunctions.h"
#include "velox/functions/prestosql/types/GeometryRegistration.h"

namespace facebook::velox::functions {

namespace {
void registerRelationPredicates(const std::string& prefix) {
  registerFunction<StRelateFunction, bool, Geometry, Geometry, Varchar>(
      {{prefix + "ST_Relate"}});

  registerFunction<StContainsFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Contains"}});
  registerFunction<StCrossesFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Crosses"}});
  registerFunction<StDisjointFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Disjoint"}});
  registerFunction<StEqualsFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Equals"}});
  registerFunction<StIntersectsFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Intersects"}});
  registerFunction<StOverlapsFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Overlaps"}});
  registerFunction<StTouchesFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Touches"}});
  registerFunction<StWithinFunction, bool, Geometry, Geometry>(
      {{prefix + "ST_Within"}});
}

void registerOverlayOperations(const std::string& prefix) {
  registerFunction<StDifferenceFunction, Geometry, Geometry, Geometry>(
      {{prefix + "ST_Difference"}});
  registerFunction<StIntersectionFunction, Geometry, Geometry, Geometry>(
      {{prefix + "ST_Intersection"}});
  registerFunction<StSymDifferenceFunction, Geometry, Geometry, Geometry>(
      {{prefix + "ST_SymDifference"}});
  registerFunction<StUnionFunction, Geometry, Geometry, Geometry>(
      {{prefix + "ST_Union"}});
}

} // namespace

void registerGeometryFunctions(const std::string& prefix) {
  registerGeometryType();
  registerFunction<StGeometryFromTextFunction, Geometry, Varchar>(
      {{prefix + "ST_GeometryFromText"}});

  registerFunction<StGeomFromBinaryFunction, Geometry, Varbinary>(
      {{prefix + "ST_GeomFromBinary"}});

  registerFunction<StAsTextFunction, Varchar, Geometry>(
      {{prefix + "ST_AsText"}});

  registerFunction<StAsBinaryFunction, Varbinary, Geometry>(
      {{prefix + "ST_AsBinary"}});

  registerFunction<StAreaFunction, double, Geometry>({{prefix + "ST_Area"}});

  registerRelationPredicates(prefix);
  registerOverlayOperations(prefix);
}

} // namespace facebook::velox::functions
