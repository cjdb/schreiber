let Component = "Lex", CategoryName = "Lexical issue" in {
  // Errors (and correstponding notes)
  def err_unknown_parameter : Error<
    "%select{|t}0param '%1' does not map to a %select{|template }0parameter in this declaration of %2"
  >;
  def note_unknown_parameter : Note<
    "documentation for%select{| template}0 parameters must have the same name as one declared in "
    "the documented declaration"
  >;

  // Warnings (and correstponding notes)
  def warn_unknown_directive : Warning<
    "unknown directive '\\%0'"
  >;

  def warn_undocumented_decl : Warning<
    "function %0 is not documented"
  >;
  def note_undocumented_decl : Note<
    "use '\\undocumented' to indicate that %0 should not be documented"
  >;
}
