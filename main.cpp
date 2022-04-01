#include "CamMod.h"

// Export of the mod created in this file, so that the modloader can see and use it.
EXPORT CamMod* MakeMod() {
	return new CamMod();
}