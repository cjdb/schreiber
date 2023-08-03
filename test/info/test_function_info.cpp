// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <algorithm>
#include <catch2/catch_test_macros.hpp>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Tooling/Tooling.h>
#include <llvm/Support/Casting.h>
#include <memory>
#include <schreiber/info.hpp>
#include <schreiber/parser.hpp>
#include <string>
#include <string_view>
#include <vector>

namespace {
	namespace ast_matchers = clang::ast_matchers;
	namespace tooling = clang::tooling;

	using ast_matchers::cxxRecordDecl;
	using ast_matchers::functionDecl;
	using ast_matchers::functionTemplateDecl;
	using ast_matchers::match;
	using ast_matchers::selectFirst;
	using namespace std::string_view_literals;

	using header_info = info::decl_info::header_info;
	using module_info = info::decl_info::module_info;
	using return_info = info::function_info::return_info;
	using throws_info = info::function_info::throws_info;
	using precondition_info = info::function_info::precondition_info;
	using postcondition_info = info::function_info::postcondition_info;
	using exits_via_info = info::function_info::exits_via_info;

	auto const compiler_args =
	  std::vector<std::string>{"-target", "x86_64-unknown-linux-gnu", "-std=c++20"};

	TEST_CASE("function")
	{
		SECTION("nullary function")
		{
			auto ast = tooling::buildASTFromCodeWithArgs("int zero();", compiler_args);
			auto& context = ast->getASTContext();

			auto const decl =
			  selectFirst<clang::FunctionDecl>("decl", match(functionDecl().bind("decl"), context));
			REQUIRE(decl != nullptr);
			parser::parser p(decl->getASTContext());
			auto store = [&p](info::function_info& info, auto d) { info.store(p, {}, &d); };

			constexpr auto description = "Returns 0."sv;
			auto parameters = std::vector<info::parameter_info>{};
			auto returns = info::function_info::return_info("0", {});

			SECTION("barebones documentation")
			{
				auto const info = info::function_info(decl, std::string(description), {});
				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				CHECK(info.parameters().empty());
				CHECK(info.returns() == std::nullopt);
				CHECK(info.preconditions().empty());
				CHECK(info.postconditions().empty());
				CHECK(info.throws().empty());
				CHECK(info.exits_via().empty());
				CHECK(info.headers().empty());
				CHECK(info.modules().empty());
			}

			SECTION("uses return value, but not description")
			{
				auto info = info::function_info(decl, "", {});
				auto const headers = header_info{"header.hpp", {}};
				auto const modules = module_info{"module.m", {}};
				auto const throws = throws_info{"yes", {}};

				store(info, returns);
				store(info, headers);
				store(info, modules);
				store(info, throws);

				CHECK(info.decl() == decl);
				CHECK(info.description().empty());
				CHECK(info.parameters().empty());
				CHECK(info.returns() == returns);
				CHECK(info.preconditions().empty());
				CHECK(info.postconditions().empty());
				REQUIRE(info.throws().size() == 1);
				CHECK(info.throws()[0] == throws);
				CHECK(info.exits_via().empty());
				REQUIRE(info.headers().size() == 1);
				CHECK(info.headers()[0] == headers);
				REQUIRE(info.modules().size() == 1);
				CHECK(info.modules()[0] == modules);
			}

			SECTION("uses description and return value")
			{
				auto info = info::function_info(decl, std::string(description), {});
				auto const headers = std::vector<header_info>{
				  {"hello.hpp", {}},
				  {"world.hpp", {}},
				};
				auto const modules = std::vector<module_info>{
				  {"goodbye", {}},
				};
				auto const throws = std::vector<throws_info>{
				  {    "but", {}},
				  {"not for", {}},
				};
				auto const exits_via = std::vector<exits_via_info>{
				  { "very", {}},
				  {"long!", {}},
				};
				store(info, returns);
				store(info, throws[0]);
				store(info, throws[1]);
				store(info, exits_via[0]);
				store(info, exits_via[1]);
				store(info, headers[0]);
				store(info, headers[1]);
				store(info, modules[0]);

				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				CHECK(info.parameters().empty());
				CHECK(info.returns() == returns);
				CHECK(info.preconditions().empty());
				CHECK(info.postconditions().empty());
				CHECK(std::ranges::equal(info.headers(), headers));
				CHECK(std::ranges::equal(info.modules(), modules));
			}
		}

		SECTION("unary function")
		{
			auto ast = tooling::buildASTFromCodeWithArgs("double square(double x);", compiler_args);
			REQUIRE(ast != nullptr);
			auto& context = ast->getASTContext();
			parser::parser p(context);
			auto store = [&p](info::function_info& info, auto d) { info.store(p, {}, &d); };

			auto const decl =
			  selectFirst<clang::FunctionDecl>("decl", match(functionDecl().bind("decl"), context));
			REQUIRE(decl != nullptr);
			REQUIRE(decl->param_size() == 1);

			constexpr auto description = "Returns ``x * x``."sv;
			auto const returns = info::function_info::return_info{"x * x", {}};
			auto const parameters = std::vector{
			  info::parameter_info(decl->getLocation(), decl->getParamDecl(0), "The value to square."),
			};
			auto const preconditions = std::vector<info::function_info::precondition_info>{
			  {"``std::is_nan(x) == false``", {}},
			  {"``std::is_inf(x) == false``", {}},
			};
			auto const postconditions = std::vector<info::function_info::postcondition_info>{
			  {"square(x) >= 0.0", {}},
			};

			SECTION("doesn't describe x")
			{
				auto info = info::function_info(decl, std::string(description), {});
				store(info, returns);
				store(info, preconditions[0]);
				store(info, preconditions[1]);
				store(info, postconditions[0]);

				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				CHECK(info.parameters().empty());
				CHECK(info.returns() == returns);
				CHECK(std::ranges::equal(info.preconditions(), preconditions));
				CHECK(std::ranges::equal(info.postconditions(), postconditions));
				CHECK(info.throws().empty());
				CHECK(info.exits_via().empty());
				CHECK(info.headers().empty());
				CHECK(info.modules().empty());
			}

			SECTION("describes x")
			{
				auto info = info::function_info(decl, std::string(description), {});
				store(info, parameters[0]);
				store(info, returns);
				store(info, preconditions[0]);
				store(info, preconditions[1]);
				store(info, postconditions[0]);

				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				REQUIRE(info.parameters().size() == 1);
				CHECK(std::ranges::equal(info.parameters(), parameters));
				CHECK(info.returns() == returns);
				CHECK(std::ranges::equal(info.preconditions(), preconditions));
				CHECK(std::ranges::equal(info.postconditions(), postconditions));
				CHECK(info.throws().empty());
				CHECK(info.exits_via().empty());
				CHECK(info.headers().empty());
				CHECK(info.modules().empty());
			}
		}
	}

	template<class T>
	[[nodiscard]] auto make_template_param(
	  clang::TemplateParameterList const* const params,
	  unsigned int const i,
	  std::string_view const description) -> info::template_parameter_info
	{
		REQUIRE(i < params->size());
		return info::template_parameter_info{
		  params->getParam(i)->getSourceRange(),
		  llvm::dyn_cast<T>(params->getParam(i)),
		  std::string(description),
		};
	}

	TEST_CASE("member funciton")
	{
		constexpr auto code = R"(
struct range {
	int const* insert(int const* first, int const* last) const noexcept;
};
)";
		auto ast = tooling::buildASTFromCodeWithArgs(code, compiler_args);
		REQUIRE(ast != nullptr);
		auto& context = ast->getASTContext();
		parser::parser p(context);
		auto store = [&p](info::function_info& info, auto d) { info.store(p, {}, &d); };

		auto const record =
		  selectFirst<clang::CXXRecordDecl>("record", match(cxxRecordDecl().bind("record"), context));
		REQUIRE(record != nullptr);
		REQUIRE(record->method_begin() != record->method_end());

		auto const decl = *record->method_begin();
		constexpr auto description = "Returns a value."sv;
		auto parameters = std::vector<info::parameter_info>{};
		auto const returns = info::function_info::return_info{"a value", {}};
		auto const preconditions = std::vector<info::function_info::precondition_info>{
		  {"``last`` is reachable from ``first``.", {}},
		};
		auto const postconditions = std::vector<info::function_info::postcondition_info>{
		  {"``insert(first, last)`` returns an iterator in the closed interval $[first, last]$.", {}},
		};

		auto info = info::function_info(decl, std::string(description), {});

		store(info, returns);
		store(info, preconditions[0]);
		store(info, postconditions[0]);

		CHECK(info.description() == description);
		CHECK(info.parameters().empty());
		CHECK(info.returns() == returns);
		CHECK(std::ranges::equal(info.preconditions(), preconditions));
		CHECK(std::ranges::equal(info.postconditions(), postconditions));
		CHECK(info.throws().empty());
		CHECK(info.exits_via().empty());
		CHECK(info.headers().empty());
		CHECK(info.modules().empty());
	}
} // namespace
