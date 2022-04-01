#include "GuiWindow.h"
#include "CamMod.h"

GuiWindow::GuiWindow(CamMod* mod) {
	this->mod = mod;
}

void GuiWindow::Present() {
	if (!initialized) {
		if (!Initialize()) {
			return;
		}
	}

	ImGui::GetStyle().Colors[ImGuiCol_WindowBg] = ImVec4(0.15f, 0.10f, 0.15f, 0.95f);
	ImGui::GetStyle().Colors[ImGuiCol_PopupBg] = ImVec4(0.15f, 0.10f, 0.15f, 0.95f);
	ImGui::GetStyle().Colors[ImGuiCol_TitleBgActive] = ImVec4(0.50f, 0.30f, 0.50f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_TitleBg] = ImVec4(0.30f, 0.15f, 0.30f, 1.00f);

	ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(0.30f, 0.30f, 0.60f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_FrameBg] = ImVec4(0.30f, 0.30f, 0.60f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_ButtonActive] = ImVec4(0.45f, 0.35f, 0.75f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_ButtonHovered] = ImVec4(0.45f, 0.35f, 0.75f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_FrameBgActive] = ImVec4(0.45f, 0.35f, 0.75f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_FrameBgHovered] = ImVec4(0.45f, 0.35f, 0.75f, 1.00f);
	ImGui::GetStyle().Colors[ImGuiCol_CheckMark] = ImVec4(0.70f, 0.60f, 0.90f, 1.00f);

	ImGui::GetStyle().PopupRounding = 10.0;
	ImGui::GetStyle().WindowRounding = 10.0;
	ImGui::GetStyle().FrameRounding = 4.0;

	ImGui::GetStyle().WindowMenuButtonPosition = ImGuiDir_None;
	ImGui::GetStyle().WindowTitleAlign = ImVec2(0.02, 0.5);

	ImGuiIO& io = ImGui::GetIO();
	io.IniFilename = nullptr;
	wantMouse = io.WantCaptureMouse;
	wantKeyboard = io.WantCaptureKeyboard;
	io.Fonts->AddFontFromFileTTF("resource1.dat", 16.0f);

	ImGui_ImplDX11_NewFrame();
	io.DisplaySize = ImVec2((float)cube::GetGame()->width, (float)cube::GetGame()->height);
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	ImVec2 size(500, 500);
	ImGui::SetNextWindowSize(size);
	ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_Once);
	ImGui::Begin("Cam Mod", nullptr, size, -1.0, ImGuiWindowFlags_NoResize);

	if (ImGui::Checkbox("Recording", &recording)) mod->ToggleRecording();
	if (ImGui::Checkbox("FreeCam", &freecam)) mod->ToggleFreeCam();
	if (ImGui::Button("Add Key", ImVec2(100, 25))) mod->AddKey();
	if (ImGui::Button("View", ImVec2(100, 25))) mod->ToggleViewer();

	ImGui::Separator();

	ImGui::SliderInt("Key List", &selectedKey, 0, max(0, mod->GetKeyPosList().size()-1));
	//ImGui::LabelText("Position:", "Test");
	if (selectedKey < mod->GetKeyAngleList().size()) {
		FloatVector3 angle = FloatVector3((float)mod->GetKeyAngleList()[selectedKey].x * (PI / 180), (float)mod->GetKeyAngleList()[selectedKey].y * (PI / 180), (float)mod->GetKeyAngleList()[selectedKey].z * (PI / 180));
		ImGui::SliderAngle("x:", &angle.x, -3600, 3600);
		ImGui::SliderAngle("y:", &angle.y, -3600, 3600);
		ImGui::SliderAngle("z:", &angle.z, -3600, 3600);
		mod->key_angle_list[selectedKey] = DoubleVector3(angle.x / (PI / 180), angle.y / (PI / 180), angle.z / (PI / 180));
	}

	ImGui::End();
	ImGui::EndFrame();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

	// We just drew over the original cursor, so draw the cursor again on top of the gui
	float guiScale = game->options.guiScale;
	FloatVector2 cursorPosition = game->plasma_engine->mouse_position;
	plasma::Matrix<float>* trans = &game->gui.cursor_node->transformation->matrix;
	plasma::Matrix<float> oldTrans = *trans;
	*trans = trans->scale(guiScale).translate(cursorPosition.x - (cursorPosition.x / guiScale), cursorPosition.y - (cursorPosition.y / guiScale), 0);

	game->gui.cursor_node->Draw(0);

	*trans = oldTrans;
}

bool GuiWindow::Initialize() {
	HWND hWnd = GetActiveWindow();
	if (!hWnd) return false;
	game = cube::GetGame();
	initialized = true;
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(hWnd);
	ImGui_ImplDX11_Init(cube::GetID3D11Device(), cube::GetID3D11DeviceContext());
	return true;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
int GuiWindow::WindowProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
	if (wantMouse) {
		switch (msg) {
		case WM_LBUTTONDOWN:
		case WM_RBUTTONDOWN:
			return 1;
		}
	}
	if (wantKeyboard) return 1;

	return 0;
}
