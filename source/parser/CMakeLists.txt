cxx_library(
  TARGET parser_common
  FILENAME parser_common.cpp
  LINK_TARGETS absl::strings clangBasic
)
add_dependencies(parser_common SchreiberCommentCommandInfo)

cxx_library(
  TARGET parse_function
  FILENAME parse_function.cpp
  LINK_AND_EXPORT_TARGETS
    absl::strings
    clangBasic
)
add_dependencies(parse_function SchreiberCommentCommandInfo)
