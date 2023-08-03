cxx_test(
  TARGET test_classof
  FILENAME test_classof.cpp
  LINK_TARGETS info parser_common parse_function
)

cxx_test(
  TARGET test_function_info
  FILENAME test_function_info.cpp
  LINK_TARGETS info parser_common parse_function
)
