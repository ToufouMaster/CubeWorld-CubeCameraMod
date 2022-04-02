#include "CamMod.h"

#define CAMERA_SPEED 16384
	// Public variables.
	// For example: int number = 10;
	

	// Includes for the self written hooks.
	// For example: #include "src/hooks/a_hook.h" 

	/* Mod class containing all the functions for the mod.
	*/


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
				return true;
			}
		}
		else {
			game->PrintMessage(L"Recording is not activated!\n", 255, 100, 100);
			return false;
		}
	}

	bool CamMod::RemoveKey(int id) {
		cube::Game* game = cube::GetGame();
		if (recording) {
			if (id < key_pos_list.size()) {
				key_pos_list.erase(key_pos_list.begin() + id);
				key_angle_list.erase(key_angle_list.begin() + id);
				key_seconds.erase(key_seconds.begin() + id);
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
					old_pos = key_pos_list[0];
					old_angle = key_angle_list[0];
					int steps_per_key = (int)(60 * key_seconds[viewer_key_id]);
					viewer_key_id = 1;
					next_key_pos = key_pos_list[viewer_key_id];
					vel = LongVector3(
						(next_key_pos.x - player->entity_data.position.x) / steps_per_key,
						(next_key_pos.y - player->entity_data.position.y) / steps_per_key,
						(next_key_pos.z - player->entity_data.position.z) / steps_per_key
					);
					next_key_angle = key_angle_list[viewer_key_id];
					angle_vel = DoubleVector3(
						(next_key_angle.x - old_angle.x) / steps_per_key,
						(next_key_angle.y - old_angle.y) / steps_per_key,
						(next_key_angle.z - old_angle.z) / steps_per_key
					);
					return true;
				}
				else {
					game->PrintMessage(L"No keys set yet!\n", 255, 100, 100);
				}
			}
			return false;
		}
		game->target_camera_distance = 6;
		step = 0;
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
			int steps_per_key = (int)(60 * key_seconds[viewer_key_id - 1]);
			if (step >= steps_per_key) {
				step = 0;
				player->entity_data.position = next_key_pos;
				game->camera_angle = next_key_angle;
				if (viewer_key_id + 1 >= key_pos_list.size()) {
					viewer = false;
					viewer_key_id = 0;
					player->entity_data.position = key_pos_list[key_pos_list.size()-1];
					game->target_camera_angle = key_angle_list[key_angle_list.size() - 1];
					return;
				}
				else {
					old_pos = key_pos_list[viewer_key_id];
					old_angle = key_angle_list[viewer_key_id];
					steps_per_key = (int)(60 * key_seconds[viewer_key_id]);
					viewer_key_id += 1;
					next_key_pos = key_pos_list[viewer_key_id];
					vel = LongVector3(
						(next_key_pos.x - player->entity_data.position.x) / steps_per_key,
						(next_key_pos.y - player->entity_data.position.y) / steps_per_key,
						(next_key_pos.z - player->entity_data.position.z) / steps_per_key
					);
					next_key_angle = key_angle_list[viewer_key_id];
					angle_vel = DoubleVector3(
						(next_key_angle.x - old_angle.x) / steps_per_key,
						(next_key_angle.y - old_angle.y) / steps_per_key,
						(next_key_angle.z - old_angle.z) / steps_per_key
					);
				}
			}
			player->entity_data.position.x = old_pos.x + vel.x * step;
			player->entity_data.position.y = old_pos.y + vel.y * step;
			player->entity_data.position.z = old_pos.z + vel.z * step;
			game->target_camera_angle.x = old_angle.x + angle_vel.x * step;
			game->target_camera_angle.y = old_angle.y + angle_vel.y * step;
			game->target_camera_angle.z = old_angle.z + angle_vel.z * step;
			step += 1;
		}
		if (previewer) {
			game->camera_distance = 0;
			if (previewer_key_id < key_pos_list.size()) {
				if (previewertime > 0 and previewer_key_id < key_pos_list.size() - 1) {
					LongVector3 old_pos = key_pos_list[previewer_key_id];
					DoubleVector3 old_angle = key_angle_list[previewer_key_id];
					LongVector3 new_pos = key_pos_list[previewer_key_id +1];
					DoubleVector3 new_angle = key_angle_list[previewer_key_id +1];
					LongVector3 previewposvec = LongVector3((new_pos.x - old_pos.x) * previewertime, (new_pos.y - old_pos.y) * previewertime, (new_pos.z - old_pos.z) * previewertime);
					DoubleVector3 previewanglevec = DoubleVector3(old_angle.x + (new_angle.x - old_angle.x) * previewertime, old_angle.y + (new_angle.y - old_angle.y) * previewertime, old_angle.z + (new_angle.z - old_angle.z) * previewertime);
					player->entity_data.position.x = old_pos.x + previewposvec.x;
					player->entity_data.position.y = old_pos.y + previewposvec.y;
					player->entity_data.position.z = old_pos.z + previewposvec.z;
					game->target_camera_angle = previewanglevec;
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

	void CamMod::OnPresent(IDXGISwapChain* SwapChain, UINT SyncInterval, UINT Flags) {
		guiWindow->Present();
	}

	/* Function hook that gets called on intialization of cubeworld.
	 * [Note]:	cube::GetGame() is not yet available here!!
	 * @return	{void}
	*/

	void CamMod::Initialize() {
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