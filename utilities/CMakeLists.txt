set(parser parser_common parse_function)

cxx_binary(
  TARGET verify-diagnostics
  FILENAME verify_diagnostics.cpp
  LINK_TARGETS clangBasic info ${parser} diagnostic_ids
)
