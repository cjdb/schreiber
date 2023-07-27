cxx_library(
  TARGET parser_common
  FILENAME parser_common.cpp
  LINK_TARGETS clangBasic
)
add_dependencies(parser_common SchreiberCommentCommandInfo)
