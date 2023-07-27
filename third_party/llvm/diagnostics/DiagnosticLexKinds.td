let Component = "Lex", CategoryName = "Lexical issue" in {
  // Substitutions
  def entity : TextSubstitution<
    "%select{"
      "class template"
      "|class"
      "|concept"
      "|data member"
      "|deduction guide"
      "|enum constant"
      "|enum"
      "|function template"
      "|function"
      "|struct"
      "|type alias template"
      "|type alias"
      "|typedef"
      "|union"
      "|variable template"
      "|variable"
    "}0"
  >;
  def smf : TextSubstitution<
    "%select{|default |move |copy }0"
    "%select{constructor|destructor|assignment operator}1 for"
  >;
  def member : TextSubstitution<
    "%select{|member |nested }0%sub{entity}1|"
  >;
  def specialisation : TextSubstitution<
    "%select{partial|explicit}0 specialization of %sub{entity}1"
  >;


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
    "%select{%sub{entity}1|%sub{smf}1,3|%sub{member}1,3|%sub{specialisation}1,3}0 %2 is not documented"
  >;
  def note_undocumented_decl : Note<
    "use '\\undocumented' to indicate that %0 is intentionally undocumented"
  >;
}
