// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <clang/Basic/Diagnostic.h>
#include <schreiber/diagnostic_ids.hpp>

namespace diag {
	using clang::diag::Severity; // NOLINT(misc-include-cleaner)

	void add_diagnostics(clang::DiagnosticsEngine& engine)
	{
		// clang-format off
#define DIAG(ENUM, FLAGS, SEVERITY, DESC, GROUP, SFINAE, NOWERROR,                              \
             SHOWINSYSHEADER, SHOWINSYSMACRO, DEFERRABLE, CATEGORY)                             \
		engine.getCustomDiagID(static_cast<clang::DiagnosticsEngine::Level>(SEVERITY), DESC);
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wold-style-cast"
#include "diagnostics/DiagnosticLexKinds.inc" // NOLINT(misc-include-cleaner)
#pragma GCC diagnostic pop
#undef DIAG
		// clang-format on
	}
} // namespace diag
