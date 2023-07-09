include(FindPackageHandleStandardArgs)

if(EXISTS "${LLVM_TABLEGEN}")
  set(LLVMTableGen_FOUND On)
endif()

find_package_handle_standard_args(LLVMTableGen REQUIRED_VARS LLVMTableGen_FOUND)
