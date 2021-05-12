#include "Level0.h"

#define LEVEL0_WIDTH 18
#define LEVEL0_HEIGHT 12

#define LEVEL0_ENEMY_COUNT 0

#define LEVEL0_TRAIL_COUNT 0

#define __ 0

unsigned int level0_data[] =
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,42,42,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,0,42,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,0,42,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,0,42,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,42,42,42
};

unsigned int level0_bounce_data[] =
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0


};

unsigned int level0_force_data[] =
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0

};

void Level0::Initialize() {
	GLuint mapTextureID = Util::LoadTexture("assets/tileset-blob.png", true);
	GLuint hbounceTextureID = Util::LoadTexture("assets/tileset-blob-hbounce.png", true);
	GLuint vbounceTextureID = Util::LoadTexture("assets/tileset-blob-vbounce.png", true);
	state.map = new Map * [3];
	state.map[0] = new Map(LEVEL0_WIDTH, LEVEL0_HEIGHT, level0_bounce_data, hbounceTextureID, 1.0f, 7, 7);
	state.map[1] = new Map(LEVEL0_WIDTH, LEVEL0_HEIGHT, level0_force_data,  vbounceTextureID, 1.0f, 7, 7);
	state.map[2] = new Map(LEVEL0_WIDTH, LEVEL0_HEIGHT, level0_data, mapTextureID, 1.0f, 7, 7);
	

	// Move over all of the player and enemy code from initialization.
		// Initialize Player
	state.player = new Entity();
	//state.player->position = glm::vec3(26, -83, 0);
	state.player->isActive = false;
	// Initialize Game Objects
	state.currentTrail = 0;
	state.inputs = 0b000;

	state.fontTextureID = Util::LoadTexture("assets/font.png");
}

void Level0::Update(float deltaTime) {
	state.player->Update(deltaTime, NULL, LEVEL0_ENEMY_COUNT, state.map);
}

void Level0::Render(ShaderProgram* program) {
	Util::DrawText(program, state.fontTextureID, "FALL BUNNEH", 1.0f, 0.0f, glm::vec3(16.0f - 5.0f, -8 - 0.5f, 0.0f));
	Util::DrawText(program, state.fontTextureID, "PRESS ENTER", 1.0f, 0.0f, glm::vec3(16.0f - 5.0f, -8 - 1.5f, 0.0f));

}