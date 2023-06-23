# find_package(absl CONFIG REQUIRED)
find_package(Catch2 CONFIG REQUIRED)
find_package(LLVM 17 CONFIG REQUIRED)
find_package(Clang 17 CONFIG REQUIRED)
find_package(constexpr-contracts CONFIG REQUIRED)
# find_package(fmt CONFIG REQUIRED)

list(APPEND CMAKE_MODULE_PATH ${LLVM_DIR})
list(APPEND CMAKE_MODULE_PATH ${Clang_DIR})
