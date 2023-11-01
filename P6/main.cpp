#define GL_SILENCE_DEPRECATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1
#include <SDL.h>
#include <SDL_mixer.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "Entity.h"

#include "Util.h"
#include "Map.h"
#include "Scene.h"
#include "Level0.h"
#include "Level1.h"

// #include <vector>

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

Scene* currentScene;
Scene* sceneList[2];

GLuint fontTextureID;

int viewX;
int viewY;

void SwitchToScene(Scene* scene) {
	currentScene = scene;
	currentScene->Initialize();
}
Mix_Music* music;

void Initialize()
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
	displayWindow = SDL_CreateWindow("FALL BUNNEH", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1920, 1080,
		SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 1920, 1080);

	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	float projWidth = 16.0f;
	float projHeight = 9.0f;

	float xOffset = 16.0f;
	float yOffset = -9.0f;

	projectionMatrix = glm::ortho(-projWidth + xOffset, projWidth + xOffset, -projHeight + yOffset, projHeight + yOffset, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	glUseProgram(program.programID);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	fontTextureID = Util::LoadTexture("assets/font.png");

	sceneList[0] = new Level0();
	sceneList[1] = new Level1();

	Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
	music = Mix_LoadMUS("assets/stage1.mp3");
	Mix_VolumeMusic(MIX_MAX_VOLUME / 2);
	Mix_PlayMusic(music, -1);
	SwitchToScene(sceneList[0]);
}




void ProcessInput()
{

	//currentScene->state.player->movement = glm::vec3(0);
	currentScene->state.inputs = 0b000;

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	currentScene->state.inputs |= keys[SDL_SCANCODE_LEFT] ? 0b100 : 0b000;
	currentScene->state.inputs |= keys[SDL_SCANCODE_UP] ? 0b010 : 0b000;
	currentScene->state.inputs |= keys[SDL_SCANCODE_RIGHT] ? 0b001 : 0b000;

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
				if (!currentScene->state.player->jumpCharge)
				currentScene->state.player->movement.x = -1;
				currentScene->state.presses += 0.5f;
				break;

			case SDLK_RIGHT:
				// Move the player right
				if (!currentScene->state.player->jumpCharge)
				currentScene->state.player->movement.x = 1;
				currentScene->state.presses += 0.5f;
				break;

			case SDLK_RETURN:
				SwitchToScene(sceneList[1]);
				break;
			}

			break; // SDL_KEYDOWN
		case SDL_KEYUP:
			switch (event.key.keysym.sym) {
			case SDLK_LEFT:
				currentScene->state.presses += 0.5f;
				if (!keys[SDL_SCANCODE_RIGHT]) {
					if (currentScene->state.player->collidedBottom) {
						currentScene->state.player->jump = true;
						currentScene->state.player->jumpCharge = false;
					}
				}
				break;
			case SDLK_RIGHT:
				currentScene->state.presses += 0.5f;
				if (!keys[SDL_SCANCODE_LEFT]) {
					if (currentScene->state.player->collidedBottom) {
						currentScene->state.player->jump = true;
						currentScene->state.player->jumpCharge = false;
					}
				}
				break;

			}
		}
	}



	switch (currentScene->state.inputs & 0b101) {
	case 0b100:
		currentScene->state.player->movement.x = -1;
		currentScene->state.player->jumpCharge = true;
		break;
	case 0b001:
		currentScene->state.player->movement.x = 1;
		currentScene->state.player->jumpCharge = true;
		break;
	case 0b101:
		currentScene->state.player->jumpCharge = true;
		break;
	}
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
	viewX = currentScene->state.player->position.x;
	viewY = currentScene->state.player->position.y;
	viewX = viewX / 32 * 32;
	viewY = viewY / 18 * 18;
	viewMatrix = glm::translate(viewMatrix, glm::vec3(-viewX, -viewY, 0));

	
}


void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);
	glUseProgram(program.programID);

	program.SetViewMatrix(viewMatrix);

	currentScene->Render(&program);


	//Util::DrawText(&program, fontTextureID, std::to_string(currentScene->state.player->position.x), 0.5f, 0, glm::vec3(currentScene->state.player->position.x, currentScene->state.player->position.y + 1.6f, 0));
	//Util::DrawText(&program, fontTextureID, std::to_string(currentScene->state.player->position.y), 0.5f, 0, glm::vec3(currentScene->state.player->position.x, currentScene->state.player->position.y + 1, 0));

	SDL_GL_SwapWindow(displayWindow);
}


void Shutdown()
{
	Mix_FreeChunk(currentScene->state.jump);
	Mix_FreeChunk(currentScene->state.jump2);
	Mix_FreeChunk(currentScene->state.land);
	Mix_FreeChunk(currentScene->state.charged);
	Mix_FreeChunk(currentScene->state.charge);
	Mix_FreeMusic(music);
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
