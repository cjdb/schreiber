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

cxx_library(
  TARGET diagnostic_ids
  FILENAME diagnostic_ids.cpp
  LINK_AND_EXPORT_TARGETS
    clangBasic
)
add_dependencies(diagnostic_ids schreiber-tablegen-targets)

add_subdirectory(parser)
