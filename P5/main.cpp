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
#include "Entity.h"

#include "Util.h"
#include "Map.h"
#include "Scene.h"
#include "Level1.h"

// #include <vector>

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

Scene* currentScene;
Level1* level1;

void SwitchToScene(Scene* scene) {
	currentScene = scene;
	currentScene->Initialize();
}

void Initialize()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Press up!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
		SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 640, 480);

	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	float projWidth = 5.0f;
	float projHeight = 3.75f;

	float xOffset = 0.0f;
	float yOffset = 0.0f;

	projectionMatrix = glm::ortho(-projWidth + xOffset, projWidth + xOffset, -projHeight + yOffset, projHeight + yOffset, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	glUseProgram(program.programID);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	level1 = new Level1();
	SwitchToScene(level1);
}



void ProcessInput()
{

	currentScene->state.player->movement = glm::vec3(0);
	currentScene->state.inputs = 0b000;

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;

		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
				// Move the player left
				break;

			case SDLK_RIGHT:
				// Move the player right
				break;

			case SDLK_SPACE:
				// if (state.player->collidedBottom) state.player->jump = true;
				break;
				//			case SDLK_r:
				//				Initialize();
				//				break;
			}

			break; // SDL_KEYDOWN
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	currentScene->state.inputs |= keys[SDL_SCANCODE_LEFT] ? 0b100 : 0b000;
	currentScene->state.inputs |= keys[SDL_SCANCODE_UP] ? 0b010 : 0b000;
	currentScene->state.inputs |= keys[SDL_SCANCODE_RIGHT] ? 0b001 : 0b000;

	switch (currentScene->state.inputs & 0b101) {
	case 0b100:
		currentScene->state.player->movement.x = -1;
		break;
	case 0b001:
		currentScene->state.player->movement.x = 1;
		break;
	}
	if ((currentScene->state.inputs & 0b010) == 0b010) {
		currentScene->state.player->movement.y = 8.0f;
	}

	switch (currentScene->state.inputs) {
	case 0b001:
		currentScene->state.player->animIndices = currentScene->state.player->animR;
		break;
	case 0b010:
		currentScene->state.player->animIndices = currentScene->state.player->animU;
		break;
	case 0b011:
		currentScene->state.player->animIndices = currentScene->state.player->animUR;
		break;
	case 0b100:
		currentScene->state.player->animIndices = currentScene->state.player->animL;
		break;
	case 0b101:
		currentScene->state.player->animIndices = currentScene->state.player->animLR;
		break;
	case 0b110:
		currentScene->state.player->animIndices = currentScene->state.player->animLU;
		break;
	case 0b111:
		currentScene->state.player->animIndices = currentScene->state.player->animLUR;
		break;
	default:
		currentScene->state.player->animIndices = currentScene->state.player->animD;
	}


	//if (glm::length(state.player->movement) > 1.0f) {
	//    state.player->movement = glm::normalize(state.player->movement);
	//}

}

constexpr auto FIXED_TIMESTEP = (1.0f / 288.0f);
float lastTicks = 0;
float accumulator = 0.0f;
void Update()
{
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;
	deltaTime += accumulator;
	if (deltaTime < FIXED_TIMESTEP) {
		accumulator = deltaTime;
		return;
	}
	while (deltaTime >= FIXED_TIMESTEP) {
		// Update. Notice it's FIXED_TIMESTEP. Not deltaTime
		currentScene->Update(FIXED_TIMESTEP);
		// state.player->Update(FIXED_TIMESTEP, NULL, 0, state.map);
		deltaTime -= FIXED_TIMESTEP;
	}
	accumulator = deltaTime;

	viewMatrix = glm::mat4(1.0f);
	if (currentScene->state.player->position.x > 5) {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-1 * currentScene->state.player->position.x, 3.75, 0));
	}
	else {
		viewMatrix = glm::translate(viewMatrix, glm::vec3(-5, 3.75, 0));
	}
}


void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program.programID);

	program.SetViewMatrix(viewMatrix);

	currentScene->Render(&program);

	SDL_GL_SwapWindow(displayWindow);
}


void Shutdown()
{
	SDL_Quit();
}

int main(int argc, char* argv[])
{
	Initialize();

	while (gameIsRunning) {
		ProcessInput();
		Update();
		Render();
	}

	Shutdown();
	return 0;
}
