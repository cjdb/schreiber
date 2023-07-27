// clang-format off
// RUN: %{verify} %s 2>&1 | \
// RUN: FileCheck %s --match-full-lines --implicit-check-not=error --implicit-check-not=warning --implicit-check-not=note --allow-empty

/// These things cannot be stopped: Inherited will. People's dreams. The ebb and flow of the ages.
/// As long as people seek the answer to freedom, these will never cease to be!
template<class T>
concept inherited_will = __is_integral(T);

/// The most notorious four pirate captains in the world.
class yonkō;

/// The Seven Warlords of the Sea, deputised by the World Government to plunder other pirates on
/// their behalf.
struct shichibukai;

/// The members of a pirate crew.
union crew;

/// The rarest of the Devil Fruits, Logias allow those who eat them to transform their bodies into
/// a natural element at will.
template<class T>
class logia;

/// Zoan-type Devil Fruits allow those who eat them to transform their bodies into an animal at will.
template<class T>
union zoan;

/// Any Devil Fruit that is not a Logia or a Zoan.
template<class T>
struct paramecia;

/// The leader of a pirate crew.
int captain(int x)
{
	int y;
	struct leadership {};
	return x;
}

/// A spirit that dwells within a ship that has been looked after by its crew.
using klabautermann = float;

/// The world inside a mirror.
template<class T>
using mirroworld = void*;

/// A sacred box of the Ryugu Kingdom.
typedef short tatame_box;

/// An indestructible cube that contains part of the world's history, written in an extinct language.
auto const ponegliff = 0;

/// A Ponegliff that contains special coordinates.
template<class T>
auto const road_ponegliff = 0;
