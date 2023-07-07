// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <cjdb/contracts.hpp>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/DeclTemplate.h>
#include <schreiber/info.hpp>
#include <span>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace info {
	decl_info::decl_info(
	  kind kind,
	  clang::Decl const* decl,
	  std::string description,
	  std::vector<header_info> headers,
	  std::vector<module_info> modules) noexcept
	: kind_(kind)
	, decl_((CJDB_ASSERT(decl != nullptr), decl))
	, description_(std::move(description))
	, headers_(std::move(headers))
	, modules_(std::move(modules))
	{}

	auto decl_info::decl() const noexcept -> clang::Decl const*
	{
		return decl_;
	}

	auto decl_info::description() const noexcept -> std::string_view
	{
		return description_;
	}

	auto decl_info::headers() const noexcept -> std::span<header_info const>
	{
		return headers_;
	}

	auto decl_info::modules() const noexcept -> std::span<module_info const>
	{
		return modules_;
	}

	function_info::function_info(
	  clang::FunctionDecl const* decl,
	  std::string description,
	  std::vector<parameter_info> parameters,
	  return_info returns,
	  std::vector<precondition_info> preconditions,
	  std::vector<postcondition_info> postconditions,
	  std::vector<throws_info> throws,
	  std::vector<exits_via_info> exits_via,
	  std::vector<header_info> headers,
	  std::vector<module_info> modules) noexcept
	: decl_info(kind::function_info, decl, std::move(description), std::move(headers), std::move(modules))
	, parameters_((CJDB_EXPECTS(parameters.size() <= decl->param_size()), std::move(parameters)))
	, returns_(std::move(returns))
	, preconditions_(std::move(preconditions))
	, postconditions_(std::move(postconditions))
	, throws_(std::move(throws))
	, exits_via_(std::move(exits_via))
	{}

	function_info::function_info(
	  clang::FunctionTemplateDecl const* decl,
	  std::string description,
	  std::vector<template_parameter_info> template_parameters,
	  std::vector<parameter_info> parameters,
	  noexcept_if_info exception_specifier,
	  return_info returns,
	  std::vector<precondition_info> preconditions,
	  std::vector<postcondition_info> postconditions,
	  std::vector<throws_info> throws,
	  std::vector<exits_via_info> exits_via,
	  std::vector<header_info> headers,
	  std::vector<module_info> modules) noexcept
	: decl_info(kind::function_info, decl, std::move(description), std::move(headers), std::move(modules))
	, template_parameters_(std::move(template_parameters))
	, parameters_(std::move(parameters))
	, exception_specifier_(std::move(exception_specifier))
	, returns_(std::move(returns))
	, preconditions_(std::move(preconditions))
	, postconditions_(std::move(postconditions))
	, throws_(std::move(throws))
	, exits_via_(std::move(exits_via))
	{}

	auto function_info::parameters() const noexcept -> std::span<parameter_info const>
	{
		return parameters_;
	}

	auto function_info::returns() const noexcept -> return_info const&
	{
		return returns_;
	}

	auto function_info::template_parameters() const noexcept -> std::span<template_parameter_info const>
	{
		return template_parameters_;
	}

	auto function_info::exception_specifier() const noexcept -> noexcept_if_info const&
	{
		return exception_specifier_;
	}

	auto function_info::preconditions() const noexcept -> std::span<precondition_info const>
	{
		return preconditions_;
	}

	auto function_info::postconditions() const noexcept -> std::span<postcondition_info const>
	{
		return postconditions_;
	}

	auto function_info::throws() const noexcept -> std::span<throws_info const>
	{
		return throws_;
	}

	auto function_info::exits_via() const noexcept -> std::span<exits_via_info const>
	{
		return exits_via_;
	}

	auto function_info::classof(decl_info const* const decl) -> bool
	{
		return get_kind(*decl) == kind::function_info;
	}

	parameter_info::parameter_info(clang::ParmVarDecl const* decl, std::string description) noexcept
	: decl_info(kind::parameter_info, decl, std::move(description))
	{}

	auto parameter_info::classof(decl_info const* const decl) -> bool
	{
		return get_kind(*decl) == kind::parameter_info;
	}

	template_parameter_info::template_parameter_info(
	  clang::TemplateTypeParmDecl const* decl,
	  std::string description)
	: decl_info(kind::template_parameter_info, decl, std::move(description))
	{}

	template_parameter_info::template_parameter_info(
	  clang::NonTypeTemplateParmDecl const* decl,
	  std::string description)
	: decl_info(kind::template_parameter_info, decl, std::move(description))
	{}

	template_parameter_info::template_parameter_info(
	  clang::TemplateTemplateParmDecl const* decl,
	  std::string description)
	: decl_info(kind::template_parameter_info, decl, std::move(description))
	{}

	auto template_parameter_info::classof(decl_info const* const decl) -> bool
	{
		return get_kind(*decl) == kind::template_parameter_info;
	}
} // namespace info
