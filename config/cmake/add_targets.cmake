# Copyright (c) Christopher Di Bella.
# SPDX-License-Identifier: Apache-2.0
#

# Extracts builder arguments and prepares them for use. Some of the builders have additional single
# values, which are provided using `extra_single_values`.
macro(extract_target_args extra_single_values)
  set(optional_values "")
  set(single_values
    TARGET
    FILENAME
    WORKING_DIRECTORY
    "${extra_single_values}"
  )
  set(
    multi_values
    FILENAMES
    COMPILE_OPTIONS
    INCLUDE
    DEFINITIONS
    LINK_OPTIONS
    LINK_TARGETS
    LINK_AND_EXPORT_TARGETS
  )
  cmake_parse_arguments(
    add_target_args
    "${optional_values}"
    "${single_values}"
    "${multi_values}"
    ${ARGN}
  )

  if("${add_target_args_TARGET}" STREQUAL "")
    message(FATAL_ERROR "TARGET is missing: we need to provide a target to name our binary.")
  endif()

  if(add_target_args_FILENAME AND add_target_args_FILENAMES)
    message(SEND_ERROR "We're only allowed to set one of FILENAME or FILENAMES but we set both for ${add_target_args_TARGET}.")
  elseif(NOT add_target_args_FILENAME AND NOT add_target_args_FILENAMES)
    message(SEND_ERROR "We need to set exactly one of FILENAME or FILENAMES but we set neither for ${add_target_args_TARGET}.")
  endif()
endmacro()

# Performs the common steps for building a target. This is assembling include directories, compile
# options, and link options.
function(build_common target include_directories definitions compile_options link_targets link_options link_and_export_targets)
  target_include_directories(
    "${target}"
    PUBLIC "${PROJECT_SOURCE_DIR}/include"
    PRIVATE "${include_directories}"
  )
  target_compile_definitions("${target}" PRIVATE "${definitions}")
  target_compile_options(
    "${target}"
    PRIVATE
    $<$<CONFIG:Release>:-flto=thin -Wno-ignored-attributes>
    "${compile_options}")
  target_link_libraries(
    "${target}"
    PRIVATE "${link_targets}"
    PUBLIC "${link_and_export_targets}")
  target_link_options(
    "${target}" PRIVATE
    -fuse-ld=lld
    $<$<CONFIG:Debug>:-fstack-protector-strong>
    $<$<CONFIG:Release>:-flto=thin>
    "${link_options}"
  )
endfunction()

# Builds a C++ program.
# Required arguments:
#   * TARGET target_name: Name of the executable. Required.
#   * FILENAME source_name: Name of exactly one source file to build the target. Required only if
#     FILENAMES is absent.
#   * FILENAMES source_names+: Names of multiple source files to build the target. Required only
#     if FILENAME is absent.
#
# Optional arguments:
#   * COMPILE_OPTIONS options+:  Options that are passed to the compiler.
#   * DEFINITIONS macros+: Macros passed to the compiler. Macros take the form `macro` or
#     `macro=value`.
#   * INCLUDE path+: Paths to directories that the compiler will use in its include paths.
#   * LINK_TARGETS target+: A set of targets to link to the program. Actual linking is
#     determined by CMake: we use this to reference a CMake target rather than an actual binary.
#   * LINK_OPTIONS options+: Options that are passed to the compiler. If options such as
#     `-Wl,--whole-archive` are necessary, pass your CMake target here, rather than to LINK_TARGETS
#     and CMake will resolve it appropriately. The default link options are
#     `-fstack-protector-strong` for debug builds, `-fsanitize=cfi -flto=thin` for release builds,
#     and `-fuse-lld` for all builds.
#   * WORKING_DIRECTORY path: Sets the working directory for the compiler. If nothing is specified,
#     then the directory is the same as where the target has been created.
function(cxx_binary)
  extract_target_args("" ${ARGN})
  add_executable(
    "${add_target_args_TARGET}"
    "${add_target_args_FILENAME}"
    "${add_target_args_FILENAMES}"
  )
  build_common(
    "${add_target_args_TARGET}"
    "${add_target_args_INCLUDE_DIRECTORIES}"
    "${add_target_args_DEFINITIONS}"
    "${add_target_args_COMPILE_OPTIONS}"
    "${add_target_args_LINK_TARGETS}"
    "${add_target_args_LINK_OPTIONS}"
    "${add_target_args_LINK_AND_EXPORT_TARGETS}"
  )
endfunction()

function(check_library_type type)
  if (NOT type)
    return()
  endif()

  set(legal_types "STATIC" "SHARED" "MODULE" "OBJECT")
  list(FIND legal_types "${type}" result)

  if(result EQUAL -1)
    message(FATAL_ERROR "LIBRARY_TYPE is an optional parameter but if set, must be one of STATIC, SHARED, MODULE, or OBJECT.")
  endif()
endfunction()

# Builds a C++ library. Accepts the same options as `cxx_binary`, as well as LIBRARY_TYPE, which is
# a required argument to describe the kind of library being built. Acceptable values include
# STATIC, SHARED, MODULE, and OBJECT. Please see https://cmake.org/cmake/help/latest/command/add_library.html#normal-libraries
# for the definitions of the first three, and https://cmake.org/cmake/help/latest/command/add_library.html#object-libraries
# for the definition of OBJECT.
function(cxx_library)
  extract_target_args("LIBRARY_TYPE" ${ARGN})
  check_library_type("${add_target_args_LIBRARY_TYPE}")
  add_library(
    "${add_target_args_TARGET}"
    "${add_target_args_LIBRARY_TYPE}"
    "${add_target_args_FILENAME}"
    "${add_target_args_FILENAMES}"
  )
  build_common(
    "${add_target_args_TARGET}"
    "${add_target_args_INCLUDE_DIRECTORIES}"
    "${add_target_args_DEFINITIONS}"
    "${add_target_args_COMPILE_OPTIONS}"
    "${add_target_args_LINK_TARGETS}"
    "${add_target_args_LINK_OPTIONS}"
    "${add_target_args_LINK_AND_EXPORT_TARGETS}"
  )

endfunction()

# Builds a test object library and links it to a target called `test_main`, where `test_main` is a
# `cxx_binary` created by the user. Accepts  the same options as `cxx_binary`.
function(cxx_test)
  cxx_binary(${ARGN})
  extract_target_args("" ${ARGN})
  set(target "${add_target_args_TARGET}")
  target_link_libraries("${target}" PRIVATE "${${PROJECT_NAME}_TEST_FRAMEWORK}")
  add_test("test.${target}" "${target}")
endfunction()
