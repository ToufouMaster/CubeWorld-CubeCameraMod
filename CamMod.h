#ifndef CAMMOD_H
#define CAMMOD_H
#define PI 3.14159265

#include "cwmods/cwsdk.h"
#include "GuiWindow.h"
#include "imgui-1.73/imgui.h"
#include "imgui-1.73/examples/imgui_impl_dx11.h"
#include "imgui-1.73/examples/imgui_impl_win32.h"
#include <cmath>

class CamMod : GenericMod {
	GuiWindow* guiWindow;

	bool free_cam = false;
	bool recording = false;
	bool viewer = false;
	int viewer_key_id = 0;
	int steps_per_key = 60 * 5;
	int step = 0;
	LongVector3 cam_focus_pos;
	LongVector3 next_key_pos;
	LongVector3 vel;
	LongVector3 old_pos;
	DoubleVector3 next_key_angle;
	DoubleVector3 angle_vel;
	DoubleVector3 old_angle;
	cube::Creature::EntityData::Appearance player_appearance;

public:
	std::vector<LongVector3> key_pos_list;
	std::vector<DoubleVector3> key_angle_list;

	bool AddKey();
	bool ToggleRecording();
	bool ToggleViewer();
	bool ToggleFreeCam();

	std::vector<LongVector3> GetKeyPosList();
	std::vector<DoubleVector3> GetKeyAngleList();

	virtual int OnChat(std::wstring* message) override;

	virtual void OnGameTick(cube::Game* game) override;

	virtual void OnPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) override;

	virtual void Initialize() override;

	virtual int OnWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;

	virtual void OnGetKeyboardState(BYTE* diKeys) override;

};

#endif