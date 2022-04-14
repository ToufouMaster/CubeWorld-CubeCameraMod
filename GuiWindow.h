#ifndef GUIWINDOW_H
#define GUIWINDOW_H

#include "cwmods/cwsdk.h"
#include "imgui-1.73/imgui.h"
#include "imgui-1.73/examples/imgui_impl_dx11.h"
#include "imgui-1.73/examples/imgui_impl_win32.h"
#include <filesystem>

class CamMod;
class GuiWindow {
	bool recording = false;
	bool freecam = false;
	bool previewer = false;
	int selectedKey = 0;
	float previewertime = 0.;
	float keysecond = 0.;
	std::string loaded_files;
	std::string save_text = "";
	int file_toload = 0;

	bool keyRemapComplete = false;
	bool awaitingKeyRemap = false;
	bool textInputComplete = false;
	bool awaitingTextInput = false;
	bool wantMouse = false;
	bool wantKeyboard = false;
	bool initialized = false;
	cube::Game* game;
	CamMod* mod;
public:
	GuiWindow(CamMod* mod);
	void LoadFiles();
	void Present();
	bool Initialize();
	int WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void OnGetKeyboardState(BYTE* diKeys);
};

#endif // GUIWINDOW_H