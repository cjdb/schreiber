// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <catch2/catch_test_macros.hpp>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Tooling/Tooling.h>
#include <schreiber/info.hpp>

namespace tooling = clang::tooling;

using clang::ast_matchers::functionTemplateDecl;
using clang::ast_matchers::match;

TEST_CASE("classof works correctly")
{
	SECTION("function-based code")
	{
		constexpr auto code = R"(
template<class T>
void f(T t);
)";
		auto ast = tooling::buildASTFromCodeWithArgs(code, {"-target", "x86_64-unknown-linux-gnu"});
		auto& context = ast->getASTContext();
		auto const decl = selectFirst<clang::FunctionTemplateDecl>(
		  "decl",
		  match(functionTemplateDecl().bind("decl"), context));
		REQUIRE(decl != nullptr);
		REQUIRE(decl->getAsFunction()->param_size() == 1);

		SECTION("function_info")
		{
			auto f = info::function_info(decl, {}, {}, {}, {}, {}, {}, {}, {}, {}, {}, {});
			CHECK(info::function_info::classof(static_cast<info::decl_info const*>(&f)));
			CHECK_FALSE(info::parameter_info::classof(static_cast<info::decl_info const*>(&f)));
			CHECK_FALSE(info::template_parameter_info::classof(static_cast<info::decl_info const*>(&f)));
		}

		SECTION("parameter_info")
		{
			auto const param = decl->getAsFunction()->getParamDecl(0);
			auto p = info::parameter_info(param, {});
			CHECK_FALSE(info::function_info::classof(static_cast<info::decl_info const*>(&p)));
			CHECK(info::parameter_info::classof(static_cast<info::decl_info const*>(&p)));
			CHECK_FALSE(info::template_parameter_info::classof(static_cast<info::decl_info const*>(&p)));
		}

		SECTION("template_parameter_info")
		{
			auto const template_param_list = decl->getTemplateParameters();
			REQUIRE(template_param_list->size() == 1);
			auto t = info::template_parameter_info(
			  llvm::dyn_cast<clang::TemplateTypeParmDecl>(template_param_list->getParam(0)),
			  {});
			CHECK_FALSE(info::function_info::classof(static_cast<info::decl_info const*>(&t)));
			CHECK_FALSE(info::parameter_info::classof(static_cast<info::decl_info const*>(&t)));
			CHECK(info::template_parameter_info::classof(static_cast<info::decl_info const*>(&t)));
		}
	}
}
