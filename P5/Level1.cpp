#include "Level1.h"

#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8

#define LEVEL1_ENEMY_COUNT 1

unsigned int level1_data[] =
{
3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1,
3, 1, 1, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2,
3, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2
};

void Level1::Initialize() {
	GLuint mapTextureID = Util::LoadTexture("assets/tileset.png");
	state.map = new Map(LEVEL1_WIDTH, LEVEL1_HEIGHT, level1_data, mapTextureID, 1.0f, 4, 1);

	// Move over all of the player and enemy code from initialization.
		// Initialize Player
	state.player = new Entity();
	state.player->position = glm::vec3(2, 3, 0);
	state.player->movement = glm::vec3(0);
	state.player->acceleration = glm::vec3(0, -4.0f, 0);
	state.player->speed = 4.0f;
	state.player->textureID = Util::LoadTexture("assets/ship.png");

	state.player->entityType = PLAYER;

	state.player->animD = new int[4]{ 0,  8, 16, 24 };
	state.player->animL = new int[4]{ 1,  9, 17, 25 };
	state.player->animU = new int[4]{ 2, 10, 18, 26 };
	state.player->animR = new int[4]{ 3, 11, 19, 27 };
	state.player->animLU = new int[4]{ 4, 12, 20, 28 };
	state.player->animUR = new int[4]{ 5, 13, 21, 29 };
	state.player->animLR = new int[4]{ 6, 14, 22, 30 };
	state.player->animLUR = new int[4]{ 7, 15, 23, 31 };

	state.player->animIndices = state.player->animD;
	state.player->animFrames = 4;
	state.player->animIndex = 0;
	state.player->animTime = 0;
	state.player->animCols = 8;
	state.player->animRows = 4;

	state.player->height = 0.5f;
	state.player->width = 0.5f;

	state.player->jumpPower = 5.0f;

	// Initialize Game Objects

	state.inputs = 0b000;

}

void Level1::Update(float deltaTime) {
	state.player->Update(deltaTime, state.enemies, 0, state.map);
}

void Level1::Render(ShaderProgram* program) {
	state.map->Render(program);
	state.player->Render(program);
}