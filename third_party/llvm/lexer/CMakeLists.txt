set(CLANG_TABLEGEN_EXE "${SCHREIBER_TABLEGEN}")
clang_tablegen(
  CommentCommandInfo.inc
  -gen-schreiber-comment-command-info
  SOURCE CommentCommands.td
  TARGET SchreiberCommentCommandInfo
  )
add_dependencies(SchreiberCommentCommandInfo schreiber-tblgen)

clang_tablegen(
  CommentCommandList.inc
  -gen-schreiber-comment-command-list
  SOURCE CommentCommands.td
  TARGET SchreiberCommentCommandList
)
add_dependencies(SchreiberCommentCommandList schreiber-tblgen)
