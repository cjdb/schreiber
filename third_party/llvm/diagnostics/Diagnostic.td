//===--- Diagnostic.td - C Language Family Diagnostic Handling ------------===//
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
//===----------------------------------------------------------------------===//
//
//  This file defines the TableGen core definitions for the diagnostics
//  and diagnostic control.
//
//===----------------------------------------------------------------------===//

// See the LLVM Internals Manual, section The Diagnostics Subsystem for an overview.

// Define the diagnostic severities.
class Severity<string N> {
  string Name = N;
}
def SEV_Ignored : Severity<"Ignored">;
def SEV_Warning : Severity<"Warning">;
def SEV_Error   : Severity<"Error">;
def SEV_Fatal   : Severity<"Fatal">;

// Define the diagnostic classes.
class DiagClass;
def CLASS_NOTE      : DiagClass;
def CLASS_WARNING   : DiagClass;
def CLASS_ERROR     : DiagClass;

// Responses to a diagnostic in a SFINAE context.
class SFINAEResponse;
def SFINAE_SubstitutionFailure : SFINAEResponse;
def SFINAE_Suppress            : SFINAEResponse;
def SFINAE_Report              : SFINAEResponse;
def SFINAE_AccessControl       : SFINAEResponse;

// Textual substitutions which may be performed on the text of diagnostics
class TextSubstitution<string Text> {
  string Substitution = Text;
  // TODO: These are only here to allow substitutions to be declared inline with
  // diagnostics
  string Component = "";
  string CategoryName = "";
  bit Deferrable = 0;
}

// DiagnosticGroups
class DiagGroup<string Name, list<DiagGroup> subgroups = []> {
  string GroupName = Name;
  list<DiagGroup> SubGroups = subgroups;
  string CategoryName = "";
}
class InGroup<DiagGroup G> { DiagGroup Group = G; }

// All diagnostics emitted by the compiler are an indirect subclass of this.
class Diagnostic<string summary, DiagClass DC, Severity defaultmapping> {
  /// Component is specified by the file with a big let directive.
  string         Component = ?;
  string         Summary = summary;
  DiagClass      Class = DC;
  SFINAEResponse SFINAE = SFINAE_Suppress;
  bit            AccessControl = 0;
  bit            WarningNoWerror = 0;
  bit            ShowInSystemHeader = 0;
  bit            ShowInSystemMacro = 1;
  bit            Deferrable = 0;
  Severity       DefaultSeverity = defaultmapping;
  DiagGroup      Group;
  string         CategoryName = "";
}

class SFINAEFailure {
  SFINAEResponse SFINAE = SFINAE_SubstitutionFailure;
}
class NoSFINAE {
  SFINAEResponse SFINAE = SFINAE_Report;
}

class Error<string str>     : Diagnostic<str, CLASS_ERROR, SEV_Error>;

// This is used for warnings about questionable code.
class Warning<string str>   : Diagnostic<str, CLASS_WARNING, SEV_Warning>;

// Notes can provide supplementary information on errors, warnings, and remarks.
class Note<string str>      : Diagnostic<str, CLASS_NOTE, SEV_Ignored/*ignored*/>;

class DefaultIgnore { Severity DefaultSeverity = SEV_Ignored; }
class DefaultWarn   { Severity DefaultSeverity = SEV_Warning; }
class DefaultError  { Severity DefaultSeverity = SEV_Error; }
class DefaultFatal  { Severity DefaultSeverity = SEV_Fatal; }

include "DiagnosticLexKinds.td"
