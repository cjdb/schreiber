set(LLVM_LINK_COMPONENTS Support)

cxx_binary(
  TARGET schreiber-tblgen
  FILENAMES
    ASTTableGen.cpp
    SchreiberCommentCommandInfoEmitter.cpp
    TableGen.cpp
  LINK_TARGETS
    clangSupport
    LLVMSupport
    LLVMTableGen
)
