// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note

/// \returns hello
/// \returns world
/// \returns goodbye
int cube(int x);
// CHECK: input.cc:6:5: error: repeated '\returns' directive for function 'cube'
// CHECK: input.cc:5:5: note: previous definition is here
// CHECK: input.cc:7:5: error: repeated '\returns' directive for function 'cube'
// CHECK: input.cc:5:5: note: previous definition is here
