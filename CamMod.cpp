#include "CamMod.h"

#define CAMERA_SPEED 16384
// Public variables.
// For example: int number = 10;


// Includes for the self written hooks.
// For example: #include "src/hooks/a_hook.h" 

/* Mod class containing all the functions for the mod.
*/

long double curtime() {
	long double time = (long double)std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::system_clock::now().time_since_epoch()
		).count();
	return time / 1000;
}

cube::DButton* CamMod::GetGuiButton()
{
	return &guiButton;
}

bool CamMod::AddKey(int id) {
	cube::Game* game = cube::GetGame();
	cube::Creature* player = game->GetPlayer();
	if (recording) {
		if (id >= 0) {
			if (key_pos_list.size() != 0) id += 1;
			wchar_t buffer[250];
			swprintf_s(buffer, 250, L"Key succesfully added to id %d!\n", id);
			game->PrintMessage(buffer, 100, 255, 100);
			key_pos_list.insert(key_pos_list.begin() + id, player->entity_data.position);
			key_angle_list.insert(key_angle_list.begin() + id, game->camera_angle);
			key_seconds.insert(key_seconds.begin() + id, 5.);
			key_animation_list.insert(key_animation_list.begin() + id, AnimationType::Linear);
			return true;
		}
	}
	else {
		game->PrintMessage(L"Recording is not activated!\n", 255, 100, 100);
		return false;
	}
}

bool CamMod::RemoveKey(int id) {
	LoadRecord("test.json");
	cube::Game* game = cube::GetGame();
	if (recording) {
		if (id < key_pos_list.size()) {
			key_pos_list.erase(key_pos_list.begin() + id);
			key_angle_list.erase(key_angle_list.begin() + id);
			key_seconds.erase(key_seconds.begin() + id);
			key_animation_list.erase(key_animation_list.begin() + id);
			return true;
		}
		else {
			game->PrintMessage(L"No keys set yet!\n", 255, 100, 100);
		}
	}
	else {
		game->PrintMessage(L"Recording is not activated!\n", 255, 100, 100);
	}
	return false;
}

bool CamMod::ToggleRecording()
{
	cube::Game* game = cube::GetGame();
	if (viewer) {
		game->PrintMessage(L"Error: Viewer still activated!\n", 255, 100, 100);
		return false;
	}
	if (not recording) previewer = true;
	recording = !recording;
	return true;
}

bool CamMod::ToggleViewer()
{
	cube::Game* game = cube::GetGame();
	cube::Creature* player = game->GetPlayer();
	if (not viewer) {
		if (not recording) {
			if (key_pos_list.size() > 1) {
				previewer = false;
				viewer = true;
				player->entity_data.position = key_pos_list[0];
				game->camera_angle = key_angle_list[0];
				old_time = curtime();
				old_pos = key_pos_list[0];
				old_angle = key_angle_list[0];
				viewer_key_id = 1;
				CalculateNewKey();
				return true;
			}
			else {
				game->PrintMessage(L"No keys set yet!\n", 255, 100, 100);
			}
		}
		return false;
	}
	game->target_camera_distance = 6;
	viewer = false;
	return true;
}

bool CamMod::TogglePreViewer()
{
	cube::Game* game = cube::GetGame();
	if (previewer) { previewer = false; game->camera_distance = 6; return true; }
	if (viewer || not recording) return false;
	previewer = true;
	return true;
}

bool CamMod::ToggleFreeCam() {
	cube::Game* game = cube::GetGame();
	cube::Creature* player = game->GetPlayer();
	if (not free_cam) {
		player_appearance = player->entity_data.appearance;
	}
	else {
		player->entity_data.appearance = player_appearance;
	}
	free_cam = not free_cam;
	return true;
}

std::vector<LongVector3> CamMod::GetKeyPosList()
{
	return key_pos_list;
}

std::vector<DoubleVector3> CamMod::GetKeyAngleList()
{
	return key_angle_list;
}

/* Hook for the chat function. Triggers when a user sends something in the chat.
 * @param	{std::wstring*} message
 * @return	{int}
*/
int CamMod::OnChat(std::wstring* message) {
	return 0;
}

/* Function hook that gets called every game tick.
 * @param	{cube::Game*} game
 * @return	{void}
*/
void CamMod::OnGameTick(cube::Game* game) {
	cube::Creature* player = game->GetPlayer();
	if (viewer) {
		game->target_camera_distance = 0;
		float actual_time = curtime() - old_time;
		float progress = (key_seconds[viewer_key_id - 1] - actual_time) / key_seconds[viewer_key_id - 1];
		float animated_time = ApplyAnimation(key_animation_list[viewer_key_id - 1], 1 - progress);
		LongVector3 vel(
			(next_key_pos.x - old_pos.x) * animated_time,
			(next_key_pos.y - old_pos.y) * animated_time,
			(next_key_pos.z - old_pos.z) * animated_time
		);
		DoubleVector3 angle_vel(
			(next_key_angle.x - old_angle.x) * animated_time,
			(next_key_angle.y - old_angle.y) * animated_time,
			(next_key_angle.z - old_angle.z) * animated_time
		);
		player->entity_data.position.x = old_pos.x + vel.x;
		player->entity_data.position.y = old_pos.y + vel.y;
		player->entity_data.position.z = old_pos.z + vel.z;
		game->target_camera_angle.x = old_angle.x + angle_vel.x;
		game->target_camera_angle.y = old_angle.y + angle_vel.y;
		game->target_camera_angle.z = old_angle.z + angle_vel.z;

		if (actual_time >= key_seconds[viewer_key_id - 1]) {
			if (viewer_key_id + 1 >= key_pos_list.size()) {
				viewer_key_id = 0;
				if (not repeat) {
					viewer = false;
					player->entity_data.position = key_pos_list[key_pos_list.size() - 1];
					game->target_camera_angle = key_angle_list[key_angle_list.size() - 1];
					return;
				}
				player->entity_data.position = key_pos_list[0];
				game->target_camera_angle = key_angle_list[0];
			}
			GetOldKey();
			viewer_key_id += 1;
			CalculateNewKey();
		}
	}
	if (previewer) {
		game->camera_distance = 0;
		if (previewer_key_id < key_pos_list.size()) {
			if (previewertime > 0 and previewer_key_id < key_pos_list.size() - 1) {
				LongVector3 old_pos = key_pos_list[previewer_key_id];
				DoubleVector3 old_angle = key_angle_list[previewer_key_id];
				LongVector3 new_pos = key_pos_list[previewer_key_id + 1];
				DoubleVector3 new_angle = key_angle_list[previewer_key_id + 1];
				float animated_time = ApplyAnimation(CamMod::AnimationType::EaseInSine, previewertime);
				LongVector3 previewposvec = LongVector3((new_pos.x - old_pos.x) * animated_time, (new_pos.y - old_pos.y) * animated_time, (new_pos.z - old_pos.z) * animated_time);
				DoubleVector3 previewanglevec = DoubleVector3((new_angle.x - old_angle.x) * animated_time, (new_angle.y - old_angle.y) * animated_time, (new_angle.z - old_angle.z) * animated_time);
				player->entity_data.position.x = old_pos.x + previewposvec.x;
				player->entity_data.position.y = old_pos.y + previewposvec.y;
				player->entity_data.position.z = old_pos.z + previewposvec.z;
				game->target_camera_angle.x = old_angle.x + previewanglevec.x;
				game->target_camera_angle.y = old_angle.y + previewanglevec.y;
				game->target_camera_angle.z = old_angle.z + previewanglevec.z;
			}
			else {
				player->entity_data.position = key_pos_list[previewer_key_id];
				game->target_camera_angle = key_angle_list[previewer_key_id];
			}
		}
	}
	if (player && free_cam) {
		player->entity_data.velocity = FloatVector3(0, 0, 0);
		player->entity_data.acceleration = FloatVector3(0, 0, 0);
		player->entity_data.appearance.chest_model = -1;
		player->entity_data.appearance.feet_model = -1;
		player->entity_data.appearance.hair_model = -1;
		player->entity_data.appearance.hands_model = -1;
		player->entity_data.appearance.head_model = -1;
		player->entity_data.appearance.shoulder_model = -1;
		player->entity_data.appearance.tail_model = -1;
		player->entity_data.appearance.wings_model = -1;
		return;
	}
	return;
}

void CamMod::GetOldKey() {
	cube::Game* game = cube::GetGame();
	cube::Creature* player = game->GetPlayer();
	old_time = curtime();
	old_pos = key_pos_list[viewer_key_id];
	old_angle = key_angle_list[viewer_key_id];
}

void CamMod::CalculateNewKey() {
	cube::Game* game = cube::GetGame();
	cube::Creature* player = game->GetPlayer();
	next_time = game->world->GetTime();
	next_key_pos = key_pos_list[viewer_key_id];
	next_key_angle = key_angle_list[viewer_key_id];
}

float CamMod::ApplyAnimation(AnimationType type, float x) {
	if (type == AnimationType::Linear) return x;
	if (type == AnimationType::EaseInSine) return easeInSine(x);
	if (type == AnimationType::EaseOutSine) return easeOutSine(x);
	if (type == AnimationType::EaseInOutSine) return easeInOutSine(x);
}

void CamMod::OnPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
	guiWindow->Present();
}

/* Function hook that gets called on intialization of cubeworld.
 * [Note]:	cube::GetGame() is not yet available here!!
 * @return	{void}
*/

void CamMod::Initialize() {
	LoadKeyBinds();
	guiWindow = new GuiWindow(this);
	return;
}

int CamMod::OnWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	return guiWindow->WindowProc(hwnd, uMsg, wParam, lParam);
}

void CamMod::OnGetKeyboardState(BYTE* diKeys) {
	guiWindow->OnGetKeyboardState(diKeys);
	guiButton.Update(diKeys);
	if (guiButton.Pressed() == cube::DButton::State::Pressed) {
		showwindow = !showwindow;
	}

	if (free_cam) {
		static cube::DButton KeySpace = cube::DButton(57); // Space
		static cube::DButton KeyW = cube::DButton(17); //W
		static cube::DButton KeyLCtrl(29); // LCtrl
		static cube::DButton KeyA = cube::DButton(30); //A
		static cube::DButton KeyS = cube::DButton(31); //S
		static cube::DButton KeyD = cube::DButton(32); //D


		KeySpace.Update(diKeys);
		KeyLCtrl.Update(diKeys);
		KeyW.Update(diKeys);
		KeyA.Update(diKeys);
		KeyS.Update(diKeys);
		KeyD.Update(diKeys);

		cube::Game* game = cube::GetGame();
		cube::Creature* player = game->GetPlayer();

		if (KeySpace.Pressed() == cube::DButton::State::Held) {
			player->entity_data.position.z += CAMERA_SPEED;
		}
		if (KeyLCtrl.Pressed() == cube::DButton::State::Held) {
			player->entity_data.position.z -= CAMERA_SPEED;
		}

		// Add Atan2 Math based on Camera Rotation
		double s = sin(game->camera_angle.z * PI / 180);
		double c = cos(game->camera_angle.z * PI / 180);

		if (KeyW.Pressed() == cube::DButton::State::Held) {
			player->entity_data.position.y -= c * CAMERA_SPEED;
			player->entity_data.position.x += s * CAMERA_SPEED;
		}
		if (KeyS.Pressed() == cube::DButton::State::Held) {
			player->entity_data.position.y += c * CAMERA_SPEED;
			player->entity_data.position.x -= s * CAMERA_SPEED;
		}
		if (KeyA.Pressed() == cube::DButton::State::Held) {
			player->entity_data.position.y += s * CAMERA_SPEED;
			player->entity_data.position.x += c * CAMERA_SPEED;
		}
		if (KeyD.Pressed() == cube::DButton::State::Held) {
			player->entity_data.position.y -= s * CAMERA_SPEED;
			player->entity_data.position.x -= c * CAMERA_SPEED;
		}
	}
}

void CamMod::SaveRecord(std::string record_name) {
	CreateDirectory("Mods\\CubeCamera\\jsons", NULL);
	std::ofstream file;
	json j;
	for (int id = 0; id < key_pos_list.size(); id++) {
		std::string s_id = std::to_string(id);
		j[s_id] = {
			{"duration", key_seconds[id]},
			{"anim_type", key_animation_list[id]},
			{"pos_x", key_pos_list[id].x},
			{"pos_y", key_pos_list[id].y},
			{"pos_z", key_pos_list[id].z},
			{"ang_x", key_angle_list[id].x},
			{"ang_y", key_angle_list[id].y},
			{"ang_z", key_angle_list[id].z}
		};
	}
	std::string to_json = j.dump();
	file.open("Mods\\CubeCamera\\jsons\\" + record_name, std::ios::out);
	file << to_json.c_str();
	file.close();
	cube::GetGame()->PrintMessage(L"Don't forget to rename the newly created file located at Mods/CubeCamera/jsons/.. !\n", 255, 100, 100);
}

void CamMod::LoadRecord(std::string record_name) {
	std::ifstream json_file("Mods\\CubeCamera\\jsons\\" + record_name);
	if (json_file.is_open()) {
		std::string text;
		std::string temp;
		while (std::getline(json_file, temp)) {
			text.append(temp);
		}
		picojson::value v;
		std::string err = picojson::parse(v, text);
		if (!err.empty()) {
			cube::GetGame()->PrintMessage(L"Error loading JSON file");
			json_file.close();
			return;
		}
		int id = 0;
		key_seconds.clear();
		key_animation_list.clear();
		key_pos_list.clear();
		key_angle_list.clear();
		while (v.contains(std::to_string(id))) {
			std::string s_id = std::to_string(id);
			float duration = std::stof(v.get(s_id).get("duration").to_str());
			int anim_type = std::stoi(v.get(s_id).get("anim_type").to_str());
			LongVector3 pos = LongVector3(
				std::stol(v.get(s_id).get("pos_x").to_str()),
				std::stol(v.get(s_id).get("pos_y").to_str()),
				std::stol(v.get(s_id).get("pos_z").to_str())
			);
			DoubleVector3 ang = DoubleVector3(
				std::stod(v.get(s_id).get("ang_x").to_str()),
				std::stod(v.get(s_id).get("ang_y").to_str()),
				std::stod(v.get(s_id).get("ang_z").to_str())
			);
			key_seconds.push_back(duration);
			key_animation_list.push_back(AnimationType(anim_type));
			key_pos_list.push_back(pos);
			key_angle_list.push_back(ang);
			id++;
		}
	}
	json_file.close();
}

// Save and Load system made by ChrisMiunchiz
void CamMod::SaveKeyBinds() {
	CreateDirectory("Mods\\CubeCamera", NULL);
	std::ofstream file;
	file.open("Mods\\CubeCamera\\keybinds.txt", std::ios::out | std::ios::binary);
	file.write((char*)&guiButton.diKey, sizeof(guiButton.diKey));
	file.close();
}

void CamMod::LoadKeyBinds() {
	CreateDirectory("Mods\\CubeCamera", NULL);
	std::ifstream file("Mods\\CubeCamera\\keybinds.txt", std::ios::in | std::ios::binary | std::ios::ate);
	if (file.is_open()) {
		size_t fsize = file.tellg();
		if (fsize == sizeof(guiButton.diKey)) {
			file.seekg(0, std::ios::beg);
			file.read((char*)&guiButton.diKey, fsize);
		}
		file.close();
	}
	SaveKeyBinds();
}