// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <optional>
#include <schreiber/diagnostic_ids.hpp>
#include <schreiber/info.hpp>
#include <schreiber/parser.hpp>
#include <set>

namespace {
	struct compare_locations {
		[[nodiscard]] bool operator()(clang::Decl const* const x, clang::Decl const* const y) const noexcept
		{
			assert(x != nullptr);
			assert(y != nullptr);

			return x->getLocation() < y->getLocation();
		}
	};

	std::set<clang::Decl const*, compare_locations> undocumented_declarations;
	std::set<clang::Decl const*, compare_locations> documented_declarations;

	enum class entity {
		class_template,
		class_,
		concept_,
		data_member,
		deduction_guide,
		enum_,
		enum_constant,
		function_template,
		function,
		struct_,
		type_alias_template,
		type_alias,
		typedef_,
		union_,
		variable_template,
		variable,
		constructor,
		default_constructor,
		move_constructor,
		copy_constructor,
		destructor,
		move_assignment,
		copy_assignment,
		conversion,
	};

	clang::DiagnosticBuilder const& operator<<(clang::DiagnosticBuilder const& b, entity const k)
	{
		return b << static_cast<int>(k);
	}

	[[nodiscard]] auto get_kind(clang::Decl const* const decl) -> std::optional<entity>
	{
		switch (decl->getKind()) {
		case clang::Decl::Kind::Concept:
			return entity::concept_;
		case clang::Decl::Kind::CXXRecord: {
			auto const record = llvm::dyn_cast<clang::CXXRecordDecl>(decl);
			return record->isClass() ? entity::class_
			     : record->isStruct()
			       ? entity::struct_
			       : entity::union_;
		}
		case clang::Decl::Kind::ClassTemplate:
			return entity::class_template;
		case clang::Decl::Kind::Enum:
			return entity::enum_;
		case clang::Decl::Kind::EnumConstant:
			return entity::enum_constant;
		case clang::Decl::Kind::Function:
			return entity::function;
		case clang::Decl::Kind::FunctionTemplate:
			return entity::function_template;
		case clang::Decl::Kind::TypeAlias:
			return entity::type_alias;
		case clang::Decl::Kind::TypeAliasTemplate:
			return entity::type_alias_template;
		case clang::Decl::Kind::Typedef:
			return entity::typedef_;
		case clang::Decl::Kind::Var:
			return decl->getDeclContext()->isRecord() ? entity::data_member : entity::variable;
		case clang::Decl::Kind::VarTemplate:
			return entity::variable_template;
		case clang::Decl::Kind::CXXMethod: {
			auto const member = llvm::dyn_cast<clang::CXXMethodDecl>(decl);
			return member->isMoveAssignmentOperator() ? entity::move_assignment
			     : member->isCopyAssignmentOperator()
			       ? entity::copy_assignment
			       : entity::function;
		}
		case clang::Decl::Kind::CXXConstructor: {
			auto const ctor = llvm::dyn_cast<clang::CXXConstructorDecl>(decl);
			return ctor->isDefaultConstructor() ? entity::default_constructor
			     : ctor->isMoveConstructor()    ? entity::move_constructor
			     : ctor->isCopyConstructor()
			       ? entity::copy_constructor
			       : entity::constructor;
		}
		case clang::Decl::Kind::CXXDestructor:
			return entity::destructor;
		case clang::Decl::Kind::CXXConversion:
			return entity::conversion;
		case clang::Decl::Kind::CXXDeductionGuide:
			return entity::deduction_guide;
		default:
			return std::nullopt;
		}
	}

	enum class prefix_with {
		nothing,
		member,
		nested,
	};

	clang::DiagnosticBuilder const& operator<<(clang::DiagnosticBuilder const& b, prefix_with const k)
	{
		return b << static_cast<int>(k);
	}

	[[nodiscard]] auto prefix(entity const kind) noexcept -> prefix_with
	{
		switch (kind) {
		case entity::concept_:
		case entity::constructor:
		case entity::conversion:
		case entity::copy_assignment:
		case entity::copy_constructor:
		case entity::data_member:
		case entity::deduction_guide:
		case entity::default_constructor:
		case entity::destructor:
		case entity::enum_constant:
		case entity::move_assignment:
		case entity::move_constructor:
		case entity::variable:
			return prefix_with::nothing;
		case entity::class_:
		case entity::class_template:
		case entity::enum_:
		case entity::struct_:
		case entity::union_:
		case entity::variable_template:
			return prefix_with::nested;
		case entity::function:
		case entity::function_template:
		case entity::type_alias:
		case entity::type_alias_template:
		case entity::typedef_:
			return prefix_with::member;
		}
	}
} // namespace

namespace parser {
#include "lexer/CommentCommandInfo.inc"

	namespace stdr = std::ranges;

	parser::parser(clang::ASTContext& context) noexcept
	: context_(context)
	, source_manager_(context.getSourceManager())
	, diags_(context.getDiagnostics())
	{}

	parser::~parser()
	{
		for (auto const i : undocumented_declarations) {
			diagnose_undocumented_decl(llvm::dyn_cast<clang::NamedDecl>(i));
		}
	}

	auto parser::parse(clang::NamedDecl const* const decl) -> std::unique_ptr<info::decl_info>
	{
		auto decl_context = decl->getDeclContext();
		if (decl_context->isFunctionOrMethod()) {
			return nullptr;
		}

		auto const raw_comment = context_.getRawCommentForDeclNoCache(decl);
		if (raw_comment == nullptr) {
			if (auto const canonical = decl->getCanonicalDecl();
			    not documented_declarations.contains(canonical))
			{
				undocumented_declarations.insert(canonical);
			}
			return nullptr;
		}

		return nullptr;
	}

	void parser::diagnose_undocumented_decl(clang::NamedDecl const* const decl) const
	{
		auto const decl_context = decl->getDeclContext();
		auto const kind = get_kind(decl);
		if (not kind.has_value()) {
			return;
		}

		enum { entity, smf, member, specialisation };

		if (auto const parent = decl_context->getOuterLexicalRecordContext()) {
			switch (*kind) {
			case entity::default_constructor:
			case entity::copy_constructor:
			case entity::move_constructor:
			case entity::move_assignment:
			case entity::copy_assignment:
			case entity::constructor:
			case entity::destructor: {
				enum { blank, default_, move, copy };

				auto const adjective =
				  *kind == entity::default_constructor                                    ? default_
				  : *kind == entity::move_constructor or *kind == entity::move_assignment ? move
				  : *kind == entity::copy_constructor or *kind == entity::copy_assignment
				    ? copy
				    : blank;

				enum { constructor, destructor, assignment };

				auto const noun =
				  *kind == entity::destructor ? destructor
				  : *kind == entity::move_assignment or *kind == entity::copy_assignment
				    ? assignment
				    : constructor;
				diags_.Report(decl->getLocation(), diag::warn_undocumented_decl)
				  << smf << adjective << parent << noun;
				break;
			}
			default: {
				diags_.Report(decl->getLocation(), diag::warn_undocumented_decl)
				  << member << prefix(*kind) << decl << *kind;
				break;
			}
			}
		}
		else {
			diags_.Report(decl->getLocation(), diag::warn_undocumented_decl) << entity << *kind << decl;
		}

		diags_.Report(decl->getLocation(), diag::note_undocumented_decl) << decl;
	}

	void parser::diagnose_unknown_directive(
	  clang::SourceLocation const directive_location,
	  std::string_view const directive,
	  bool const has_description,
	  clang::PresumedLoc const comment_begin) const
	{
		auto const next_loc = directive_location.getLocWithOffset(
		  static_cast<int>(has_description ? comment_begin.getColumn() - 1 : 0));

		auto doxygen_command = context_.getCommentCommandTraits().getCommandInfoOrNULL(directive);
		if (doxygen_command != nullptr) {
			diagnose_unsupported_doxygen_directive(next_loc, comment_begin, doxygen_command);
		}
		else {
			diags_.Report(next_loc, diag::warn_unknown_directive) << directive;
		}
	}

	static auto is_equal(clang::PresumedLoc const x, clang::PresumedLoc const y) noexcept -> bool
	{
		if (x.isInvalid() or y.isInvalid()) {
			return false;
		}

		return x.getFileID() == y.getFileID() and x.getLine() == y.getLine()
		   and x.getColumn() == y.getColumn();
	}

	void parser::diagnose_unsupported_doxygen_directive(
	  clang::SourceLocation directive_location,
	  clang::PresumedLoc comment_begin,
	  clang::comments::CommandInfo const* doxygen_command) const
	{
		auto const directive = std::string_view(doxygen_command->Name);
		auto equivalent_directive =
		  stdr::find_if(commands, [doxygen_command](command_info const& directive) {
			  return stdr::find(directive.equivalent_doxygen_commands, doxygen_command->Name)
			      != directive.equivalent_doxygen_commands.end();
		  });

		auto diag =
		  diags_.Report(directive_location, diag::warn_unsupported_doxygen_directive)
		  << directive << /*suggest_alternative=*/(equivalent_directive != commands.end())
		  << equivalent_directive->kind;

		// Suggest a fix if the directive location and the presumed comment start location are in fact
		// the same line.
		if (equivalent_directive != commands.end()
		    and is_equal(source_manager_.getPresumedLoc(directive_location), comment_begin))
		{
			auto const directive_range = clang::SourceRange(
			  directive_location.getLocWithOffset(1),
			  directive_location.getLocWithOffset(static_cast<int>(directive.size() + 1)));
			diag << clang::FixItHint::CreateReplacement(directive_range, equivalent_directive->name);
		}
	}
} // namespace parser
