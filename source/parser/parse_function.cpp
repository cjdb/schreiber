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
#include <numeric>
#include <ranges>
#include <schreiber/diagnostic_ids.hpp>
#include <schreiber/info.hpp>
#include <schreiber/parser.hpp>
#include <string_view>
#include <vector>

namespace parser {
	namespace stdr = std::ranges;
	namespace stdv = std::views;
	using namespace std::string_view_literals;

	template<stdr::forward_range R>
	[[nodiscard]] static auto to_string_view(R&& r) noexcept -> std::string_view
	{
		return {r.begin(), stdr::next(r.begin(), r.end())};
	}

	namespace {
		struct parse_info_result {
			struct return_result {
				info::return_info value;
				clang::SourceLocation location;
			};

			std::vector<info::parameter_info> param;
			return_result returns;
			std::vector<info::precondition_info> pre;
			std::vector<info::postcondition_info> post;
			std::vector<info::throws_info> throws;
			std::vector<info::exits_via_info> exits_via;
			std::vector<info::header_info> headers;
			std::vector<info::module_info> modules;
		};
	} // namespace

	[[nodiscard]]
	static auto parse_info(
	  parser const& p,
	  line_iterator first,
	  line_iterator last,
	  clang::FunctionDecl const* decl,
	  clang::SourceLocation begin_loc,
	  bool const has_description) -> parse_info_result
	{
		auto result = parse_info_result{};

		while (first != last) {
			auto const text = std::string_view(first->Text);
			auto const begin = first->Begin;
			assert(text.starts_with('\\'));

			auto const directive = directive::extract(text, begin_loc);
			auto const description =
			  description::extract(first, last, std::string_view(directive.text.end(), text.end()), begin_loc);

			auto const token = lex(directive.text);
			if (token == nullptr) {
				p.diagnose_unknown_directive(begin_loc, directive.text, has_description, begin);
				begin_loc = description.range.getEnd();
				first = description.next_directive;
				continue;
			}

			switch (token->kind) {
			case command_info::headers:
				result.headers.append_range(parse_header_info(description.text));
				break;
			case command_info::modules:
				result.modules.append_range(parse_module_info(description.text));
				break;
			case command_info::param: {
				auto name = to_string_view(
				  absl::StripLeadingAsciiWhitespace(description.text)
				  | stdv::take_while(std::not_fn(is_space)));
				auto parameters = decl->parameters();
				auto parameter = stdr::find_if(parameters, [name](clang::ParmVarDecl const* const p) {
					return std::string_view{p->getName()} == name;
				});
				if (parameter == decl->param_end()) {
					auto const report_loc =
					  begin_loc.getLocWithOffset(static_cast<int>(directive.text.size() + 2));
					p.diagnose(report_loc, diag::err_unknown_parameter) << /*is_template=*/false << name << decl;
					p.diagnose(report_loc, diag::note_unknown_parameter) << command_info::param;
					break;
				}

				auto const prior_definition =
				  stdr::find_if(result.param, [name](info::parameter_info const& x) {
					  return std::string_view{llvm::dyn_cast<clang::ParmVarDecl>(x.decl())->getName()} == name;
				  });
				if (prior_definition != result.param.end()) {
					constexpr auto param = 1;
					p.diagnose(begin_loc, diag::err_repeated_directive)
					  << command_info::param << param << *parameter << decl;
					p.diagnose(prior_definition->source_range().getBegin(), clang::diag::note_previous_definition);
					break;
				}
				result.param.emplace_back(
				  clang::SourceRange(begin_loc, description.range.getEnd()),
				  *parameter,
				  std::string(absl::StripAsciiWhitespace(
				    std::string_view(name.end(), std::string_view{description.text}.end()))));
				break;
			}
			case command_info::returns:
				if (result.returns.location.isValid()) {
					p.diagnose(begin_loc, diag::err_repeated_directive)
					  << command_info::returns << /*directive=*/0 << decl;
					p.diagnose(result.returns.location, clang::diag::note_previous_definition);
					break;
				}
				result.returns = {
				  .value = info::return_info{.data = std::string(absl::StripAsciiWhitespace(description.text))},
				  .location = begin_loc,
				};
				break;
			case command_info::pre:
				result.pre.emplace_back(std::string(absl::StripAsciiWhitespace(description.text)));
				break;
			case command_info::post:
				result.post.emplace_back(std::string(absl::StripAsciiWhitespace(description.text)));
				break;
			case command_info::throws:
				result.throws.emplace_back(std::string(absl::StripAsciiWhitespace(description.text)));
				break;
			case command_info::exits_via:
				result.exits_via.emplace_back(std::string(absl::StripAsciiWhitespace(description.text)));
				break;
			default:
				std::unreachable();
			}

			begin_loc = description.range.getEnd();
			first = description.next_directive;
		}

		return result;
	}

	auto parser::visit(clang::FunctionDecl const* decl) const -> std::unique_ptr<info::decl_info>
	{
		auto const raw_comment = context_.getRawCommentForDeclNoCache(decl);
		if (raw_comment == nullptr) {
			diagnose(decl->getLocation(), diag::warn_undocumented_decl) << decl;
			diagnose(decl->getLocation(), diag::note_undocumented_decl) << decl;
			return nullptr;
		}

		auto const lines = raw_comment->getFormattedLines(source_manager_, diags_);

		auto const description = llvm::iterator_range(
		  lines.begin(),
		  stdr::find_if(lines, starts_with_backslash, &clang::RawComment::CommentLine::Text));
		auto text_description = llvm::join(description | stdv::transform(to_text), "\n");

		auto const initial_comment_offset =
		  lines[0].Begin.getColumn() - source_manager_.getPresumedColumnNumber(raw_comment->getBeginLoc());
		auto info = parse_info(
		  *this,
		  description.end(),
		  lines.end(),
		  decl,
		  raw_comment->getBeginLoc().getLocWithOffset(std::accumulate(
		    // Skip the first one so we don't double-count the first line (which is obtained via
		    /// `getBeginLoc()`).
		    stdr::next(description.begin(), 1, description.end()),
		    description.end(),
		    static_cast<int>(text_description.size() + initial_comment_offset),
		    [](int const x, clang::RawComment::CommentLine const& comment) {
			    return x + static_cast<int>(comment.Begin.getColumn());
		    })),
		  not description.empty());

		return std::make_unique<info::function_info>(
		  decl,
		  std::move(text_description),
		  std::move(info.param),
		  std::move(info.returns.value),
		  std::move(info.pre),
		  std::move(info.post),
		  std::move(info.throws),
		  std::move(info.exits_via),
		  std::move(info.headers),
		  std::move(info.modules));
	}
} // namespace parser
