// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#ifndef SCHREIBER_DIAGNOSTIC_IDS_HPP
#define SCHREIBER_DIAGNOSTIC_IDS_HPP

#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/DiagnosticIDs.h>

namespace diag {
	enum class size {
		lex = 0,
	};

	enum start {
		DIAG_START_LEX = clang::diag::DIAG_UPPER_LIMIT - 1, // NOLINT(readability-identifier-naming)
	};

	// clang-format off
	enum id {
#define DIAG(ENUM, FLAGS, DEFAULT_MAPPING, DESC, GROUP, SFINAE, NOWERROR,      \
             SHOWINSYSHEADER, SHOWINSYSMACRO, DEFERRABLE, CATEGORY)            \
  ENUM,
#define LEXSTART
#include "diagnostics/DiagnosticLexKinds.inc"
		num_schreiber_lex_diagnostics,
	};
#undef DIAG

	// clang-format on

	/// Adds all the Schreiber diagnostics to the diagnostics engine.
	void add_diagnostics(clang::DiagnosticsEngine& engine);
} // namespace diag

#endif // SCHREIBER_DIAGNOSTIC_IDS_HPP
