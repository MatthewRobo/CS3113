#pragma once

#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Map.h"

enum EntityType {
	NONE = -1,
	PLAYER,
	WALL,
	GOAL,
	START,
	ENEMY
};

enum AIType {
	WALKER,
	BOUNCER,
	UNGA
};

enum AIState {
	IDLE,
	WALKING,
	ATTACKING
};

class Entity
{
public:
	bool isActive = true;

	EntityType entityType = NONE;
	AIType aiType;
	AIState aiState = IDLE;

	glm::vec3 position;
	glm::vec3 movement;
	glm::vec3 acceleration;
	glm::vec3 velocity;

	float width = 1.0f;
	float height = 1.0f;

	bool jump = false;
	float jumpPower = 0.0f;

	float speed;

	GLuint textureID;

	glm::mat4 modelMatrix;

	int* animD = NULL;
	int* animL = NULL;
	int* animU = NULL;
	int* animR = NULL;
	int* animLU = NULL;
	int* animUR = NULL;
	int* animLR = NULL;
	int* animLUR = NULL;


	int* animIndices = NULL;
	int animFrames = 0;
	int animIndex = 0;
	float animTime = 0;
	int animCols = 0;
	int animRows = 0;

	Entity();

	bool collidedTop = false;
	bool collidedBottom = false;
	bool collidedLeft = false;
	bool collidedRight = false;

	Entity* lastCollision = NULL;

	bool CheckCollision(Entity* other);
	void CheckCollisionsY(Entity* objects, int objectCount);
	void CheckCollisionsX(Entity* objects, int objectCount);

	void CheckCollisionsX(Map* map);
	void CheckCollisionsY(Map* map);

	void Update(float deltaTime = 0.0f, Entity* objects = NULL, int objectCount = 0, Map* map = NULL, Entity* player = NULL);

	void Render(ShaderProgram* program);
	void DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index);
		


	void AI(Entity* player = NULL);
	void AIWalker();
	void AIBouncer();
	void AIUnga(Entity* player = NULL);
};
