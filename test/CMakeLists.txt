# Copyright (c) Christopher Di Bella.
# SPDX-License-Identifier: Apache-2.0
#
set(${PROJECT_NAME}_TEST_FRAMEWORK "Catch2::Catch2" "Catch2::Catch2WithMain" CACHE STRING "")

add_subdirectory(diagnostics)
add_subdirectory(info)
