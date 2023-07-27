cxx_binary(
  TARGET verify-diagnostics
  FILENAME verify_diagnostics.cpp
  LINK_TARGETS clangBasic info parser_common diagnostic_ids
)
