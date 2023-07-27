// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note

namespace new_world {
template<class T>
concept inherited_will = __is_integral(T);
// CHECK: input.cc:[[@LINE-1]]:9: warning: concept 'inherited_will' is not documented
// CHECK: input.cc:[[@LINE-2]]:9: note: use '\undocumented' to indicate that 'inherited_will' is intentionally undocumented

class yonkō;
// CHECK: input.cc:[[@LINE-1]]:7: warning: class 'yonkō' is not documented
// CHECK: input.cc:[[@LINE-2]]:7: note: use '\undocumented' to indicate that 'yonkō' is intentionally undocumented

struct shichibukai;
// CHECK: input.cc:[[@LINE-1]]:8: warning: struct 'shichibukai' is not documented
// CHECK: input.cc:[[@LINE-2]]:8: note: use '\undocumented' to indicate that 'shichibukai' is intentionally undocumented

union crew;
// CHECK: input.cc:[[@LINE-1]]:7: warning: union 'crew' is not documented
// CHECK: input.cc:[[@LINE-2]]:7: note: use '\undocumented' to indicate that 'crew' is intentionally undocumented

template<class T>
class logia;
// CHECK: input.cc:[[@LINE-1]]:7: warning: class template 'logia' is not documented
// CHECK: input.cc:[[@LINE-2]]:7: note: use '\undocumented' to indicate that 'logia' is intentionally undocumented

template<class T>
struct paramecia;
// CHECK: input.cc:[[@LINE-1]]:8: warning: class template 'paramecia' is not documented
// CHECK: input.cc:[[@LINE-2]]:8: note: use '\undocumented' to indicate that 'paramecia' is intentionally undocumented

template<class T>
union zoan;
// CHECK: input.cc:[[@LINE-1]]:7: warning: class template 'zoan' is not documented
// CHECK: input.cc:[[@LINE-2]]:7: note: use '\undocumented' to indicate that 'zoan' is intentionally undocumented

int captain(int x)
// CHECK: input.cc:[[@LINE-1]]:5: warning: function 'captain' is not documented
// CHECK: input.cc:[[@LINE-2]]:5: note: use '\undocumented' to indicate that 'captain' is intentionally undocumented
{
	int y;
	struct leadership {};
	return x;
}

using klabautermann = float;
// CHECK: input.cc:[[@LINE-1]]:7: warning: type alias 'klabautermann' is not documented
// CHECK: input.cc:[[@LINE-2]]:7: note: use '\undocumented' to indicate that 'klabautermann' is intentionally undocumented

template<class T>
using mirroworld = void*;
// CHECK: input.cc:[[@LINE-1]]:1: warning: type alias template 'mirroworld' is not documented
// CHECK: input.cc:[[@LINE-2]]:1: note: use '\undocumented' to indicate that 'mirroworld' is intentionally undocumented

typedef short tatame_box;
// CHECK: input.cc:[[@LINE-1]]:15: warning: typedef 'tatame_box' is not documented
// CHECK: input.cc:[[@LINE-2]]:15: note: use '\undocumented' to indicate that 'tatame_box' is intentionally undocumented

auto const ponegliff = 0;
// CHECK: input.cc:[[@LINE-1]]:12: warning: variable 'ponegliff' is not documented
// CHECK: input.cc:[[@LINE-2]]:12: note: use '\undocumented' to indicate that 'ponegliff' is intentionally undocumented

template<class T>
auto const road_ponegliff = 0;
// CHECK: input.cc:[[@LINE-1]]:12: warning: variable template 'road_ponegliff' is not documented
// CHECK: input.cc:[[@LINE-2]]:12: note: use '\undocumented' to indicate that 'road_ponegliff' is intentionally undocumented
}
