// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include "schreiber/info.hpp"
#include "schreiber/parser.hpp"
#include <catch2/catch_test_macros.hpp>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Tooling/Tooling.h>
#include <schreiber/diagnostic_ids.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace {
	namespace ast_matchers = clang::ast_matchers;
	namespace tooling = clang::tooling;

	using ast_matchers::functionDecl;
	using ast_matchers::match;
	using ast_matchers::selectFirst;

	using namespace std::string_view_literals;

	struct function_decl {
		explicit function_decl(std::string_view const code)
		: ast(tooling::buildASTFromCode(code))
		{
			diags.getDiagnosticOptions().ShowFixits = true;
			diags.setClient(new clang::TextDiagnosticPrinter(stream, &diags.getDiagnosticOptions(), false));
			diag::add_diagnostics(diags);
			auto client = diags.getClient();
			client->BeginSourceFile(ast->getLangOpts());
		}

		~function_decl()
		{
			context.getDiagnostics().getClient()->EndSourceFile();
		}

		std::unique_ptr<clang::ASTUnit> ast;
		clang::ASTContext& context = ast->getASTContext();
		clang::DiagnosticsEngine& diags = context.getDiagnostics();
		clang::FunctionDecl const* decl =
		  selectFirst<clang::FunctionDecl>("decl", match(functionDecl().bind("decl"), context));
		std::string text;
		llvm::raw_string_ostream stream{text};
	};

	TEST_CASE("a function documented with the lyrics of 'We Are!' by Shoko Fujibayashi")
	{
		auto const function = function_decl(R"(
			/// Come aboard, and bring along
			/// All your hopes and dreams
			/// Together we'll find everything
			/// That we're looking for
			///
			/// ONE PIECE!
			/// \param first Compass left behind
			/// \param last It'll only slow us down
			/// \pre Your heart will be your guide
			/// \pre Raise the sails and take the helm
			/// \pre That legendary place, that the end of the map reveals
			/// \pre Is only legendary
			/// \post 'Till someone proves it real
			/// \post Through all the troubled times
			/// \post Through the heartache, and through the pain
			/// \throws Know that I'll be there to stand by you
			/// \throws Just like I know you'll stand by me!
			/// \returns So come aboard, and bring along
			///          All your hopes and dreams
			///          Together we'll find everything
			///          That we're looking for
			/// \headers There's always room for you, if you wanna be my friend
			/// \modules We.are, we.are, on.the.cruise
			/// \exits-via We are!
			int const* find(int const* first, int const* last);)");
		auto p = parser::parser(function.context);

		auto const info = p.parse(function.decl);
		auto const f = llvm::dyn_cast<info::function_info>(info.get());
		REQUIRE(function.decl != nullptr);
		REQUIRE(function.diags.getNumErrors() == 0);
		REQUIRE(function.diags.getNumWarnings() == 0);

		CHECK(f->decl() == function.decl);
		CHECK(
		  f->description()
		  == "Come aboard, and bring along\n"
		     "All your hopes and dreams\n"
		     "Together we'll find everything\n"
		     "That we're looking for\n"
		     "\n"
		     "ONE PIECE!"sv);
		CHECK(f->template_parameters().empty());
		REQUIRE(f->parameters().size() == 2);
		{
			constexpr auto index = 0;
			auto const& param = f->parameters()[index];
			CHECK(param.decl() == function.decl->parameters()[index]);
			CHECK(param.description() == "Compass left behind");
		}
		{
			constexpr auto index = 1;
			auto const& param = f->parameters()[index];
			CHECK(param.decl() == function.decl->parameters()[index]);
			CHECK(param.description() == "It'll only slow us down");
		}

		REQUIRE(f->preconditions().size() == 4);
		CHECK(f->preconditions()[0].data == "Your heart will be your guide");
		CHECK(f->preconditions()[1].data == "Raise the sails and take the helm");
		CHECK(f->preconditions()[2].data == "That legendary place, that the end of the map reveals");
		CHECK(f->preconditions()[3].data == "Is only legendary");

		REQUIRE(f->postconditions().size() == 3);
		CHECK(f->postconditions()[0].data == "'Till someone proves it real");
		CHECK(f->postconditions()[1].data == "Through all the troubled times");
		CHECK(f->postconditions()[2].data == "Through the heartache, and through the pain");

		REQUIRE(f->throws().size() == 2);
		CHECK(f->throws()[0].data == "Know that I'll be there to stand by you");
		CHECK(f->throws()[1].data == "Just like I know you'll stand by me!");

		CHECK(
		  f->returns().data
		  == "So come aboard, and bring along\n"
		     "         All your hopes and dreams\n"
		     "         Together we'll find everything\n"
		     "         That we're looking for");

		REQUIRE(f->headers().size() == 2);
		CHECK(f->headers()[0].data == "There's always room for you");
		CHECK(f->headers()[1].data == "if you wanna be my friend");

		REQUIRE(f->modules().size() == 3);
		CHECK(f->modules()[0].data == "We.are");
		CHECK(f->modules()[1].data == "we.are");
		CHECK(f->modules()[2].data == "on.the.cruise");

		REQUIRE(f->exits_via().size() == 1);
		CHECK(f->exits_via()[0].data == "We are!");

		CHECK(f->exception_specifier().data.empty());
	}
} // namespace
