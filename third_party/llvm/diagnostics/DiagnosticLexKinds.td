let Component = "Lex", CategoryName = "Lexical issue" in {
  // Errors (and correstponding notes)
  def err_unknown_directive : Error<"unknown directive `%0`">;

  // Warnings (and correstponding notes)
  def warn_undocumented_decl : Warning<
    "function %0 is not documented"
  >;
  def note_undocumented_decl : Note<
    "use `\\undocumented` to indicate that %0 should not be documented"
  >;
}
