// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <absl/strings/ascii.h>
#include <absl/strings/strip.h>
#include <algorithm>
#include <clang/AST/ASTContext.h>
#include <clang/AST/CommentCommandTraits.h>
#include <clang/AST/Decl.h>
#include <clang/AST/RawCommentList.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/PartialDiagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <functional>
#include <iterator>
#include <llvm/ADT/StringExtras.h>
#include <llvm/ADT/iterator_range.h>
#include <memory>
#include <ranges>
#include <schreiber/diagnostic_ids.hpp>
#include <schreiber/info.hpp>
#include <schreiber/parser.hpp>
#include <string_view>

namespace parser {
	namespace stdr = std::ranges;
	namespace stdv = std::views;
	using namespace std::string_view_literals;

	template<stdr::contiguous_range R>
	[[nodiscard]] static auto to_string_view(R&& r) noexcept -> std::string_view
	{
		return {r.begin(), stdr::next(r.begin(), r.end())};
	}

	auto make_parse_result(
	  parser& p,
	  clang::FunctionDecl const* decl,
	  directive const directive,
	  std::string description) -> std::unique_ptr<info::basic_info>
	{
		switch (directive.token->kind) {
		case command_info::headers:
			return std::make_unique<info::decl_info::header_info>(std::move(description), directive.location);
		case command_info::modules:
			return std::make_unique<info::decl_info::module_info>(std::move(description), directive.location);
		case command_info::param: {
			auto name = to_string_view(
			  absl::StripLeadingAsciiWhitespace(description) | stdv::take_while(std::not_fn(is_space)));
			auto parameters = decl->parameters();
			auto parameter = stdr::find_if(parameters, [name](clang::ParmVarDecl const* const p) {
				return std::string_view{p->getName()} == name;
			});
			if (parameter == decl->param_end()) {
				auto const report_loc =
				  directive.location.getLocWithOffset(static_cast<int>(directive.text.size() + 2));
				p.diagnose(report_loc, diag::err_unknown_parameter) << /*is_template=*/false << name << decl;
				p.diagnose(report_loc, diag::note_unknown_parameter) << command_info::param;
				return nullptr;
			}

			auto desc = description.substr(name.size());
			absl::StripLeadingAsciiWhitespace(&desc);
			return std::make_unique<info::parameter_info>(directive.location, *parameter, std::move(desc));
		}
		case command_info::returns:
			return std::make_unique<info::function_info::return_info>(
			  std::move(description),
			  directive.location);
		case command_info::pre:
			return std::make_unique<info::function_info::precondition_info>(
			  std::move(description),
			  directive.location);
		case command_info::post:
			return std::make_unique<info::function_info::postcondition_info>(
			  std::move(description),
			  directive.location);
		case command_info::throws:
			return std::make_unique<info::function_info::throws_info>(
			  std::move(description),
			  directive.location);
		case command_info::exits_via:
			return std::make_unique<info::function_info::exits_via_info>(
			  std::move(description),
			  directive.location);
		default:
			std::unreachable();
		}
	}

	auto parser::visit(clang::FunctionDecl const* decl, directive directive, description description)
	  -> std::expected<parse_result_t, next_directive>
	{
		auto result = parse_result_t{
		  .info = make_parse_result(*this, decl, directive, std::move(description.text)),
		  .current = directive,
		  .next = description.next,
		};

		if (result.info == nullptr) {
			return std::unexpected(description.next);
		}

		return result;
	}
} // namespace parser
