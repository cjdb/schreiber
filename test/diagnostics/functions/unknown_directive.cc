// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note

/// None of these directives exist, but some can be automatically fixed.
///
/// \extends
/// \return
/// \retval
/// \result
/// \throw
/// \exception
/// \ a
/// \buggy d. clown
void typo();

// CHECK: input.cc:7:5: warning: '\extends' is an unsupported Doxygen command and will be ignored
// CHECK: input.cc:8:5: warning: '\return' is an unsupported Doxygen command and will be ignored; use '\returns' instead
// CHECK: input.cc:9:5: warning: '\retval' is an unsupported Doxygen command and will be ignored; use '\returns' instead
// CHECK: input.cc:10:5: warning: '\result' is an unsupported Doxygen command and will be ignored; use '\returns' instead
// CHECK: input.cc:11:5: warning: '\throw' is an unsupported Doxygen command and will be ignored; use '\throws' instead
// CHECK: input.cc:12:5: warning: '\exception' is an unsupported Doxygen command and will be ignored; use '\throws' instead
// CHECK: input.cc:13:5: error: a backslash must be followed by a non-space character
// CHECK: input.cc:14:5: warning: unknown directive '\buggy'
