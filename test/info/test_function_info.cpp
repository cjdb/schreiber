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

			constexpr auto description = "Returns 0."sv;
			auto parameters = std::vector<info::parameter_info>{};
			auto const returns = info::return_info{"0"};

			SECTION("barebones documentation")
			{
				auto info = info::function_info(
				  decl,
				  std::string(description),
				  parameters,
				  info::return_info{""},
				  {},
				  {},
				  {},
				  {},
				  {},
				  {});
				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				CHECK(info.parameters().empty());
				CHECK(info.returns().data.empty());
				CHECK(info.template_parameters().empty());
				CHECK(info.exception_specifier().data.empty());
				CHECK(info.preconditions().empty());
				CHECK(info.postconditions().empty());
				CHECK(info.throws().empty());
				CHECK(info.exits_via().empty());
				CHECK(info.headers().empty());
				CHECK(info.modules().empty());
			}

			SECTION("uses return value, but not description")
			{
				auto const headers = std::vector{info::header_info{"header.hpp"}};
				auto const modules = std::vector{info::module_info{"module.m"}};
				auto const throws = std::vector{info::throws_info{"yes"}};

				auto const info =
				  info::function_info(decl, "", parameters, returns, {}, {}, throws, {}, headers, modules);
				CHECK(info.decl() == decl);
				CHECK(info.description().empty());
				CHECK(info.parameters().empty());
				CHECK(info.returns() == returns);
				CHECK(info.template_parameters().empty());
				CHECK(info.exception_specifier().data.empty());
				CHECK(info.preconditions().empty());
				CHECK(info.postconditions().empty());
				CHECK(std::ranges::equal(info.throws(), throws));
				CHECK(info.exits_via().empty());
				CHECK(std::ranges::equal(info.headers(), headers));
				CHECK(std::ranges::equal(info.modules(), modules));
			}

			SECTION("uses description and return value")
			{
				auto const headers = std::vector<info::header_info>{
				  {"hello.hpp"},
				  {"world.hpp"},
				};
				auto const modules = std::vector<info::module_info>{
				  {"goodbye"},
				};
				auto const throws = std::vector<info::throws_info>{
				  {"but"},
				  {"not for"},
				};
				auto const exits_via = std::vector<info::exits_via_info>{
				  {"very"},
				  {"long!"},
				};
				auto const info = info::function_info(
				  decl,
				  std::string(description),
				  parameters,
				  returns,
				  {},
				  {},
				  throws,
				  exits_via,
				  headers,
				  modules);
				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				CHECK(info.parameters().empty());
				CHECK(info.returns() == returns);
				CHECK(info.template_parameters().empty());
				CHECK(info.exception_specifier().data.empty());
				CHECK(info.preconditions().empty());
				CHECK(info.postconditions().empty());
				CHECK(std::ranges::equal(info.headers(), headers));
				CHECK(std::ranges::equal(info.modules(), modules));
			}
		}

		SECTION("unary function")
		{
			auto ast = tooling::buildASTFromCodeWithArgs("double square(double x);", compiler_args);
			auto& context = ast->getASTContext();

			auto const decl =
			  selectFirst<clang::FunctionDecl>("decl", match(functionDecl().bind("decl"), context));
			REQUIRE(decl != nullptr);
			REQUIRE(decl->param_size() == 1);

			constexpr auto description = "Returns ``x * x``."sv;
			auto const returns = info::return_info{"x * x"};
			auto const parameters = std::vector{
			  info::parameter_info(decl->getParamDecl(0), "The value to square."),
			};
			auto const preconditions = std::vector<info::precondition_info>{
			  {"``std::is_nan(x) == false``"},
			  {"``std::is_inf(x) == false``"},
			};
			auto const postconditions = std::vector<info::postcondition_info>{
			  {"square(x) >= 0.0"},
			};

			SECTION("doesn't describe x")
			{
				auto const info = info::function_info(
				  decl,
				  std::string(description),
				  {},
				  returns,
				  preconditions,
				  postconditions,
				  {},
				  {},
				  {},
				  {});

				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				CHECK(info.template_parameters().empty());
				CHECK(info.parameters().empty());
				CHECK(info.returns() == returns);
				CHECK(info.exception_specifier().data.empty());
				CHECK(std::ranges::equal(info.preconditions(), preconditions));
				CHECK(std::ranges::equal(info.postconditions(), postconditions));
				CHECK(info.throws().empty());
				CHECK(info.exits_via().empty());
				CHECK(info.headers().empty());
				CHECK(info.modules().empty());
			}

			SECTION("describes x")
			{
				auto const info = info::function_info(
				  decl,
				  std::string(description),
				  parameters,
				  returns,
				  preconditions,
				  postconditions,
				  {},
				  {},
				  {},
				  {});

				CHECK(info.decl() == decl);
				CHECK(info.description() == description);
				CHECK(info.template_parameters().empty());
				REQUIRE(info.parameters().size() == 1);
				CHECK(info.parameters()[0].decl() == parameters[0].decl());
				CHECK(info.parameters()[0].description() == parameters[0].description());
				CHECK(info.returns() == returns);
				CHECK(info.exception_specifier().data.empty());
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
		auto& context = ast->getASTContext();

		auto const record =
		  selectFirst<clang::CXXRecordDecl>("record", match(cxxRecordDecl().bind("record"), context));
		REQUIRE(record != nullptr);
		REQUIRE(record->method_begin() != record->method_end());

		auto const decl = *record->method_begin();
		constexpr auto description = "Returns a value."sv;
		auto parameters = std::vector<info::parameter_info>{};
		auto const returns = info::return_info{"a value"};
		auto const preconditions = std::vector<info::precondition_info>{
		  {"``last`` is reachable from ``first``."},
		};
		auto const postconditions = std::vector<info::postcondition_info>{
		  {"``insert(first, last)`` returns an iterator in the closed interval $[first, last]$."},
		};

		auto const info = info::function_info(
		  decl,
		  std::string(description),
		  parameters,
		  returns,
		  preconditions,
		  postconditions,
		  {},
		  {},
		  {},
		  {});
		CHECK(info.description() == description);
		CHECK(info.template_parameters().empty());
		CHECK(info.parameters().empty());
		CHECK(info.returns() == returns);
		CHECK(info.exception_specifier().data.empty());
		CHECK(std::ranges::equal(info.preconditions(), preconditions));
		CHECK(std::ranges::equal(info.postconditions(), postconditions));
		CHECK(info.throws().empty());
		CHECK(info.exits_via().empty());
		CHECK(info.headers().empty());
		CHECK(info.modules().empty());
	}

	TEST_CASE("function templates")
	{
		constexpr auto code = R"(
template<class T>
concept regular = true;

template<class T>
inline constexpr bool is_nothrow_copyable = true;

template<auto V, class T, regular U, template<class> class W, class... Args>
auto f(Args&&... args) noexcept((is_nothrow_copyable<Args> and ...));
)";
		auto ast = tooling::buildASTFromCodeWithArgs(code, compiler_args);
		auto& context = ast->getASTContext();
		auto const decl = selectFirst<clang::FunctionTemplateDecl>(
		  "decl",
		  match(functionTemplateDecl().bind("decl"), context));
		REQUIRE(decl != nullptr);
		REQUIRE(decl->getAsFunction()->param_size() == 1);
		auto const template_param_list = decl->getTemplateParameters();
		REQUIRE(template_param_list->size() == 5);

		auto const description = "Foosha Village"sv;
		auto template_parameters = std::vector{
		  make_template_param<clang::NonTypeTemplateParmDecl>(template_param_list, 0, "Goat Island"),
		  make_template_param<clang::TemplateTypeParmDecl>(template_param_list, 1, "Shells Town"),
		  make_template_param<clang::TemplateTypeParmDecl>(template_param_list, 2, "Shimotsuki Village"),
		  make_template_param<clang::TemplateTemplateParmDecl>(template_param_list, 3, "Orange Town"),
		  make_template_param<clang::TemplateTypeParmDecl>(template_param_list, 4, "Syrup Village"),
		};
		auto const parameters = std::vector{
		  info::parameter_info(decl->getAsFunction()->getParamDecl(0), "Baratie"),
		};
		auto const exception_specifier = info::noexcept_if_info{"Cocoyasi Village"};
		auto const returns = info::return_info{"Arlong Park"};
		auto const preconditions = std::vector<info::precondition_info>{{"Loguetown"}};
		auto const postconditions = std::vector<info::postcondition_info>{{"Reverse Mountain"}};
		auto const throws = std::vector<info::throws_info>{{"Twin Cape"}};
		auto const exits_via = std::vector<info::exits_via_info>{{"Whisky Peak"}};
		auto const headers = std::vector<info::header_info>{{"Little Garden"}};
		auto const modules = std::vector<info::module_info>{{"Drum"}};

		SECTION("no template parameters described")
		{
			auto info = info::function_info(
			  decl,
			  std::string(description),
			  {},
			  parameters,
			  exception_specifier,
			  returns,
			  preconditions,
			  postconditions,
			  throws,
			  exits_via,
			  headers,
			  modules);

			CHECK(info.decl() == decl);
			CHECK(info.description() == description);
			CHECK(info.template_parameters().empty());
			CHECK(info.returns() == returns);
			CHECK(std::ranges::equal(info.preconditions(), preconditions));
			CHECK(std::ranges::equal(info.postconditions(), postconditions));
			CHECK(std::ranges::equal(info.throws(), throws));
			CHECK(std::ranges::equal(info.exits_via(), exits_via));
			CHECK(std::ranges::equal(info.headers(), headers));
			CHECK(std::ranges::equal(info.modules(), modules));
		}

		SECTION("some template parameters described")
		{
			template_parameters.erase(template_parameters.begin() + 2);
			auto info = info::function_info(
			  decl,
			  std::string(description),
			  template_parameters,
			  parameters,
			  exception_specifier,
			  returns,
			  preconditions,
			  postconditions,
			  throws,
			  exits_via,
			  headers,
			  modules);

			CHECK(info.decl() == decl);
			CHECK(info.description() == description);
			CHECK(std::ranges::equal(info.template_parameters(), template_parameters));
			CHECK(info.returns() == returns);
			CHECK(std::ranges::equal(info.preconditions(), preconditions));
			CHECK(std::ranges::equal(info.postconditions(), postconditions));
			CHECK(std::ranges::equal(info.throws(), throws));
			CHECK(std::ranges::equal(info.exits_via(), exits_via));
			CHECK(std::ranges::equal(info.headers(), headers));
			CHECK(std::ranges::equal(info.modules(), modules));
		}

		SECTION("all template parameters described")
		{
			auto info = info::function_info(
			  decl,
			  std::string(description),
			  template_parameters,
			  parameters,
			  exception_specifier,
			  returns,
			  preconditions,
			  postconditions,
			  throws,
			  exits_via,
			  headers,
			  modules);

			CHECK(info.decl() == decl);
			CHECK(info.description() == description);
			CHECK(std::ranges::equal(info.template_parameters(), template_parameters));
			CHECK(info.returns() == returns);
			CHECK(std::ranges::equal(info.preconditions(), preconditions));
			CHECK(std::ranges::equal(info.postconditions(), postconditions));
			CHECK(std::ranges::equal(info.throws(), throws));
			CHECK(std::ranges::equal(info.exits_via(), exits_via));
			CHECK(std::ranges::equal(info.headers(), headers));
			CHECK(std::ranges::equal(info.modules(), modules));
		}
	}
} // namespace
