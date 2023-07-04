// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#ifndef SCHREIBER_INFO_HPP
#define SCHREIBER_INFO_HPP

#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace info::detail {
	enum class sequences {
		headers,
		modules,
		preconditions,
		postconditions,
		noexcept_if,
		returns,
		throws,
		exits_via,
	};

	template<sequences>
	struct sequenced_info final {
		std::string data;

		friend auto operator==(sequenced_info const&, sequenced_info const&) -> bool = default;
	};
}; // namespace info::detail

namespace info {
	/// Identifies a header that a declaration can be found in.
	using header_info = detail::sequenced_info<detail::sequences::headers>;

	/// Identifies a module that a declaration can be found in.
	using module_info = detail::sequenced_info<detail::sequences::modules>;

	/// Base class for the documentation for all declarations. This class---and all its derivatives---
	/// record what is documented as comments (such as this). They do not usually record properties
	/// such as type information or attributes, since libClang already records this data.
	class decl_info {
	public:
		virtual ~decl_info() = default;

		/// Returns a pointer to the declaration in the Clang AST.
		[[nodiscard]] virtual auto decl() const noexcept -> clang::Decl const*;

		/// Returns a description of the declaration.
		[[nodiscard]] auto description() const noexcept -> std::string_view;

		/// Returns which headers the declaration can be imported from.
		[[nodiscard]] auto headers() const noexcept -> std::span<header_info const>;

		/// Returns which modules the declaration can be imported from.
		[[nodiscard]] auto modules() const noexcept -> std::span<module_info const>;

		friend auto operator==(decl_info const&, decl_info const&) -> bool = default;
	protected:
		decl_info(
		  clang::Decl const* decl,
		  std::string description,
		  std::vector<header_info> headers = {},
		  std::vector<module_info> modules = {}) noexcept;
	private:
		clang::Decl const* decl_;
		std::string description_;
		std::vector<header_info> headers_;
		std::vector<module_info> modules_;
	};

	/// Describes a template parameter.
	class template_parameter_info : public decl_info {
	public:
		/// Constructs a ``template_parameter_info`` object.
		///
		/// \param decl A pointer to the template parameter's declaration.
		/// \param description A description of the declaration.
		template_parameter_info(clang::TemplateTypeParmDecl const* decl, std::string description);
		template_parameter_info(clang::NonTypeTemplateParmDecl const* decl, std::string description);
		template_parameter_info(clang::TemplateTemplateParmDecl const* decl, std::string description);

		friend auto
		operator==(template_parameter_info const&, template_parameter_info const&) -> bool = default;
	private:
		bool show_default_;
	};

	/// Describes a function parameter.
	class parameter_info : public decl_info {
	public:
		/// Constructs a ``parameter_info`` object.
		///
		/// \param decl A pointer to the parameter's declaration.
		/// \param description A description of the declaration.
		parameter_info(clang::ParmVarDecl const* decl, std::string description) noexcept;

		friend auto operator==(parameter_info const&, parameter_info const&) -> bool = default;
	private:
		bool show_default_;
	};

	/// Describes a precondition.
	using precondition_info = detail::sequenced_info<detail::sequences::preconditions>;

	/// Describes a postcondition.
	using postcondition_info = detail::sequenced_info<detail::sequences::postconditions>;

	/// Describes what a function returns.
	using return_info = detail::sequenced_info<detail::sequences::returns>;

	/// Describes the conditions upon which a function template is ``noexcept``.
	using noexcept_if_info = detail::sequenced_info<detail::sequences::noexcept_if>;

	/// Describes an exception that a function may throw.
	using throws_info = detail::sequenced_info<detail::sequences::throws>;

	/// Describes how a function can exit, other than returning and throwing (e.g. ``std::abort();``).
	using exits_via_info = detail::sequenced_info<detail::sequences::exits_via>;

	/// Describes a C++ function.
	class function_info final : public decl_info {
	public:
		/// Constructs a function_info object for a function. Returns a a vector of ``info_error``
		/// objects if there are too many parameters for the function, or when the described parameters
		/// don't match the names of their declaration's counterparts (one for each error).
		///
		/// \param decl A pointer to the function declaration.
		/// \param description A description of the declaration.
		/// \param parameters A set of descriptions containing the function's parameters.
		/// \param returns A description of what the function returns.
		/// \param preconditions A set of descriptions for the function's preconditions.
		/// \param postconditions A set of descriptions for the function's postconditions.
		/// \param throws A set of exceptions that the function might throw.
		/// \param exits_via A set of ways the function might exit other than returning and throwing.
		/// \param headers A set of headers that the function can be imported from.
		/// \param modules A set of modules that the function can be imported from.
		function_info(
		  clang::FunctionDecl const* decl,
		  std::string description,
		  std::vector<parameter_info> parameters,
		  return_info returns,
		  std::vector<precondition_info> preconditions,
		  std::vector<postcondition_info> postconditions,
		  std::vector<throws_info> throws,
		  std::vector<exits_via_info> exits_via,
		  std::vector<header_info> headers,
		  std::vector<module_info> modules) noexcept;

		/// Constructs a function_info object for a function template.
		///
		/// \param decl A pointer to the function declaration.
		/// \param description A description of the declaration.
		/// \param template_parameters A set of descriptions containing the function's template
		/// parameters.
		/// \param parameters A set of descriptions containing the function's parameters.
		/// \param exception_specifier A description of a conditional noexcept specifier.
		/// \param returns A description of what the function returns.
		/// \param throws A set of exceptions that the function might throw.
		/// \param exits_via A set of ways the function might exit other than returning and throwing.
		/// \param preconditions A set of descriptions for the function's preconditions.
		/// \param postconditions A set of descriptions for the function's postconditions.
		/// \param headers A set of headers that the function can be imported from.
		/// \param modules A set of modules that the function can be imported from.
		function_info(
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
		  std::vector<module_info> modules) noexcept;

		/// Returns descriptions of the function's parameters.
		[[nodiscard]] auto parameters() const noexcept -> std::span<parameter_info const>;

		/// Returns a description of what the function returns.
		[[nodiscard]] auto returns() const noexcept -> return_info const&;

		/// Returns descriptions of the function's template parameters.
		[[nodiscard]]
		auto template_parameters() const noexcept -> std::span<template_parameter_info const>;

		/// Returns a description of the function's noexcept specifier (if any).
		[[nodiscard]] auto exception_specifier() const noexcept -> noexcept_if_info const&;

		/// Returns the set of preconditions.
		[[nodiscard]] auto preconditions() const noexcept -> std::span<precondition_info const>;

		/// Returns the set of postconditions.
		[[nodiscard]] auto postconditions() const noexcept -> std::span<postcondition_info const>;

		/// Returns the set of exceptions that a function might throw.
		[[nodiscard]] auto throws() const noexcept -> std::span<throws_info const>;

		/// Returns the set of ways a function might exit, other than returning or throwing.
		[[nodiscard]] auto exits_via() const noexcept -> std::span<exits_via_info const>;

		friend auto operator==(function_info const&, function_info const&) -> bool = default;
	private:
		std::vector<template_parameter_info> template_parameters_;
		std::vector<parameter_info> parameters_;
		noexcept_if_info exception_specifier_;
		return_info returns_;
		std::vector<precondition_info> preconditions_;
		std::vector<postcondition_info> postconditions_;
		std::vector<throws_info> throws_;
		std::vector<exits_via_info> exits_via_;
	};
} // namespace info

#endif // SCHREIBER_INFO_HPP
