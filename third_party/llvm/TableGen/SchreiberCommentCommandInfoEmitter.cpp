//===--- SchreiberCommentCommandInfoEmitter.cpp - Generate command lists
//-----====//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This tablegen backend emits command lists and efficient matchers for command
// names that are used in documentation comments.
//
//===----------------------------------------------------------------------===//

#include "TableGenBackends.h"

#include "llvm/TableGen/Record.h"
#include "llvm/TableGen/StringMatcher.h"
#include "llvm/TableGen/TableGenBackend.h"
#include <vector>

using namespace llvm;

namespace tablegen {
void EmitSchreiberCommentCommandInfo(RecordKeeper &Records, raw_ostream &OS) {
  emitSourceFileHeader("A list of commands useable in documentation "
                       "comments",
                       OS);

  OS << "#include <array>\n"
        "#include <string_view>\n"
        "\n"
        "namespace {\n"
        "\tauto const commands = std::array{\n";
  std::vector<Record *> Tags = Records.getAllDerivedDefinitions("Command");
  for (size_t i = 0, e = Tags.size(); i != e; ++i) {
    Record &Tag = *Tags[i];
    OS << "\t\tcommand_info{"
       // Global directives
       << "\"" << Tag.getValueAsString("Name") << "\", "
       << "},\n";
  }
  OS << "\t};\n"
        "} // unnamed namespace\n\n";

  std::vector<StringMatcher::StringPair> Matches;
  for (size_t i = 0, e = Tags.size(); i != e; ++i) {
    Record &Tag = *Tags[i];
    std::string Name = std::string(Tag.getValueAsString("Name"));
    std::string Return;
    raw_string_ostream(Return) << "return &commands[" << i << "];";
    Matches.emplace_back(std::move(Name), std::move(Return));
  }

  OS << "auto lex_directive(std::string_view name) -> command_info const* {\n";
  StringMatcher("name", Matches, OS).Emit();
  OS << "  return nullptr;\n"
     << "}\n\n";
}

namespace {
static std::string MangleName(StringRef Str) {
  std::string Mangled;
  for (char const i : Str) {
    switch (i) {
    default:
      Mangled += i;
      break;
    case '(':
      Mangled += "lparen";
      break;
    case ')':
      Mangled += "rparen";
      break;
    case '[':
      Mangled += "lsquare";
      break;
    case ']':
      Mangled += "rsquare";
      break;
    case '{':
      Mangled += "lbrace";
      break;
    case '}':
      Mangled += "rbrace";
      break;
    case '$':
      Mangled += "dollar";
      break;
    case '/':
      Mangled += "slash";
      break;
    }
  }
  return Mangled;
}
} // namespace

void EmitSchreiberCommentCommandList(RecordKeeper &Records, raw_ostream &OS) {
  emitSourceFileHeader("A list of commands useable in documentation "
                       "comments",
                       OS);

  OS << "#ifndef COMMENT_COMMAND\n"
     << "#  define COMMENT_COMMAND(NAME)\n"
     << "#endif\n";

  std::vector<Record *> Tags = Records.getAllDerivedDefinitions("Command");
  for (size_t i = 0, e = Tags.size(); i != e; ++i) {
    Record &Tag = *Tags[i];
    std::string MangledName = MangleName(Tag.getValueAsString("Name"));

    OS << "COMMENT_COMMAND(" << MangledName << ")\n";
  }
}
} // namespace tablegen
