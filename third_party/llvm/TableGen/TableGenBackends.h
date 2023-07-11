//===- TableGenBackends.h - Declarations for Schreiber TableGen Backends
//------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations for all of the Schreiber TableGen
// backends. A "TableGen backend" is just a function. See
// "$LLVM_ROOT/utils/TableGen/TableGenBackends.h" for more info.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SCHREIBER_UTILS_TABLEGEN_TABLEGENBACKENDS_H
#define LLVM_SCHREIBER_UTILS_TABLEGEN_TABLEGENBACKENDS_H

#include <string>

namespace llvm {
class raw_ostream;
class RecordKeeper;
} // namespace llvm

namespace tablegen {
void EmitSchreiberDeclContext(llvm::RecordKeeper &RK, llvm::raw_ostream &OS);
void EmitSchreiberASTNodes(llvm::RecordKeeper &RK, llvm::raw_ostream &OS,
                           std::string const &N, std::string const &S);

void EmitSchreiberDiagsDefs(llvm::RecordKeeper &Records, llvm::raw_ostream &OS,
                            std::string const &Component);
void EmitSchreiberDiagGroups(llvm::RecordKeeper &Records,
                             llvm::raw_ostream &OS);
void EmitSchreiberDiagsIndexName(llvm::RecordKeeper &Records,
                                 llvm::raw_ostream &OS);

void EmitSchreiberCommentCommandInfo(llvm::RecordKeeper &Records,
                                     llvm::raw_ostream &OS);
void EmitSchreiberCommentCommandList(llvm::RecordKeeper &Records,
                                     llvm::raw_ostream &OS);
} // namespace tablegen

#endif
