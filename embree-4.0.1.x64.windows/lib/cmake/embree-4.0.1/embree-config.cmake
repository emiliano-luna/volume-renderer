## Copyright 2009-2021 Intel Corporation
## SPDX-License-Identifier: Apache-2.0

SET(EMBREE_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../")
GET_FILENAME_COMPONENT(EMBREE_ROOT_DIR "${EMBREE_ROOT_DIR}" ABSOLUTE)

SET(EMBREE_INCLUDE_DIRS "${EMBREE_ROOT_DIR}/include")
SET(EMBREE_LIBRARY "${EMBREE_ROOT_DIR}/lib/embree4.lib")
SET(EMBREE_LIBRARIES ${EMBREE_LIBRARY})

SET(EMBREE_VERSION 4.0.1)
SET(EMBREE_VERSION_MAJOR 4)
SET(EMBREE_VERSION_MINOR 0)
SET(EMBREE_VERSION_PATCH 1)
SET(EMBREE_VERSION_NOTE "")

SET(EMBREE_MAX_ISA NONE)
SET(EMBREE_ISA_SSE2  ON)
SET(EMBREE_ISA_SSE42 ON)
SET(EMBREE_ISA_AVX ON) 
SET(EMBREE_ISA_AVX2  ON)
SET(EMBREE_ISA_AVX512 ON)
SET(EMBREE_ISA_AVX512SKX ON) # just for compatibility
SET(EMBREE_ISA_NEON )

SET(EMBREE_BUILD_TYPE Release)
SET(EMBREE_ISPC_SUPPORT ON)
SET(EMBREE_STATIC_LIB OFF)
SET(EMBREE_SYCL_SUPPORT OFF)
SET(EMBREE_SYCL_GEOMETRY_CALLBACK OFF)
SET(EMBREE_TUTORIALS ON)

SET(EMBREE_RAY_MASK ON)
SET(EMBREE_STAT_COUNTERS OFF)
SET(EMBREE_BACKFACE_CULLING OFF)
SET(EMBREE_FILTER_FUNCTION ON)
SET(EMBREE_IGNORE_INVALID_RAYS OFF)
SET(EMBREE_TASKING_SYSTEM TBB)
SET(EMBREE_TBB_COMPONENT tbb)
SET(EMBREE_COMPACT_POLYS OFF)

SET(EMBREE_GEOMETRY_TRIANGLE ON)
SET(EMBREE_GEOMETRY_QUAD ON)
SET(EMBREE_GEOMETRY_CURVE ON)
SET(EMBREE_GEOMETRY_SUBDIVISION ON)
SET(EMBREE_GEOMETRY_USER ON)
SET(EMBREE_GEOMETRY_POINT ON)

SET(EMBREE_RAY_PACKETS ON)
SET(EMBREE_MAX_INSTANCE_LEVEL_COUNT 1)
SET(EMBREE_CURVE_SELF_INTERSECTION_AVOIDANCE_FACTOR 2.0)
SET(EMBREE_DISC_POINT_SELF_INTERSECTION_AVOIDANCE ON)
SET(EMBREE_MIN_WIDTH OFF)

IF (EMBREE_STATIC_LIB AND (EMBREE_TASKING_SYSTEM STREQUAL "TBB"))
  INCLUDE(CMakeFindDependencyMacro)
  FIND_DEPENDENCY(TBB)
ENDIF()

IF (EMBREE_STATIC_LIB)

  INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/sys-targets.cmake")
  INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/math-targets.cmake")
  INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/simd-targets.cmake")
  INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/lexers-targets.cmake")
  INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/tasking-targets.cmake")

  IF (EMBREE_ISA_SSE42)
    INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree_sse42-targets.cmake")
  ENDIF()

  IF (EMBREE_ISA_AVX)
    INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree_avx-targets.cmake")
  ENDIF()

  IF (EMBREE_ISA_AVX2)
    INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree_avx2-targets.cmake")
  ENDIF()

  IF (EMBREE_ISA_AVX512)
    INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree_avx512-targets.cmake")
  ENDIF()

ENDIF()

IF (EMBREE_SYCL_SUPPORT)

  SET(EMBREE_SYCL_AOT_DEVICES )
  SET(EMBREE_SYCL_LARGEGRF OFF)
  SET(EMBREE_SYCL_RT_VALIDATION_API OFF)
  IF (EMBREE_SYCL_RT_VALIDATION_API)
    INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree_rthwif_sycl-targets.cmake")
  ENDIF()
  IF (EMBREE_STATIC_LIB)
    INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree_rthwif-targets.cmake")
  ENDIF()
  INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree_sycl-targets.cmake")

ENDIF()

INCLUDE("${EMBREE_ROOT_DIR}/lib/cmake/embree-4.0.1/embree-targets.cmake")