include(FindPackageHandleStandardArgs)

if(EXISTS "${CLANG_TABLEGEN}")
  set(ClangTableGen_FOUND On)
endif()

find_package_handle_standard_args(ClangTableGen REQUIRED_VARS ClangTableGen_FOUND)
