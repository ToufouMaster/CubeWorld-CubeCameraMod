#ifndef GUIWINDOW_H
#define GUIWINDOW_H

#include "cwmods/cwsdk.h"
#include "imgui-1.73/imgui.h"
#include "imgui-1.73/examples/imgui_impl_dx11.h"
#include "imgui-1.73/examples/imgui_impl_win32.h"

class CamMod;
class GuiWindow {
	bool recording = false;
	bool freecam = false;
	int selectedKey = 0;

	bool wantMouse = false;
	bool wantKeyboard = false;
	bool initialized = false;
	cube::Game* game;
	CamMod* mod;
public:
	GuiWindow(CamMod* mod);
	void Present();
	bool Initialize();
	int WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
};

#endif // GUIWINDOW_H