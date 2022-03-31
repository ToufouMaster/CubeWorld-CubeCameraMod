#include "main.h"
#include <cmath>

#define PI 3.14159265
#define CAMERA_SPEED 16384
// Public variables.
// For example: int number = 10;

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

std::vector<LongVector3> key_pos_list;
std::vector<DoubleVector3> key_angle_list;

// Includes for the self written hooks.
// For example: #include "src/hooks/a_hook.h" 

/* Mod class containing all the functions for the mod.
*/


class Mod : GenericMod {

	/* Hook for the chat function. Triggers when a user sends something in the chat.
	 * @param	{std::wstring*} message
	 * @return	{int}
	*/
	virtual int OnChat(std::wstring* message) override {
		cube::Game* game = cube::GetGame();
		cube::Creature* player = game->GetPlayer();
		const wchar_t* msg = message->c_str();
		if (_wcsicmp(msg, L"/free") == 0) {
			if (not free_cam) {
				player_appearance = player->entity_data.appearance;
			} else {
				player->entity_data.appearance = player_appearance;
			}
			free_cam = not free_cam;
			return 1;
		}
		if (_wcsicmp(msg, L"/record start") == 0) {
			if (viewer) {
				game->PrintMessage(L"Error: Viewer still activated!\n", 255, 100, 100);
				return 1;
			}
			recording = true;
			game->PrintMessage(L"Recording!\nType /addkey to add a new keyposition to the record\nType /delkey <id> to remove the key from the recording\nAnd finally type /stoprecord to stop recording\n[don't forget to type /free to disable freecam]", 255, 50, 50);
			return 1;
		}
		if (_wcsicmp(msg, L"/record add") == 0) {
			if (recording) {
				wchar_t buffer[250];
				swprintf_s(buffer, 250, L"Key succesfully added to id %d!\n", key_pos_list.size());
				game->PrintMessage(buffer, 100, 255, 100);
				key_pos_list.push_back(player->entity_data.position);
				key_angle_list.push_back(game->camera_angle);
			}
			else {
				game->PrintMessage(L"Recording is not activated!\n", 255, 100, 100);
			}
			return 1;
		}
		if (_wcsicmp(msg, L"/record stop") == 0) {
			recording = false;
			viewer = false;
		}
		if (_wcsicmp(msg, L"/record view") == 0) {
			if (not recording) {
				if (key_pos_list.size() > 0) {
					viewer = true;
					player->entity_data.position = key_pos_list[0];
					game->camera_angle = key_angle_list[0];
					old_pos = key_pos_list[0];
					old_angle = key_angle_list[0];
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
				}
				else {
					game->PrintMessage(L"No keys set yet!\n", 255, 100, 100);
				}
			}
		}

		return 0;
	}

	/* Function hook that gets called every game tick.
	 * @param	{cube::Game*} game
	 * @return	{void}
	*/
	virtual void OnGameTick(cube::Game* game) override {
		cube::Creature* player = game->GetPlayer();
		if (viewer) {
			if (step >= steps_per_key) {
				step = 0;
				player->entity_data.position = next_key_pos;
				game->camera_angle = next_key_angle;
				if (viewer_key_id+1 >= key_pos_list.size()) {
					viewer = false;
					viewer_key_id = 0;
				}
				else {
					old_pos = key_pos_list[viewer_key_id];
					old_angle = key_angle_list[viewer_key_id];
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
			game->PrintMessage(std::to_wstring(game->target_camera_angle.z).c_str());
			game->PrintMessage(L" ");
			game->PrintMessage(std::to_wstring(angle_vel.z).c_str());
			game->PrintMessage(L"\n");
			step += 1;
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

	/* Function hook that gets called on intialization of cubeworld.
	 * [Note]:	cube::GetGame() is not yet available here!!
	 * @return	{void}
	*/
	virtual void Initialize() override {
		return;
	}

	void OnGetKeyboardState(BYTE* diKeys) {
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
};

// Export of the mod created in this file, so that the modloader can see and use it.
EXPORT Mod* MakeMod() {
	return new Mod();
}