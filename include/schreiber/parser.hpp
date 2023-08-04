// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#ifndef SCHREIBER_PARSER_HPP
#define SCHREIBER_PARSER_HPP

#include <clang/AST/ASTContext.h>
#include <clang/AST/CommentCommandTraits.h>
#include <clang/AST/Decl.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <expected>
#include <schreiber/info.hpp>
#include <string_view>

namespace parser {
	struct command_info {
		enum directive_kind : uint8_t {
			// Global directives
			headers,
			modules,
			// Function directives
			param,
			returns,
			pre,
			post,
			throws,
			exits_via,
		};

		std::string name;
		directive_kind kind;
		std::uint16_t : 4;
		std::uint16_t is_export_command : 1;
		std::uint16_t is_param_command : 1;
		std::uint16_t is_exit_command : 1;
		std::uint16_t is_contract_command : 1;
		std::vector<std::string_view> equivalent_doxygen_commands;

		friend clang::DiagnosticBuilder const&
		operator<<(clang::DiagnosticBuilder const& builder, command_info::directive_kind const k)
		{
			switch (k) {
			case directive_kind::headers:
				return builder << "'\\headers'";
			case directive_kind::modules:
				return builder << "'\\modules'";
			case directive_kind::param:
				return builder << "'\\param'";
			case directive_kind::returns:
				return builder << "'\\returns'";
			case directive_kind::pre:
				return builder << "'\\pre'";
			case directive_kind::post:
				return builder << "'\\post'";
			case directive_kind::throws:
				return builder << "'\\throws'";
			case directive_kind::exits_via:
				return builder << "'\\exits-via'";
			}
		}
	};

#include "lexer/CommentCommandInfo.inc"

	using line_iterator = std::vector<clang::RawComment::CommentLine>::const_iterator;

	struct directive {
		command_info const* token;
		std::string_view text;
		clang::SourceLocation location;

		[[nodiscard]]
		static auto extract(std::string_view text, clang::SourceLocation begin_loc) -> directive;
	};

	struct next_directive {
		line_iterator line;
		clang::SourceLocation location;
	};

	struct description {
		std::string text;
		clang::SourceLocation location;
		next_directive next;

		[[nodiscard]]
		static auto
		extract(line_iterator first, line_iterator last, std::string_view text, clang::SourceLocation begin_loc)
		  -> description;
	};

	class parser {
	public:
		explicit parser(clang::ASTContext& context) noexcept;
		~parser();

		/// Parses a named declaration's documentation and returns its intermediate representation.
		[[nodiscard]] auto parse(clang::NamedDecl const* decl) -> std::unique_ptr<info::decl_info>;

		/// Emits a warning for an unknown directive.
		void diagnose_unknown_directive(
		  clang::SourceLocation directive_location,
		  std::string_view directive,
		  bool has_description,
		  clang::PresumedLoc comment_begin) const;

		/// Emits a warning for an unsupported Doxygen directive
		void diagnose_unsupported_doxygen_directive(
		  clang::SourceLocation directive_location,
		  clang::PresumedLoc comment_begin,
		  clang::comments::CommandInfo const* doxygen_command) const;

		/// Emits a diagnostic based on the input.
		///
		/// \param loc The source location of the diagnostic.
		/// \param diag_id The diagnostic's DiagnosticEngine ID.
		auto diagnose(clang::SourceLocation loc, unsigned int diag_id) const -> clang::DiagnosticBuilder;
	private:
		clang::ASTContext& context_;
		clang::SourceManager& source_manager_;
		clang::DiagnosticsEngine& diags_;

		/// Emits a warning for a declaration being undocumented.
		void diagnose_undocumented_decl(clang::NamedDecl const*) const;

		void parse_directives(
		  info::entity_info& decl,
		  line_iterator first,
		  line_iterator last,
		  clang::SourceLocation begin_loc);

		struct parse_result_t {
			std::unique_ptr<info::basic_info> info;
			directive current;
			next_directive next;
		};

		// Parses a function declaration's documentation.
		//
		// Function declarations support the following directives:
		//
		// :list-table:
		//   :header-rows: 1
		//
		//   * - ``\\param <parameter-name> <description>``
		//     - Describes one of the function's parameters. The ``<parameter-name>`` must match a name
		//       in the declaration immediately following the documentation.
		//   * - ``\\final-param <parameter-name>``
		//     - Tells the tool to hide all parameters following ``<parameter-name>`` when generating
		//       the function declaration component of the documentation. The parameter after
		//       ``<parameter-name>`` must have a default value.
		//   * - ``\\returns <description>``
		//     - Describes what the function returns. Not permitted if the function is annotated with
		//       ``[[noreturn]]``.
		//   * - ``\\pre <description>``
		//     - Describes a precondition for the function. May be repeated.
		//   * - ``\\post <description>``
		//     - Describes a postcondition for the function. May be repeated.
		//   * - ``\\throws <type> <description>``
		//     - Describes an exception that might be thrown by the function. The function cannot be
		//       ``noexcept``. May be repeated.
		//   * - ``\\exits-via <description>``
		//     - Describes how a function exits, aside from returning or throwing (e.g. via
		//       ``std::abort()``).
		//   * - ``\\headers <paths>``
		//     - A comma-separated list of typical include paths that the declaration can be imported
		//       from. The path validity is not checked. May be repeated.
		//   * - ``\\modules <module-name>``
		//     - A comma-separated list of modules that the declaration can be imported from.
		//       ``<module-name>`` must be a valid identifier. May be repeated.
		//
		// There isn't any need to document specifiers, standard attributes, and some non-standard
		// attributes recognised by Clang: these will be picked up and put into the documentation.
		[[nodiscard]]
		auto visit(clang::FunctionDecl const* decl, directive directive, description description)
		  -> std::expected<parse_result_t, next_directive>;
	};

	[[nodiscard]] auto to_text(clang::RawComment::CommentLine const& line) noexcept -> std::string_view;
	[[nodiscard]] auto starts_with_backslash(std::string_view c) noexcept -> bool;
	[[nodiscard]] auto is_space(char c) noexcept -> bool;

	auto parse_module_info(std::string_view text) -> std::vector<info::decl_info::module_info>;
	auto parse_header_info(std::string_view text) -> std::vector<info::decl_info::header_info>;
} // namespace parser

#endif // SCHREIBER_PARSER_HPP
