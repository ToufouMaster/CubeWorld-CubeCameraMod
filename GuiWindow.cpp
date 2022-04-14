#include "GuiWindow.h"
#include "CamMod.h"

static const char* AnimationTypeNames[]{ "Linear", "EaseInSine", "EaseOutSine", "EaseInOutSine" };

GuiWindow::GuiWindow(CamMod* mod) {
	this->mod = mod;
}

std::vector<std::string> split(std::string x, char delim = ' ')
{
	x += delim; //includes a delimiter at the end so last word is also read
	std::vector<std::string> splitted;
	std::string temp = "";
	for (int i = 0; i < x.length(); i++)
	{
		if (x[i] == delim)
		{
			splitted.push_back(temp); //store words in "splitted" vector
			temp = "";
			i++;
		}
		temp += x[i];
	}
	return splitted;
}

const wchar_t* GetWC(const char* c)
{
	const size_t cSize = strlen(c) + 1;
	wchar_t* wc = new wchar_t[cSize];
	mbstowcs(wc, c, cSize);

	return wc;
}

// Search of file stolen from ChrisMiunchiz ModLoader
void GuiWindow::LoadFiles()
{
	WIN32_FIND_DATA data;
	HANDLE hFind;
	CreateDirectory("Mods\\CubeCamera\\jsons", NULL);
	hFind = FindFirstFile("Mods\\CubeCamera\\jsons\\*.json", &data);
	std::wstring ws(L"");
	ws += GetWC(data.cFileName);
	while (FindNextFile(hFind, &data)) {
		if (hFind != INVALID_HANDLE_VALUE) {
			ws += (wchar_t)0x00;
			ws += GetWC(data.cFileName);
		}
	}
	loaded_files = std::string(ws.begin(), ws.end());;
	FindClose(hFind);
}

void GuiWindow::Present() {
	if (!initialized) {
		if (!Initialize()) {
			return;
		}
	}

	if (!mod->showwindow) {
		wantMouse = false;
		wantKeyboard = false;
		return;
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

	recording = mod->recording;
	if (ImGui::Checkbox("Recording", &recording)) mod->ToggleRecording();
	ImGui::SameLine(340);
	ImGui::Text("Toggle Window");
	if (ImGui::Checkbox("FreeCam", &freecam)) mod->ToggleFreeCam();
	ImGui::SameLine(340);
	if (ImGui::Button(awaitingKeyRemap ?
		"Waiting..." : (std::string("Remap ") + mod->GetGuiButton()->GetKeyName()).c_str()
		, ImVec2(150, 25))) {
		awaitingKeyRemap = !awaitingKeyRemap;
	}

	if (ImGui::Button("Save", ImVec2(100, 25))) { mod->SaveRecord("RENAME_ME.json"); LoadFiles();}
	/*ImGui::SameLine(0, 10);
	auto savetext = *save_text.c_str();
	if (ImGui::InputText("", &savetext, 20)) {
		//awaitingTextInput = !awaitingTextInput;
	}
	save_text = savetext;*/
	if (ImGui::Button("Load", ImVec2(100, 25))) mod->LoadRecord(split(loaded_files, 0x00)[file_toload]);
	ImGui::SameLine(0, 10);
	ImGui::Combo("", &file_toload, loaded_files.c_str());

	if (ImGui::Button("Add Key", ImVec2(100, 25))) mod->AddKey(selectedKey);
	ImGui::SameLine(0, 10);
	if (ImGui::Button("Del Key", ImVec2(100, 25))) mod->RemoveKey(selectedKey);
	if (ImGui::Button("View", ImVec2(100, 25))) mod->ToggleViewer();
	ImGui::SameLine(0, 10);
	previewer = mod->previewer;
	if (ImGui::Checkbox("Previewer", &previewer)) mod->TogglePreViewer();
	ImGui::SameLine(0, 10);
	ImGui::Checkbox("Repeat", &mod->repeat);

	ImGui::SliderInt("Key List", &selectedKey, 0, max(0, mod->GetKeyPosList().size() - 1));

	if (selectedKey < mod->GetKeyPosList().size()) {
		if (selectedKey < mod->GetKeyPosList().size() - 1) {
			ImGui::SliderFloat("Prerender Time", &previewertime, 0, 1.);
			float time = mod->key_seconds[selectedKey];
			ImGui::SliderFloat("Key Timer", &time, 0, 50., "%.1f sec");
			mod->key_seconds[selectedKey] = time;
			int selected_anim = (int)mod->key_animation_list[selectedKey];
			ImGui::Combo("Animation Type", &selected_anim, AnimationTypeNames, 4);
			mod->key_animation_list[selectedKey] = CamMod::AnimationType(selected_anim);
		}

		ImGui::Separator();

		FloatVector3 angle = FloatVector3((float)mod->GetKeyAngleList()[selectedKey].x, (float)mod->GetKeyAngleList()[selectedKey].y, (float)mod->GetKeyAngleList()[selectedKey].z);
		ImGui::DragFloat("x angle", &angle.x, 1., angle.x - 100, angle.x + 100, "%.0f deg");
		ImGui::DragFloat("y angle", &angle.y, 1., angle.y - 100, angle.y + 100, "%.0f deg");
		ImGui::DragFloat("z angle", &angle.z, 1., angle.z - 100, angle.z + 100, "%.0f deg");
		mod->key_angle_list[selectedKey] = DoubleVector3(angle.x, angle.y, angle.z);

		ImGui::Separator();

		LongVector3 pos = LongVector3(mod->GetKeyPosList()[selectedKey].x / 65536, mod->GetKeyPosList()[selectedKey].y / 65536, mod->GetKeyPosList()[selectedKey].z / 65536);
		LongVector3 posmin = LongVector3(pos.x - 100, pos.y - 100, pos.z - 100);
		LongVector3 posmax = LongVector3(pos.x + 100, pos.y + 100, pos.z + 100);
		ImGui::DragScalar("x position", ImGuiDataType_S64, &pos.x, 1., &posmin.x, &posmax.x);
		ImGui::DragScalar("y position", ImGuiDataType_S64, &pos.y, 1., &posmin.y, &posmax.y);
		ImGui::DragScalar("z position", ImGuiDataType_S64, &pos.z, 1., &posmin.z, &posmax.z);
		mod->key_pos_list[selectedKey].x = pos.x * 65536;
		mod->key_pos_list[selectedKey].y = pos.y * 65536;
		mod->key_pos_list[selectedKey].z = pos.z * 65536;
		mod->previewer_key_id = selectedKey;

		mod->previewertime = previewertime;
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
	LoadFiles();
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

	if (awaitingKeyRemap && keyRemapComplete) {
		awaitingKeyRemap = false;
		keyRemapComplete = false;
		return 1;
	}

	if (awaitingTextInput && textInputComplete) {
		awaitingTextInput = false;
		textInputComplete = false;
		return 1;
	}

	return 0;
}

void GuiWindow::OnGetKeyboardState(BYTE* diKeys) {
	if (wantKeyboard) {
		memset(diKeys, 0, 256);
	}
	if (awaitingKeyRemap) {
		for (int i = 0; i < 256; i++) {
			if (diKeys[i]) {
				keyRemapComplete = true;
				mod->GetGuiButton()->SetKey(i); // Remap
				memset(diKeys, 0, 256);
				mod->SaveKeyBinds();
				break;
			}
		}
	}
	else if (awaitingTextInput) {
		if (diKeys[DIK_RETURN]) {
			textInputComplete = true;
		}
	}
}