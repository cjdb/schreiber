// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <catch2/catch_test_macros.hpp>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/raw_ostream.h>
#include <schreiber/diagnostic_ids.hpp>
#include <string>

namespace {
	namespace ast_matchers = clang::ast_matchers;
	namespace tooling = clang::tooling;

	using ast_matchers::functionDecl;
	using ast_matchers::match;
	using ast_matchers::selectFirst;

	TEST_CASE("errors can be emitted")
	{
		auto text = std::string();
		auto stream = llvm::raw_string_ostream(text);

		auto ast = tooling::buildASTFromCode("int square(int x);");
		auto& context = ast->getASTContext();
		auto& engine = context.getDiagnostics();
		engine.setClient(new clang::TextDiagnosticPrinter(stream, &engine.getDiagnosticOptions(), false));
		diag::add_diagnostics(engine);

		auto const* decl =
		  selectFirst<clang::FunctionDecl>("decl", match(functionDecl().bind("decl"), context));

		REQUIRE(decl != nullptr);
		REQUIRE(engine.getNumErrors() == 0);
		REQUIRE(engine.getNumWarnings() == 0);

		SECTION("reports an error and a note")
		{
			engine.Report(diag::err_unknown_parameter) << /*template=*/false << "y" << decl;
			engine.Report(diag::note_unknown_parameter) << "'\\param'";
			CHECK(engine.getNumErrors() == 1);
			CHECK(engine.getNumWarnings() == 0);
			CHECK(
			  text
			  == "error: documented parameter 'y' does not map to a parameter in this declaration of "
			     "'square'\n"
			     "note: the word immediately after '\\param' must name one of the parameters in "
			     "the function declaration\n");
		}

		SECTION("reports a warning")
		{
			engine.Report(diag::warn_undocumented_decl) << decl->getCanonicalDecl();
			CHECK(engine.getNumErrors() == 0);
			CHECK(engine.getNumWarnings() == 1);

			engine.Report(diag::note_undocumented_decl) << decl;
			CHECK(text
			      == "warning: function 'square' is not documented\n"
			         "note: use '\\undocumented' to indicate that 'square' should not be documented\n");

			CHECK(engine.getNumErrors() == 0);
			CHECK(engine.getNumWarnings() == 1);
		}
	}
} // namespace
