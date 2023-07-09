cxx_test(
  TARGET check_diagnostic_ids
  FILENAME check_diagnostic_ids.cpp
  LINK_TARGETS
    clangAST
    clangTooling
    diagnostic_ids
)
