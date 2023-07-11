//===- TableGen.cpp - Top-Level TableGen implementation for Schreiber
//---------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the main function for Schreiber's TableGen.
//
//===----------------------------------------------------------------------===//

#include "TableGenBackends.h" // Declares all backends.
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/ManagedStatic.h"
#include "llvm/Support/PrettyStackTrace.h"
#include "llvm/Support/Signals.h"
#include "llvm/TableGen/Main.h"
#include "llvm/TableGen/Record.h"

using namespace llvm;

enum ActionType {
  PrintRecords,
  DumpJSON,
  GenSchreiberCommentCommandInfo,
  GenSchreiberCommentCommandList,
};

namespace {
cl::opt<ActionType> Action(
    cl::desc("Action to perform:"),
    cl::values(clEnumValN(PrintRecords, "print-records",
                          "Print all records to stdout (default)"),
               clEnumValN(DumpJSON, "dump-json",
                          "Dump all records as machine-readable JSON"),
               clEnumValN(GenSchreiberCommentCommandInfo,
                          "gen-schreiber-comment-command-info",
                          "Generate command properties for commands that "
                          "are used in documentation comments"),
               clEnumValN(GenSchreiberCommentCommandList,
                          "gen-schreiber-comment-command-list",
                          "Generate list of commands that are used in "
                          "documentation comments")));

cl::opt<std::string>
    SchreiberComponent("schreiber-component",
                       cl::desc("Only use warnings from specified component"),
                       cl::value_desc("component"), cl::Hidden);

bool SchreiberTableGenMain(raw_ostream &OS, RecordKeeper &Records) {
  switch (Action) {
  case PrintRecords:
    OS << Records; // No argument, dump all contents
    break;
  case DumpJSON:
    EmitJSON(Records, OS);
    break;
  case GenSchreiberCommentCommandInfo:
    tablegen::EmitSchreiberCommentCommandInfo(Records, OS);
    break;
  case GenSchreiberCommentCommandList:
    tablegen::EmitSchreiberCommentCommandList(Records, OS);
    break;
  }

  return false;
}
} // namespace

int main(int argc, char **argv) {
  sys::PrintStackTraceOnErrorSignal(argv[0]);
  PrettyStackTraceProgram X(argc, argv);
  cl::ParseCommandLineOptions(argc, argv);

  llvm_shutdown_obj Y;

  return TableGenMain(argv[0], &SchreiberTableGenMain);
}

#ifdef __has_feature
#if __has_feature(address_sanitizer)
#include <sanitizer/lsan_interface.h>

// Disable LeakSanitizer for this binary as it has too many leaks that are not
// very interesting to fix. See compiler-rt/include/sanitizer/lsan_interface.h .
int __lsan_is_turned_off() { return 1; }
#endif // __has_feature(address_sanitizer)
#endif // defined(__has_feature)
