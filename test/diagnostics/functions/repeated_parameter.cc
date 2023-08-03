// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note

/// \param y is the rhs
/// \param x is the lhs
/// \param x pretty cool
/// \param y also cool
int multiply(int x, int y);
// CHECK: input.cc:7:5: error: repeated '\param' directive for parameter 'x' in function 'multiply'
// CHECK: input.cc:6:5: note: previous definition is here
// CHECK: input.cc:8:5: error: repeated '\param' directive for parameter 'y' in function 'multiply'
// CHECK: input.cc:5:5: note: previous definition is here
