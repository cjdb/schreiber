// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note

/// \param num Top
/// \param denominator Bottom
int div(int num, int denom);
// CHECK: input.cc:6:12: error: documented parameter 'denominator' does not map to a parameter in this declaration of 'div'
// CHECK: input.cc:6:12: note: the word immediately after '\param' must name one of the parameters in the function declaration
