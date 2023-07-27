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

	auto lex(std::string_view name) -> command_info const*;

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
	private:
		clang::ASTContext& context_;
		clang::SourceManager& source_manager_;
		clang::DiagnosticsEngine& diags_;

		/// Emits a warning for a declaration being undocumented.
		void diagnose_undocumented_decl(clang::NamedDecl const*) const;

		void diagnose_unsupported_doxygen_directive(
		  clang::SourceLocation directive_location,
		  clang::PresumedLoc comment_begin,
		  clang::comments::CommandInfo const* doxygen_command) const;
	};
} // namespace parser

#endif // SCHREIBER_PARSER_HPP
