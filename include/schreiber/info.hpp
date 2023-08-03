// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#ifndef SCHREIBER_INFO_HPP
#define SCHREIBER_INFO_HPP

#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/Basic/SourceLocation.h>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <vector>

namespace parser {
	class parser;
	struct directive;
} // namespace parser

namespace info {
	/// Base class for the documentation for all declarations. This class---and all its derivatives---
	/// record what is documented as comments (such as this). They do not usually record properties
	/// such as type information or attributes, since libClang already records this data.
	class basic_info {
	public:
		virtual ~basic_info() = default;

		[[nodiscard]] auto description() const noexcept -> std::string_view;
		[[nodiscard]] auto location() const noexcept -> clang::SourceLocation;

		friend auto operator==(basic_info const&, basic_info const&) -> bool = default;
	protected:
		enum class kind {
			header_info,
			module_info,
			precondition_info,
			postcondition_info,
			return_info,
			noexcept_if_info,
			throws_info,
			exits_via_info,
			parameter_info,
			template_parameter_info,
			function_info,
			function_template_info,
		};

		basic_info(kind k, std::string description, clang::SourceLocation location);

		[[nodiscard]] static auto get_kind(basic_info const& info) noexcept -> kind;
	private:
		kind kind_;
		std::string description_;
		clang::SourceLocation location_;
	};

	/// Base class for describing a named declaration, such as an entity or a (template) parameter.
	class decl_info : public basic_info {
	public:
		/// Identifies a header that a declaration can be found in.
		struct header_info final : basic_info {
			header_info(std::string description, clang::SourceLocation location);
			static auto classof(basic_info const* decl) -> bool;
		};

		/// Identifies a module that a declaration can be found in.
		struct module_info final : basic_info {
			module_info(std::string description, clang::SourceLocation location);
			static auto classof(basic_info const* decl) -> bool;
		};

		[[nodiscard]] auto decl() const noexcept -> clang::Decl const*;
	protected:
		decl_info(kind k, clang::Decl const* decl, std::string description, clang::SourceLocation location);
	private:
		clang::Decl const* decl_;
	};

	/// Describes a template parameter.
	class template_parameter_info : public decl_info {
	public:
		/// Constructs a ``template_parameter_info`` object.
		///
		/// \param source_range A location indicating where in the source file the directive is.
		/// \param decl A pointer to the template parameter's declaration.
		/// \param description A description of the declaration.
		template_parameter_info(
		  clang::SourceLocation source_range,
		  clang::TemplateTypeParmDecl const* decl,
		  std::string description);
		template_parameter_info(
		  clang::SourceLocation source_range,
		  clang::NonTypeTemplateParmDecl const* decl,
		  std::string description);
		template_parameter_info(
		  clang::SourceLocation source_range,
		  clang::TemplateTemplateParmDecl const* decl,
		  std::string description);

		/// Determines whether a ``decl_info const*`` points to a ``template_parameter_info`` object.
		static auto classof(basic_info const* decl) -> bool;
	};

	/// Describes a function parameter.
	class parameter_info : public decl_info {
	public:
		/// Constructs a ``parameter_info`` object.
		///
		/// \param source_range A location indicating where in the source file the directive is
		/// \param decl A pointer to the parameter's declaration.
		/// \param description A description of the declaration.
		parameter_info(
		  clang::SourceLocation source_range,
		  clang::ParmVarDecl const* decl,
		  std::string description);

		/// Determines whether a ``decl_info const*`` points to a ``parameter_info`` object.
		static auto classof(basic_info const* decl) -> bool;
	};

	/// Base class for describing entities.
	class entity_info : public decl_info {
	public:
		/// Documents a header that the entity can be found in.
		void add_header(header_info header);
		void add_header(std::vector<header_info> headers);

		/// Documents a module that the entity can be found in.
		void add_module(module_info module);
		void add_module(std::vector<module_info> modules);

		/// Returns which headers the declaration can be imported from.
		[[nodiscard]] auto headers() const noexcept -> std::span<header_info const>;

		/// Returns which modules the declaration can be imported from.
		[[nodiscard]] auto modules() const noexcept -> std::span<module_info const>;

		/// Adds a unit of information to the entity's graph.
		virtual void store(parser::parser const& p, parser::directive directive, basic_info* info) = 0;
	protected:
		using decl_info::decl_info;
	private:
		std::vector<header_info> headers_;
		std::vector<module_info> modules_;
	};

	class function_info : public entity_info {
	public:
		function_info(clang::FunctionDecl const* decl, std::string description, clang::SourceLocation location);

		/// Returns descriptions of the function's parameters.
		[[nodiscard]] auto parameters() const noexcept -> std::span<parameter_info const>;

		/// Describes what a function returns.
		struct return_info final : basic_info {
			return_info(std::string description, clang::SourceLocation location);
			static auto classof(basic_info const* info) -> bool;
		};

		/// Returns a description of what the function returns.
		[[nodiscard]] auto returns() const noexcept -> std::optional<return_info> const&;

		/// Describes a precondition.
		struct precondition_info final : basic_info {
			precondition_info(std::string description, clang::SourceLocation location);
			static auto classof(basic_info const* info) -> bool;
		};

		/// Returns the set of preconditions.
		[[nodiscard]] auto preconditions() const noexcept -> std::span<precondition_info const>;

		/// Describes a postcondition.
		struct postcondition_info final : basic_info {
			postcondition_info(std::string description, clang::SourceLocation location);
			static auto classof(basic_info const* info) -> bool;
		};

		/// Returns the set of postconditions.
		[[nodiscard]] auto postconditions() const noexcept -> std::span<postcondition_info const>;

		/// Describes an exception that a function may throw.
		struct throws_info final : basic_info {
			throws_info(std::string description, clang::SourceLocation location);
			static auto classof(basic_info const* info) -> bool;
		};

		/// Returns the set of exceptions a function might throw.
		[[nodiscard]] auto throws() const noexcept -> std::span<throws_info const>;

		/// Describes how a function can exit, other than returning and throwing (e.g.
		/// ``std::abort();``).
		struct exits_via_info final : basic_info {
			exits_via_info(std::string description, clang::SourceLocation location);
			static auto classof(basic_info const* info) -> bool;
		};

		/// Returns the set of ways a function might exit, other than returning or throwing.
		[[nodiscard]] auto exits_via() const noexcept -> std::span<exits_via_info const>;

		void store(parser::parser const& p, parser::directive directive, basic_info* info) override;

		/// Determines whether a ``decl_info const*`` points to a ``function_info`` object.
		static auto classof(basic_info const* decl) -> bool;
	protected:
		function_info(
		  clang::FunctionTemplateDecl const* decl,
		  std::string description,
		  clang::SourceLocation location);

		/// Documents a function parameter.
		void add_parameter(parser::parser const& p, parser::directive directive, parameter_info info);
		/// Documents what the function returns.
		void add_returns(parser::parser const& p, parser::directive directive, return_info info);
		/// Documents a precondition.
		void
		add_precondition(parser::parser const& p, parser::directive directive, precondition_info info);
		/// Documents a postcondition.
		void
		add_postcondition(parser::parser const& p, parser::directive directive, postcondition_info info);
		/// Documents ways a function might throw.
		void add_throws(parser::parser const& p, parser::directive directive, throws_info info);
		/// Documents ways a function might exit, other than returning or throwing.
		void add_exits_via(parser::parser const& p, parser::directive directive, exits_via_info info);
	private:
		std::vector<parameter_info> parameters_;
		std::optional<return_info> returns_;
		std::vector<precondition_info> preconditions_;
		std::vector<postcondition_info> postconditions_;
		std::vector<throws_info> throws_;
		std::vector<exits_via_info> exits_via_;
	};

	class function_template_info final : public function_info {
	public:
		function_template_info(
		  clang::FunctionTemplateDecl const* decl,
		  std::string description,
		  clang::SourceLocation location);

		/// Documents a template parameter.
		void add_template_parameter(template_parameter_info info);

		/// Returns descriptions of the function's template parameters.
		[[nodiscard]] auto
		template_parameters() const noexcept -> std::span<template_parameter_info const>;

		/// Describes the conditions upon which a function template is ``noexcept``.
		struct noexcept_if_info final : basic_info {
			noexcept_if_info(std::string_view description, clang::SourceLocation location);
		};

		/// Documents a conditional noexcept specifier.
		void add_noexcept_if(noexcept_if_info info);

		/// Returns a description of the function's noexcept specifier (if any).
		[[nodiscard]] auto noexcept_if() const noexcept -> std::optional<noexcept_if_info> const&;

		/// Determines whether a ``decl_info const*`` points to a ``function_info`` object.
		static auto classof(basic_info const* decl) -> bool;
	private:
		std::vector<template_parameter_info> template_parameters_;
		std::optional<noexcept_if_info> noexcept_if_;
	};
} // namespace info

#endif // SCHREIBER_INFO_HPP
