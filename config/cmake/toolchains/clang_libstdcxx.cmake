# Copyright (c) LLVM Foundation
# SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
#
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR x86_64)

set(TRIPLE x86_64-unknown-linux-gnu)
set(LLVM_ROOT "/usr")
set(CMAKE_C_COMPILER "${LLVM_ROOT}/bin/clang")
set(CMAKE_CXX_COMPILER "${LLVM_ROOT}/bin/clang++")
set(CMAKE_CXX_COMPLIER_TARGET ${TRIPLE})

set(CMAKE_AR "${LLVM_ROOT}/bin/llvm-ar")
set(CMAKE_RC_COMPILER "${LLVM_ROOT}/bin/llvm-rc")
set(CMAKE_RANLIB "${LLVM_ROOT}/bin/llvm-ranlib")

include("${CMAKE_CURRENT_LIST_DIR}/common.cmake")

string(
  JOIN " " CMAKE_CXX_FLAGS
    ${CMAKE_CXX_FLAGS}
    -stdlib=libstdc++
)
