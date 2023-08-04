// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <algorithm>
#include <cjdb/contracts.hpp>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <schreiber/diagnostic_ids.hpp>
#include <schreiber/info.hpp>
#include <schreiber/parser.hpp>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace stdr = std::ranges;
namespace info {
	basic_info::basic_info(kind const kind, std::string description, clang::SourceLocation const location)
	: kind_(kind)
	, description_(std::move(description))
	, location_(location)
	{}

	auto basic_info::get_kind(basic_info const& info) noexcept -> kind
	{
		return info.kind_;
	}

	auto basic_info::description() const noexcept -> std::string_view
	{
		return description_;
	}

	auto basic_info::location() const noexcept -> clang::SourceLocation
	{
		return location_;
	}

	decl_info::decl_info(kind k, clang::Decl const* decl, std::string description, clang::SourceLocation location)
	: basic_info(k, std::move(description), location)
	, decl_((CJDB_EXPECTS(decl != nullptr), decl))
	{}

	auto decl_info::decl() const noexcept -> clang::Decl const*
	{
		return decl_;
	}

	decl_info::header_info::header_info(std::string description, clang::SourceLocation const location)
	: basic_info(basic_info::kind::header_info, std::move(description), location)
	{}

	auto decl_info::header_info::classof(basic_info const* info) -> bool
	{
		return get_kind(*info) == kind::header_info;
	}

	decl_info::module_info::module_info(std::string description, clang::SourceLocation const location)
	: basic_info(basic_info::kind::module_info, std::move(description), location)
	{}

	auto decl_info::module_info::classof(basic_info const* info) -> bool
	{
		return get_kind(*info) == kind::module_info;
	}

	void entity_info::add_header(header_info header)
	{
		headers_.push_back(std::move(header));
	}

	void entity_info::add_header(std::vector<header_info> headers)
	{
		headers_.insert(
		  headers_.end(),
		  std::move_iterator(headers.begin()),
		  std::move_iterator(headers.end()));
	}

	void entity_info::add_module(module_info module)
	{
		modules_.push_back(std::move(module));
	}

	void entity_info::add_module(std::vector<module_info> modules)
	{
		modules_.insert(
		  modules_.end(),
		  std::move_iterator(modules.begin()),
		  std::move_iterator(modules.end()));
	}

	auto entity_info::headers() const noexcept -> std::span<header_info const>
	{
		return headers_;
	}

	auto entity_info::modules() const noexcept -> std::span<module_info const>
	{
		return modules_;
	}

	void entity_info::store(parser::parser const&, parser::directive, basic_info* info)
	{
		switch (get_kind(*info)) {
		case kind::header_info:
			add_header(std::move(*llvm::dyn_cast<header_info>(info)));
			break;
		case kind::module_info:
			add_module(std::move(*llvm::dyn_cast<module_info>(info)));
			break;
		default:
			assert(false and "unhandled storage");
		}
	}

	function_info::function_info(
	  clang::FunctionDecl const* const decl,
	  std::string description,
	  clang::SourceLocation const location)
	: entity_info(kind::function_info, decl, std::move(description), location)
	{}

	void
	function_info::add_parameter(parser::parser const& p, parser::directive directive, parameter_info info)
	{
		auto const prior_definition = stdr::find_if(parameters_, [&info](info::parameter_info const& x) {
			return x.decl() == info.decl();
		});

		if (prior_definition != parameters_.end()) {
			auto param_decl = llvm::dyn_cast<clang::ParmVarDecl>(info.decl());
			constexpr auto param = 1;
			p.diagnose(directive.location, diag::err_repeated_directive)
			  << parser::command_info::param << param << param_decl
			  << llvm::dyn_cast<clang::FunctionDecl>(decl());
			p.diagnose(prior_definition->location(), clang::diag::note_previous_definition);
		}

		parameters_.push_back(std::move(info));
	}

	auto function_info::return_info::classof(basic_info const* info) -> bool
	{
		return get_kind(*info) == kind::return_info;
	}

	auto function_info::precondition_info::classof(basic_info const* info) -> bool
	{
		return get_kind(*info) == kind::precondition_info;
	}

	auto function_info::postcondition_info::classof(basic_info const* info) -> bool
	{
		return get_kind(*info) == kind::postcondition_info;
	}

	auto function_info::throws_info::classof(basic_info const* info) -> bool
	{
		return get_kind(*info) == kind::throws_info;
	}

	auto function_info::exits_via_info::classof(basic_info const* info) -> bool
	{
		return get_kind(*info) == kind::exits_via_info;
	}

	auto function_info::parameters() const noexcept -> std::span<parameter_info const>
	{
		return parameters_;
	}

	void
	function_info::add_returns(parser::parser const& p, parser::directive directive, return_info info)
	{
		if (returns_.has_value()) {
			p.diagnose(directive.location, diag::err_repeated_directive)
			  << parser::command_info::returns << /*directive=*/0
			  << llvm::dyn_cast<clang::FunctionDecl>(decl());
			p.diagnose(returns_->location(), clang::diag::note_previous_definition);
			return;
		}
		returns_ = std::move(info);
	}

	auto function_info::returns() const noexcept -> std::optional<return_info> const&
	{
		return returns_;
	}

	void
	function_info::add_precondition(parser::parser const&, parser::directive, precondition_info info)
	{
		preconditions_.push_back(std::move(info));
	}

	auto function_info::preconditions() const noexcept -> std::span<precondition_info const>
	{
		return preconditions_;
	}

	void
	function_info::add_postcondition(parser::parser const&, parser::directive, postcondition_info info)
	{
		postconditions_.push_back(std::move(info));
	}

	auto function_info::postconditions() const noexcept -> std::span<postcondition_info const>
	{
		return postconditions_;
	}

	void function_info::add_throws(parser::parser const&, parser::directive, throws_info info)
	{
		throws_.push_back(std::move(info));
	}

	auto function_info::throws() const noexcept -> std::span<throws_info const>
	{
		return throws_;
	}

	void function_info::add_exits_via(parser::parser const&, parser::directive, exits_via_info info)
	{
		exits_via_.push_back(std::move(info));
	}

	auto function_info::exits_via() const noexcept -> std::span<exits_via_info const>
	{
		return exits_via_;
	}

	void function_info::store(parser::parser const& p, parser::directive directive, basic_info* info)
	{
		assert(info != nullptr);
		switch (get_kind(*info)) {
		case kind::parameter_info:
			add_parameter(p, directive, std::move(*llvm::dyn_cast<parameter_info>(info)));
			break;
		case kind::return_info:
			add_returns(p, directive, std::move(*llvm::dyn_cast<return_info>(info)));
			break;
		case kind::precondition_info:
			add_precondition(p, directive, std::move(*llvm::dyn_cast<precondition_info>(info)));
			break;
		case kind::postcondition_info:
			add_postcondition(p, directive, std::move(*llvm::dyn_cast<postcondition_info>(info)));
			break;
		case kind::throws_info:
			add_throws(p, directive, std::move(*llvm::dyn_cast<throws_info>(info)));
			break;
		case kind::exits_via_info:
			add_exits_via(p, directive, std::move(*llvm::dyn_cast<exits_via_info>(info)));
			break;
		default:
			entity_info::store(p, directive, info);
		}
	}

	auto function_info::classof(basic_info const* const decl) -> bool
	{
		return get_kind(*decl) == kind::function_info;
	}

	parameter_info::parameter_info(
	  clang::SourceLocation const source_location,
	  clang::ParmVarDecl const* decl,
	  std::string description)
	: decl_info(kind::parameter_info, decl, std::move(description), source_location)
	{}

	auto parameter_info::classof(basic_info const* const decl) -> bool
	{
		return get_kind(*decl) == kind::parameter_info;
	}

	template_parameter_info::template_parameter_info(
	  clang::SourceLocation const source_location,
	  clang::TemplateTypeParmDecl const* const decl,
	  std::string description)
	: decl_info(kind::template_parameter_info, decl, std::move(description), source_location)
	{}

	template_parameter_info::template_parameter_info(
	  clang::SourceLocation const source_location,
	  clang::NonTypeTemplateParmDecl const* const decl,
	  std::string description)
	: decl_info(kind::template_parameter_info, decl, std::move(description), source_location)
	{}

	template_parameter_info::template_parameter_info(
	  clang::SourceLocation const source_location,
	  clang::TemplateTemplateParmDecl const* const decl,
	  std::string description)
	: decl_info(kind::template_parameter_info, decl, std::move(description), source_location)
	{}

	auto template_parameter_info::classof(basic_info const* const decl) -> bool
	{
		return get_kind(*decl) == kind::template_parameter_info;
	}

	function_info::return_info::return_info(std::string description, clang::SourceLocation const location)
	: basic_info(kind::return_info, std::move(description), location)
	{}

	function_info::precondition_info::precondition_info(
	  std::string description,
	  clang::SourceLocation const location)
	: basic_info(kind::precondition_info, std::move(description), location)
	{}

	function_info::postcondition_info::postcondition_info(
	  std::string description,
	  clang::SourceLocation const location)
	: basic_info(kind::postcondition_info, std::move(description), location)
	{}

	function_info::throws_info::throws_info(std::string description, clang::SourceLocation const location)
	: basic_info(kind::throws_info, std::move(description), location)
	{}

	function_info::exits_via_info::exits_via_info(std::string description, clang::SourceLocation const location)
	: basic_info(kind::exits_via_info, std::move(description), location)
	{}

} // namespace info
