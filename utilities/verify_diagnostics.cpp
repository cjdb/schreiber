// Copyright (c) Google LLC.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
#include <algorithm>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclTemplate.h>
#include <clang/ASTMatchers/ASTMatchFinder.h>
#include <clang/ASTMatchers/ASTMatchers.h>
#include <clang/Frontend/ASTUnit.h>
#include <clang/Frontend/TextDiagnosticPrinter.h>
#include <clang/Frontend/VerifyDiagnosticConsumer.h>
#include <clang/Tooling/Tooling.h>
#include <fstream>
#include <iterator>
#include <llvm/Support/raw_ostream.h>
#include <schreiber/diagnostic_ids.hpp>
#include <schreiber/parser.hpp>

namespace {
	namespace ast_matchers = clang::ast_matchers;
	namespace tooling = clang::tooling;

	using ast_matchers::allOf;
	using ast_matchers::anyOf;
	using ast_matchers::decl;
	using ast_matchers::friendDecl;
	using ast_matchers::hasParent;
	using ast_matchers::isImplicit;
	using ast_matchers::isPrivate;
	using ast_matchers::match;
	using ast_matchers::namedDecl;
	using ast_matchers::namespaceDecl;
	using ast_matchers::recordDecl;
	using ast_matchers::translationUnitDecl;
	using ast_matchers::unless;
} // namespace

/// A simple program to check diagnostics while there isn't a proper schreiber binary.
int main(int argc, char* argv[])
{
	if (argc != 2) {
		llvm::errs() << "usage: ./verify-diagnostics /path/to/file.cpp\n";
		return 1;
	}

	auto file = std::ifstream(argv[1]);
	if (not file) {
		llvm::errs() << "unable to open file '" << argv[1] << "'\n";
		return 1;
	}

	auto code = std::string();
	std::ranges::copy(std::istreambuf_iterator<char>(file), std::default_sentinel, std::back_inserter(code));

	file.peek();
	if (not file.eof()) {
		llvm::errs() << "unable to read file '" << argv[1] << "'\n";
		return 1;
	}

	auto const ast = tooling::buildASTFromCodeWithArgs(code, {"-std=c++23"});
	if (ast == nullptr) {
		llvm::errs() << "couldn't acquire an AST\n";
		return 1;
	}

	auto& diags = ast->getDiagnostics();
	if (diags.getNumErrors() > 0) {
		return 1;
	}

	auto decls = match(
	  decl(
	    anyOf(
	      namedDecl(allOf(
	        anyOf(hasParent(translationUnitDecl()), hasParent(namespaceDecl()), hasParent(recordDecl())),
	        unless(anyOf(isImplicit(), isPrivate())))),
	      friendDecl()))
	    .bind("root"),
	  ast->getASTContext());

	diag::add_diagnostics(diags);
	diags.getClient()->BeginSourceFile(ast->getLangOpts());
	{
		auto p = parser::parser(ast->getASTContext());
		for (auto const& i : decls) {
			if (auto named_decl = i.getNodeAs<clang::NamedDecl>("root")) {
				(void)p.parse(named_decl);
			}
			else if (auto const friend_decl = i.getNodeAs<clang::FriendDecl>("root")) {
				if ((named_decl = friend_decl->getFriendDecl())) {
					if (named_decl->hasBody()) {
						(void)p.parse(named_decl);
					}
					else if (auto const function_template = llvm::dyn_cast<clang::FunctionTemplateDecl>(named_decl);
					         function_template and function_template->getAsFunction()->hasBody())
					{
						(void)p.parse(named_decl);
					}
				}
			}
		}
	}
	diags.getClient()->EndSourceFile();
}
