// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note --allow-empty

/// The leader of a pirate crew.
int captain(int x)
{
	int y;
	struct leadership {};
	return x;
}
