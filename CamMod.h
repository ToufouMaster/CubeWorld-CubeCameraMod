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

	bool viewer = false;
	int viewer_key_id = 0;
	int step = 0;
	LongVector3 cam_focus_pos;
	LongVector3 next_key_pos;
	LongVector3 vel;
	LongVector3 old_pos;
	DoubleVector3 next_key_angle;
	DoubleVector3 angle_vel;
	DoubleVector3 old_angle;
	cube::Creature::EntityData::Appearance player_appearance;
	cube::DButton guiButton = cube::DButton(1);

public:
	bool showwindow = false;
	bool free_cam = false;
	bool recording = false;
	bool previewer = false;
	int previewer_key_id = 0;
	float previewertime = 0.;
	std::vector<float> key_seconds;
	std::vector<LongVector3> key_pos_list;
	std::vector<DoubleVector3> key_angle_list;

	cube::DButton* GetGuiButton();
	bool AddKey(int id);
	bool RemoveKey(int id);
	bool ToggleRecording();
	bool ToggleViewer();
	bool TogglePreViewer();
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