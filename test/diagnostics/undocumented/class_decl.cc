// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note

class yonkō {
// CHECK: input.cc:[[@LINE-1]]:7: warning: class 'yonkō' is not documented
// CHECK: input.cc:[[@LINE-2]]:7: note: use '\undocumented' to indicate that 'yonkō' is intentionally undocumented
public:
	yonkō(int);
	// CHECK: input.cc:[[@LINE-1]]:2: warning: constructor for 'yonkō' is not documented
	// CHECK: input.cc:[[@LINE-2]]:2: note: use '\undocumented' to indicate that 'yonkō' is intentionally undocumented

	yonkō();
	// CHECK: input.cc:[[@LINE-1]]:2: warning: default constructor for 'yonkō' is not documented
	// CHECK: input.cc:[[@LINE-2]]:2: note: use '\undocumented' to indicate that 'yonkō' is intentionally undocumented

	yonkō(yonkō&&);
	// CHECK: input.cc:[[@LINE-1]]:2: warning: move constructor for 'yonkō' is not documented
	// CHECK: input.cc:[[@LINE-2]]:2: note: use '\undocumented' to indicate that 'yonkō' is intentionally undocumented

	yonkō(yonkō const&);
	// CHECK: input.cc:[[@LINE-1]]:2: warning: copy constructor for 'yonkō' is not documented
	// CHECK: input.cc:[[@LINE-2]]:2: note: use '\undocumented' to indicate that 'yonkō' is intentionally undocumented

	yonkō& operator=(int);
	// CHECK: input.cc:[[@LINE-1]]:10: warning: member function 'operator=' is not documented
	// CHECK: input.cc:[[@LINE-2]]:10: note: use '\undocumented' to indicate that 'operator=' is intentionally undocumented

	yonkō& operator=(yonkō&&);
	// CHECK: input.cc:[[@LINE-1]]:10: warning: move assignment operator for 'yonkō' is not documented
	// CHECK: input.cc:[[@LINE-2]]:10: note: use '\undocumented' to indicate that 'operator=' is intentionally undocumented

	yonkō& operator=(yonkō const&);
	// CHECK: input.cc:[[@LINE-1]]:10: warning: copy assignment operator for 'yonkō' is not documented
	// CHECK: input.cc:[[@LINE-2]]:10: note: use '\undocumented' to indicate that 'operator=' is intentionally undocumented

	~yonkō();
	// CHECK: input.cc:[[@LINE-1]]:2: warning: destructor for 'yonkō' is not documented
	// CHECK: input.cc:[[@LINE-2]]:2: note: use '\undocumented' to indicate that '~yonkō' is intentionally undocumented

	using captain = int;
	// CHECK: input.cc:[[@LINE-1]]:8: warning: member type alias 'captain' is not documented
	// CHECK: input.cc:[[@LINE-2]]:8: note: use '\undocumented' to indicate that 'captain' is intentionally undocumented

	static void operator()();
	// CHECK: input.cc:[[@LINE-1]]:14: warning: member function 'operator()' is not documented
	// CHECK: input.cc:[[@LINE-2]]:14: note: use '\undocumented' to indicate that 'operator()' is intentionally undocumented

	inline static int x = 0;
	// CHECK: input.cc:[[@LINE-1]]:20: warning: data member 'x' is not documented
	// CHECK: input.cc:[[@LINE-2]]:20: note: use '\undocumented' to indicate that 'x' is intentionally undocumented

	friend union crew;

	template<class T>
	friend class logia;

	friend int total_commanders(yonkō);

	template<class T>
	friend int total_ships(yonkō, T);

	friend int remaining_commanders(yonkō)
	// CHECK: input.cc:[[@LINE-1]]:13: warning: function 'remaining_commanders' is not documented
	// CHECK: input.cc:[[@LINE-2]]:13: note: use '\undocumented' to indicate that 'remaining_commanders' is intentionally undocumented
	{
		return 0;
	}

	template<class T>
	friend int remaining_ships(yonkō, T)
	// CHECK: input.cc:[[@LINE-1]]:13: warning: function template 'remaining_ships' is not documented
	// CHECK: input.cc:[[@LINE-2]]:13: note: use '\undocumented' to indicate that 'remaining_ships' is intentionally undocumented
	{
		return 0;
	}
private:
	int berries();
};
