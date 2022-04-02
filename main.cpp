#include "CamMod.h"

// A lot of code (like imgui code) come from ChrisMiuchiz BuildMod. Here the github link:
// https://github.com/ChrisMiuchiz/Cube-World-Building-Mod

EXPORT CamMod* MakeMod() {
	return new CamMod();
}