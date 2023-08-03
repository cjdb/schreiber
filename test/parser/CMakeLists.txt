set(parser parser_common parse_function)

cxx_test(
  TARGET test_parse_function
  FILENAME test_parse_function.cpp
  LINK_TARGETS info ${parser} diagnostic_ids
)
