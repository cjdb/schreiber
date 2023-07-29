# Copyright (c) Christopher Di Bella.
# SPDX-License-Identifier: Apache-2.0
#
set(${PROJECT_NAME}_TEST_FRAMEWORK "Catch2::Catch2" "Catch2::Catch2WithMain" CACHE STRING "")

add_subdirectory(info)

include(configure_lit)
configure_lit_site_cfg(
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.site.cfg.py.in
  ${CMAKE_CURRENT_BINARY_DIR}/lit.site.cfg.py
  MAIN_CONFIG
  ${CMAKE_CURRENT_SOURCE_DIR}/lit.cfg.py
  PATHS
  "LLVM_SOURCE_DIR"
  "LLVM_BINARY_DIR"
  "LLVM_TOOLS_DIR"
  "LLVM_LIBS_DIR"
  "SHLIBDIR"
  "LLVM_LIT_TOOLS_DIR"
  "LLVM_EXTERNAL_LIT"
  "CLANG_BINARY_DIR"
  "CLANG_SOURCE_DIR"
  "CLANG_TOOLS_DIR"
  "CMAKE_LIBRARY_OUTPUT_DIRECTORY"
)
