// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=warning --implicit-check-not=note

class yonkō;
// CHECK: input.cc:[[@LINE-1]]:7: warning: class 'yonkō' is not documented
// CHECK: input.cc:[[@LINE-2]]:7: note: use '\undocumented' to indicate that 'yonkō' is intentionally undocumented

class yonkō;
