// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <catch2/catch_test_macros.hpp>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Casting.h>
#include <schreiber/info.hpp>

namespace {
	namespace tooling = clang::tooling;

	using clang::ast_matchers::functionDecl;
	using clang::ast_matchers::match;

	TEST_CASE("classof works correctly")
	{
		SECTION("function-based code")
		{
			constexpr auto code = R"(void f(T t);)";
			auto ast = tooling::buildASTFromCodeWithArgs(code, {"-target", "x86_64-unknown-linux-gnu"});
			auto& context = ast->getASTContext();
			auto const decl =
			  selectFirst<clang::FunctionDecl>("decl", match(functionDecl().bind("decl"), context));
			REQUIRE(decl != nullptr);
			REQUIRE(decl->getAsFunction()->param_size() == 1);

			SECTION("header_info")
			{
				auto const header = info::decl_info::header_info("", {});
				CHECK(info::decl_info::header_info::classof(&header));
				CHECK_FALSE(info::decl_info::module_info::classof(&header));
				CHECK_FALSE(info::function_info::classof(&header));
				CHECK_FALSE(info::function_info::return_info::classof(&header));
				CHECK_FALSE(info::function_info::precondition_info::classof(&header));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&header));
				CHECK_FALSE(info::function_info::throws_info::classof(&header));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&header));
				CHECK_FALSE(info::parameter_info::classof(&header));
				CHECK_FALSE(info::template_parameter_info::classof(&header));
			}

			SECTION("module_info")
			{
				auto const module = info::decl_info::module_info("", {});
				CHECK_FALSE(info::decl_info::header_info::classof(&module));
				CHECK(info::decl_info::module_info::classof(&module));
				CHECK_FALSE(info::function_info::classof(&module));
				CHECK_FALSE(info::function_info::return_info::classof(&module));
				CHECK_FALSE(info::function_info::precondition_info::classof(&module));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&module));
				CHECK_FALSE(info::function_info::throws_info::classof(&module));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&module));
				CHECK_FALSE(info::parameter_info::classof(&module));
				CHECK_FALSE(info::template_parameter_info::classof(&module));
			}

			SECTION("function_info")
			{
				auto f = info::function_info(decl, "", decl->getLocation());
				CHECK_FALSE(info::decl_info::header_info::classof(&f));
				CHECK_FALSE(info::decl_info::module_info::classof(&f));
				CHECK(info::function_info::classof(&f));
				CHECK_FALSE(info::function_info::return_info::classof(&f));
				CHECK_FALSE(info::function_info::precondition_info::classof(&f));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&f));
				CHECK_FALSE(info::function_info::throws_info::classof(&f));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&f));
				CHECK_FALSE(info::parameter_info::classof(&f));
				CHECK_FALSE(info::template_parameter_info::classof(&f));
			}

			SECTION("returns_info")
			{
				auto const returns = info::function_info::return_info("", {});
				CHECK_FALSE(info::decl_info::header_info::classof(&returns));
				CHECK_FALSE(info::decl_info::module_info::classof(&returns));
				CHECK_FALSE(info::function_info::classof(&returns));
				CHECK(info::function_info::return_info::classof(&returns));
				CHECK_FALSE(info::function_info::precondition_info::classof(&returns));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&returns));
				CHECK_FALSE(info::function_info::throws_info::classof(&returns));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&returns));
				CHECK_FALSE(info::parameter_info::classof(&returns));
				CHECK_FALSE(info::template_parameter_info::classof(&returns));
			}

			SECTION("precondition_info")
			{
				auto const precondition = info::function_info::precondition_info("", {});
				CHECK_FALSE(info::decl_info::header_info::classof(&precondition));
				CHECK_FALSE(info::decl_info::module_info::classof(&precondition));
				CHECK_FALSE(info::function_info::classof(&precondition));
				CHECK_FALSE(info::function_info::return_info::classof(&precondition));
				CHECK(info::function_info::precondition_info::classof(&precondition));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&precondition));
				CHECK_FALSE(info::function_info::throws_info::classof(&precondition));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&precondition));
				CHECK_FALSE(info::parameter_info::classof(&precondition));
				CHECK_FALSE(info::template_parameter_info::classof(&precondition));
			}

			SECTION("postcondition_info")
			{
				auto const postcondition = info::function_info::postcondition_info("", {});
				CHECK_FALSE(info::decl_info::header_info::classof(&postcondition));
				CHECK_FALSE(info::decl_info::module_info::classof(&postcondition));
				CHECK_FALSE(info::function_info::classof(&postcondition));
				CHECK_FALSE(info::function_info::return_info::classof(&postcondition));
				CHECK_FALSE(info::function_info::precondition_info::classof(&postcondition));
				CHECK(info::function_info::postcondition_info::classof(&postcondition));
				CHECK_FALSE(info::function_info::throws_info::classof(&postcondition));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&postcondition));
				CHECK_FALSE(info::parameter_info::classof(&postcondition));
				CHECK_FALSE(info::template_parameter_info::classof(&postcondition));
			}

			SECTION("throws_info")
			{
				auto const throws = info::function_info::throws_info("", {});
				CHECK_FALSE(info::decl_info::header_info::classof(&throws));
				CHECK_FALSE(info::decl_info::module_info::classof(&throws));
				CHECK_FALSE(info::function_info::classof(&throws));
				CHECK_FALSE(info::function_info::return_info::classof(&throws));
				CHECK_FALSE(info::function_info::precondition_info::classof(&throws));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&throws));
				CHECK(info::function_info::throws_info::classof(&throws));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&throws));
				CHECK_FALSE(info::parameter_info::classof(&throws));
				CHECK_FALSE(info::template_parameter_info::classof(&throws));
			}

			SECTION("exits_via_info")
			{
				auto const exits_via = info::function_info::exits_via_info("", {});
				CHECK_FALSE(info::decl_info::header_info::classof(&exits_via));
				CHECK_FALSE(info::decl_info::module_info::classof(&exits_via));
				CHECK_FALSE(info::function_info::classof(&exits_via));
				CHECK_FALSE(info::function_info::return_info::classof(&exits_via));
				CHECK_FALSE(info::function_info::precondition_info::classof(&exits_via));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&exits_via));
				CHECK_FALSE(info::function_info::throws_info::classof(&exits_via));
				CHECK(info::function_info::exits_via_info::classof(&exits_via));
				CHECK_FALSE(info::parameter_info::classof(&exits_via));
				CHECK_FALSE(info::template_parameter_info::classof(&exits_via));
			}

			SECTION("parameter_info")
			{
				auto const param = decl->getAsFunction()->getParamDecl(0);
				auto p = info::parameter_info(param->getLocation(), param, {});
				CHECK_FALSE(info::decl_info::header_info::classof(&p));
				CHECK_FALSE(info::decl_info::module_info::classof(&p));
				CHECK_FALSE(info::function_info::classof(&p));
				CHECK_FALSE(info::function_info::return_info::classof(&p));
				CHECK_FALSE(info::function_info::precondition_info::classof(&p));
				CHECK_FALSE(info::function_info::postcondition_info::classof(&p));
				CHECK_FALSE(info::function_info::throws_info::classof(&p));
				CHECK_FALSE(info::function_info::exits_via_info::classof(&p));
				CHECK(info::parameter_info::classof(&p));
				CHECK_FALSE(info::template_parameter_info::classof(&p));
			}

			// TODO: test template_parameter_info
		}
	}
} // namespace
