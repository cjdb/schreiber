// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <absl/strings/ascii.h>
#include <absl/strings/str_cat.h>
#include <algorithm>
#include <cctype>
#include <clang/AST/ASTContext.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclCXX.h>
#include <clang/AST/DeclFriend.h>
#include <clang/Basic/Diagnostic.h>
#include <clang/Basic/SourceLocation.h>
#include <clang/Basic/SourceManager.h>
#include <expected>
#include <llvm/ADT/StringExtras.h>
#include <numeric>
#include <optional>
#include <ranges>
#include <schreiber/diagnostic_ids.hpp>
#include <schreiber/info.hpp>
#include <schreiber/parser.hpp>
#include <set>

namespace stdr = std::ranges;
namespace stdv = std::views;

namespace parser {
	namespace {
		struct compare_locations {
			[[nodiscard]] bool
			operator()(clang::Decl const* const x, clang::Decl const* const y) const noexcept
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

		struct lexed_result_t {
			directive directive;
			description description;
		};
	} // namespace

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

	[[nodiscard]]
	static auto scan(
	  parser& p,
	  std::string_view const text,
	  line_iterator const first,
	  line_iterator const last,
	  clang::SourceLocation const begin_loc,
	  bool const has_description) -> std::expected<lexed_result_t, next_directive>
	{
		auto const directive = directive::extract(text, begin_loc);
		auto description =
		  description::extract(first, last, std::string_view(directive.text.end(), text.end()), begin_loc);

		if (directive.token != nullptr) {
			return lexed_result_t{
			  .directive = directive,
			  .description = std::move(description),
			};
		}

		p.diagnose_unknown_directive(begin_loc, directive.text, has_description, first->Begin);
		return std::unexpected(description.next);
	}

	// Computes the offset of the first directive. This is required since CommentLine operates on a
	// presumed location rather than an actual source location, but diagnostics require the latter.
	[[nodiscard]] static auto initial_directive_offset(
	  clang::SourceManager const& source_manager,
	  clang::SourceLocation const begin_location,
	  clang::RawComment::CommentLine const& first_line,
	  std::span<clang::RawComment::CommentLine const> const description,
	  std::string_view const text_description) -> clang::SourceLocation
	{
		auto const initial_comment_offset =
		  first_line.Begin.getColumn() - source_manager.getPresumedColumnNumber(begin_location);
		auto const offset_by = std::accumulate(
		  // Skip the first one so we don't double-count the first line (which is obtained via
		  /// `getBeginLoc()`).
		  stdr::next(description.begin(), 1, description.end()),
		  description.end(),
		  static_cast<int>(text_description.size() + initial_comment_offset),
		  [](int const x, clang::RawComment::CommentLine const& comment) {
			  return x + static_cast<int>(comment.Begin.getColumn());
		  });
		return begin_location.getLocWithOffset(offset_by);
	}

	void parser::parse_directives(
	  info::entity_info& entity,
	  line_iterator first,
	  line_iterator last,
	  clang::SourceLocation begin_loc)
	{
		auto const decl = entity.decl();
		auto const has_description = not entity.description().empty();
		auto parse_directive = [this, &decl](lexed_result_t const& lexed_result) {
			if (auto const function = llvm::dyn_cast<clang::FunctionDecl>(decl)) {
				return visit(function, lexed_result.directive, lexed_result.description);
			}

			std::unreachable();
		};

		auto store_directive = [this, &entity, &decl](parse_result_t parsed_result) {
			switch (decl->getKind()) {
			case clang::Decl::Function:
				entity.store(*this, parsed_result.current, parsed_result.info.get());
				break;
			default:
				assert(false and "not handled");
			}

			return parsed_result.next;
		};

		while (first != last) {
			auto text = std::string_view(first->Text);
			assert(text.starts_with('\\'));

			auto next_line =
			  scan(*this, text, first, last, begin_loc, has_description)
			    .and_then(parse_directive)
			    .transform(store_directive);

			if (not next_line) {
				auto const& next = next_line.error();
				begin_loc = next.location;
				first = next.line;
				continue;
			}

			begin_loc = next_line->location;
			first = next_line->line;
		}
	}

	[[nodiscard]]
	static auto
	make_entity_info(clang::NamedDecl const* const decl, std::string description, clang::SourceLocation location)
	  -> std::unique_ptr<info::entity_info>
	{
		auto const i = decl->getKind();
		switch (i) {
		case clang::Decl::Function:
			return std::make_unique<info::function_info>(decl->getAsFunction(), std::move(description), location);
		default:
			assert(false and "not handled");
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

		undocumented_declarations.erase(decl->getCanonicalDecl());
		documented_declarations.insert(decl->getCanonicalDecl());

		auto const lines = raw_comment->getFormattedLines(source_manager_, diags_);

		auto const description = std::span(
		  lines.begin(),
		  stdr::find_if(lines, starts_with_backslash, &clang::RawComment::CommentLine::Text));
		auto text_description = llvm::join(description | stdv::transform(to_text), "\n");
		auto const directive_location = initial_directive_offset(
		  source_manager_,
		  raw_comment->getBeginLoc(),
		  lines[0],
		  description,
		  text_description);

		auto result = make_entity_info(decl, std::move(text_description), raw_comment->getBeginLoc());
		parse_directives(*result, description.end(), lines.end(), directive_location);
		return result;
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
				diagnose(decl->getLocation(), diag::warn_undocumented_decl)
				  << smf << adjective << parent << noun;
				break;
			}
			default: {
				diagnose(decl->getLocation(), diag::warn_undocumented_decl)
				  << member << prefix(*kind) << decl << *kind;
				break;
			}
			}
		}
		else {
			diagnose(decl->getLocation(), diag::warn_undocumented_decl) << entity << *kind << decl;
		}

		diagnose(decl->getLocation(), diag::note_undocumented_decl) << decl;
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
		else if (not directive.empty()) {
			diagnose(next_loc, diag::warn_unknown_directive) << directive;
		}
		else {
			diagnose(next_loc, diag::err_lone_backslash);
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
		  diagnose(directive_location, diag::warn_unsupported_doxygen_directive)
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

	auto
	parser::diagnose(clang::SourceLocation loc, unsigned int diag_id) const -> clang::DiagnosticBuilder
	{
		return diags_.Report(loc, diag_id);
	}

	auto to_text(clang::RawComment::CommentLine const& line) noexcept -> std::string_view
	{
		return line.Text;
	}

	auto starts_with_backslash(std::string_view const c) noexcept -> bool
	{
		return c.starts_with('\\');
	}

	auto is_space(char const c) noexcept -> bool
	{
		return static_cast<bool>(std::isspace(c));
	}

	auto
	directive::extract(std::string_view const text, clang::SourceLocation const begin_loc) -> directive
	{
		auto raw_directive =
		  std::string_view(text.begin() + 1, text.end())
		  | stdv::take_while([](char const c) { return not std::isspace(c); });

		auto const raw_end = stdr::next(raw_directive.begin(), raw_directive.end());
		auto const result_text = std::string_view(raw_directive.begin(), raw_end);
		return directive{
		  .token = lex(result_text),
		  .text = result_text,
		  .location = begin_loc,
    };
	}

	auto description::extract(
	  line_iterator first,
	  line_iterator last,
	  std::string_view const text,
	  clang::SourceLocation const begin_loc) -> description
	{
		auto const next_directive =
		  stdr::find_if(first + 1, last, starts_with_backslash, &clang::RawComment::CommentLine::Text);
		auto const end_loc = std::accumulate(
		  first,
		  next_directive,
		  begin_loc,
		  [](clang::SourceLocation const& x, clang::RawComment::CommentLine const& y) {
			  return x.getLocWithOffset(static_cast<int>(y.Text.size() + y.Begin.getColumn()));
		  });
		auto result = description{
		  .text = absl::StrCat(
		    text,
		    "\n",
		    llvm::join(llvm::iterator_range(first + 1, next_directive) | stdv::transform(to_text), "\n")),
		  .location = begin_loc,
		  .next = {next_directive, end_loc},
		};
		absl::StripAsciiWhitespace(&result.text);
		return result;
	}

	template<class T>
	static auto parse_exported_by(std::string_view const description) -> std::vector<T>
	{
		return stdv::split(description, ',') //
		     | stdv::transform([](auto const h) {
			       auto exporter = std::string_view(h.begin(), h.end());
			       return T(std::string(absl::StripAsciiWhitespace(exporter)), {});
		       })
		     | stdr::to<std::vector>();
	}

	auto parse_module_info(std::string_view const text) -> std::vector<info::decl_info::module_info>
	{
		return parse_exported_by<info::decl_info::module_info>(text);
	}

	auto parse_header_info(std::string_view const text) -> std::vector<info::decl_info::header_info>
	{
		return parse_exported_by<info::decl_info::header_info>(text);
	}
} // namespace parser
