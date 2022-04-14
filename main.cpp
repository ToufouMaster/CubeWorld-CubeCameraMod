#include "CamMod.h"

// A lot of code (like imgui code) come from ChrisMiuchiz BuildMod. Here the github link:
// https://github.com/ChrisMiuchiz/Cube-World-Building-Mod

// TODO:
// Alpha2:
// V Set speed based on Time and not frame
// V Add Lerp and some animation control options
// V Add Repeat Button 
// - Add Loading and Saving
//	V Keybinds
//	- Key Data
// - Add multiple presets 

EXPORT CamMod* MakeMod() {
	return new CamMod();
}