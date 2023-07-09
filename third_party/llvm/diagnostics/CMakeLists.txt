include(TableGen)

file(MAKE_DIRECTORY "${DIRECTORY}")
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

get_property(CLANG_TABLEGEN_TARGETS GLOBAL PROPERTY CLANG_TABLEGEN_TARGETS)
add_custom_target(schreiber-tablegen-targets DEPENDS "${CLANG_TABLEGEN_TARGETS}")
set_target_properties(schreiber-tablegen-targets PROPERTIES FOLDER "Misc")
