get_filename_component(LLVM_BIN "${CMAKE_CXX_COMPILER}" DIRECTORY)
set(LLVM_TABLEGEN "${LLVM_BIN}/llvm-tblgen")
set(CLANG_TABLEGEN "${LLVM_BIN}/clang-tblgen")
set(CLANG_TABLEGEN_EXE "${CLANG_TABLEGEN}")
find_package(LLVMTableGen REQUIRED)
find_package(ClangTableGen REQUIRED)
cmake_policy(SET CMP0116 OLD)

add_subdirectory(diagnostics)
