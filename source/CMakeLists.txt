include(AddLLVM)
include(AddClang)

cxx_library(
  TARGET info
  FILENAME info.cpp
  LINK_TARGETS cjdb::constexpr-contracts
  LINK_AND_EXPORT_TARGETS
    clangAST
    clangBasic
    clangFrontend
    clangTooling
)
