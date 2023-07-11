include(TableGen)

macro(schreiber_diag_gen component)
  clang_tablegen(
    Diagnostic${component}Kinds.inc
    -gen-clang-diags-defs
    -clang-component=${component}
    SOURCE Diagnostic.td
    TARGET SchreiberDiagnostic${component}
  )
endmacro(schreiber_diag_gen)

schreiber_diag_gen(Lex)
