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

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Entity.h"

#include <vector>

#define OBJECT_COUNT 44

struct GameState {
	Entity* player;
	Entity* objects;
	GLuint fontTextureID;
	int inputs;
};

GameState state;

SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, projectionMatrix;

GLuint LoadTexture(const char* filePath)
{
	int w, h, n;
	unsigned char* image = stbi_load(filePath, &w, &h, &n, STBI_rgb_alpha);

	if (image == NULL) {
		std::cout << "Unable to load image. Make sure the path is correct\n";
		assert(false);
	}

	GLuint textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return textureID;
}

void DrawText(ShaderProgram* program, GLuint fontTextureID, std::string text,
	float size, float spacing, glm::vec3 position)
{
	float width = 1.0f / 16.0f;
	float height = 1.0f / 16.0f;

	std::vector<float> vertices;
	std::vector<float> texCoords;

	for (int i = 0; i < text.size(); i++) {
		int index = (int)text[i];
		float offset = (size + spacing) * i;

		float u = (float)(index % 16) / 16.0f;
		float v = (float)(index / 16) / 16.0f;

		vertices.insert(vertices.end(), {
			offset + (-0.5f * size), 0.5f * size,
			offset + (-0.5f * size), -0.5f * size,
			offset + (0.5f * size), 0.5f * size,
			offset + (0.5f * size), -0.5f * size,
			offset + (0.5f * size), 0.5f * size,
			offset + (-0.5f * size), -0.5f * size,
			});

		texCoords.insert(texCoords.end(), {
			u, v,
			u, v + height,
			u + width, v,
			u + width, v + height,
			u + width, v,
			u, v + height,
			});
	}

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	program->SetModelMatrix(modelMatrix);

	glUseProgram(program->programID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords.data());
	glEnableVertexAttribArray(program->texCoordAttribute);

	glBindTexture(GL_TEXTURE_2D, fontTextureID);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

	glDrawArrays(GL_TRIANGLES, 0, (int)(text.size() * 6));

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}


void Initialize()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Press up!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 540, 810,
		SDL_WINDOW_OPENGL);
	SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
	SDL_GL_MakeCurrent(displayWindow, context);

#ifdef _WINDOWS
	glewInit();
#endif

	glViewport(0, 0, 540, 810);

	program.Load("shaders/vertex_textured.glsl", "shaders/fragment_textured.glsl");

	viewMatrix = glm::mat4(1.0f);
	modelMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-4.0f, 4.0f, -6.0f, 6.0f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);

	glUseProgram(program.programID);

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


	// Initialize Game Objects

	// Initialize Player
	state.player = new Entity();
	state.player->position = glm::vec3(0, 5.0f, 0);
	state.player->movement = glm::vec3(0);
	state.player->acceleration = glm::vec3(0, -1.0f, 0);
	state.player->speed = 2.0f;
	state.player->textureID = LoadTexture("assets/ship.png");

	state.player->entityType = PLAYER;

	state.player->animD = new int[2]{ 0,  8 };
	state.player->animL = new int[2]{ 1,  9 };
	state.player->animU = new int[2]{ 2, 10 };
	state.player->animR = new int[2]{ 3, 11 };
	state.player->animLU = new int[2]{ 4, 12 };
	state.player->animUR = new int[2]{ 5, 13 };
	state.player->animLR = new int[2]{ 6, 14 };
	state.player->animLUR = new int[2]{ 7, 15 };

	state.player->animIndices = state.player->animD;
	state.player->animFrames = 2;
	state.player->animIndex = 0;
	state.player->animTime = 0;
	state.player->animCols = 8;
	state.player->animRows = 2;

	state.player->height = 0.5f;
	state.player->width = 0.5f;

	state.player->jumpPower = 5.0f;

	state.objects = new Entity[OBJECT_COUNT];

	GLuint wallTextureID = LoadTexture("assets/wall.png");
	GLuint goalTextureID = LoadTexture("assets/goal.png");
	GLuint startTextureID = LoadTexture("assets/start.png");


	state.objects[0].textureID = goalTextureID;
	state.objects[0].position = glm::vec3(2.0f - 3.5f, 0.0f - 5.5f, 0);
	state.objects[0].entityType = GOAL;

	state.objects[1].textureID = goalTextureID;
	state.objects[1].position = glm::vec3(5.0f - 3.5f, 0.0f - 5.5f, 0);
	state.objects[1].entityType = GOAL;

	state.objects[2].textureID = startTextureID;
	state.objects[2].position = glm::vec3(0, 4.125f, 0);
	state.objects[2].entityType = START;

	state.objects[3].position = glm::vec3(0.0f - 3.5f, 00.0f - 5.5f, 0);
	state.objects[4].position = glm::vec3(0.0f - 3.5f, 01.0f - 5.5f, 0);
	state.objects[5].position = glm::vec3(0.0f - 3.5f, 02.0f - 5.5f, 0);
	state.objects[6].position = glm::vec3(0.0f - 3.5f, 03.0f - 5.5f, 0);
	state.objects[7].position = glm::vec3(0.0f - 3.5f, 04.0f - 5.5f, 0);
	state.objects[8].position = glm::vec3(0.0f - 3.5f, 05.0f - 5.5f, 0);
	state.objects[9].position = glm::vec3(0.0f - 3.5f, 06.0f - 5.5f, 0);
	state.objects[10].position = glm::vec3(0.0f - 3.5f, 07.0f - 5.5f, 0);
	state.objects[11].position = glm::vec3(0.0f - 3.5f, 08.0f - 5.5f, 0);
	state.objects[12].position = glm::vec3(0.0f - 3.5f, 09.0f - 5.5f, 0);
	state.objects[13].position = glm::vec3(0.0f - 3.5f, 10.0f - 5.5f, 0);
	state.objects[14].position = glm::vec3(0.0f - 3.5f, 11.0f - 5.5f, 0);
	state.objects[15].position = glm::vec3(7.0f - 3.5f, 00.0f - 5.5f, 0);
	state.objects[16].position = glm::vec3(7.0f - 3.5f, 01.0f - 5.5f, 0);
	state.objects[17].position = glm::vec3(7.0f - 3.5f, 02.0f - 5.5f, 0);
	state.objects[18].position = glm::vec3(7.0f - 3.5f, 03.0f - 5.5f, 0);
	state.objects[19].position = glm::vec3(7.0f - 3.5f, 04.0f - 5.5f, 0);
	state.objects[20].position = glm::vec3(7.0f - 3.5f, 05.0f - 5.5f, 0);
	state.objects[21].position = glm::vec3(7.0f - 3.5f, 06.0f - 5.5f, 0);
	state.objects[22].position = glm::vec3(7.0f - 3.5f, 07.0f - 5.5f, 0);
	state.objects[23].position = glm::vec3(7.0f - 3.5f, 08.0f - 5.5f, 0);
	state.objects[24].position = glm::vec3(7.0f - 3.5f, 09.0f - 5.5f, 0);
	state.objects[25].position = glm::vec3(7.0f - 3.5f, 10.0f - 5.5f, 0);
	state.objects[26].position = glm::vec3(7.0f - 3.5f, 11.0f - 5.5f, 0);
	state.objects[27].position = glm::vec3(1.0f - 3.5f, 00.0f - 5.5f, 0);
	state.objects[28].position = glm::vec3(3.0f - 3.5f, 00.0f - 5.5f, 0);
	state.objects[29].position = glm::vec3(4.0f - 3.5f, 00.0f - 5.5f, 0);
	state.objects[30].position = glm::vec3(6.0f - 3.5f, 00.0f - 5.5f, 0);
	state.objects[31].position = glm::vec3(4.0f - 3.5f, 01.0f - 5.5f, 0);
	state.objects[32].position = glm::vec3(7.0f - 3.5f, 01.0f - 5.5f, 0);
	state.objects[33].position = glm::vec3(3.0f - 3.5f, 02.0f - 5.5f, 0);
	state.objects[34].position = glm::vec3(4.0f - 3.5f, 02.0f - 5.5f, 0);
	state.objects[35].position = glm::vec3(5.0f - 3.5f, 02.0f - 5.5f, 0);
	state.objects[36].position = glm::vec3(3.0f - 3.5f, 03.0f - 5.5f, 0);
	state.objects[37].position = glm::vec3(3.0f - 3.5f, 04.0f - 5.5f, 0);
	state.objects[38].position = glm::vec3(5.0f - 3.5f, 04.0f - 5.5f, 0);
	state.objects[39].position = glm::vec3(6.0f - 3.5f, 04.0f - 5.5f, 0);
	state.objects[40].position = glm::vec3(3.0f - 3.5f, 05.0f - 5.5f, 0);
	state.objects[41].position = glm::vec3(3.0f - 3.5f, 06.0f - 5.5f, 0);
	state.objects[42].position = glm::vec3(4.0f - 3.5f, 06.0f - 5.5f, 0);
	state.objects[43].position = glm::vec3(5.0f - 3.5f, 06.0f - 5.5f, 0);


	for (int i = 0; i < OBJECT_COUNT; i++) {
		state.objects[i].Update(0, NULL, 0);
		if (state.objects[i].entityType == NONE) {
			state.objects[i].entityType = WALL;
			state.objects[i].textureID = wallTextureID;
			//    state.objects[i].width = 0.9;
			//    state.objects[i].height = 0.9;
		}
	}

	state.fontTextureID = LoadTexture("assets/font.png");

	state.inputs = 0b000;
}

void ProcessInput()
{

	state.player->movement = glm::vec3(0);
	state.inputs = 0b000;

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

			case SDLK_UP:
				state.objects[2].isActive = 0;
				break;

			case SDLK_SPACE:
				// if (state.player->collidedBottom) state.player->jump = true;
				break;
			}
			break; // SDL_KEYDOWN
		}
	}

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	state.inputs |= keys[SDL_SCANCODE_LEFT] ? 0b100 : 0b000;
	state.inputs |= keys[SDL_SCANCODE_UP] ? 0b010 : 0b000;
	state.inputs |= keys[SDL_SCANCODE_RIGHT] ? 0b001 : 0b000;

	switch (state.inputs & 0b101) {
	case 0b100:
		state.player->movement.x = -1;
		break;
	case 0b001:
		state.player->movement.x = 1;
		break;
	}
	if ((state.inputs & 0b010) == 0b010) {
		state.player->movement.y = 2.0f;
	}

	switch (state.inputs) {
	case 0b001:
		state.player->animIndices = state.player->animR;
		break;
	case 0b010:
		state.player->animIndices = state.player->animU;
		break;
	case 0b011:
		state.player->animIndices = state.player->animUR;
		break;
	case 0b100:
		state.player->animIndices = state.player->animL;
		break;
	case 0b101:
		state.player->animIndices = state.player->animLR;
		break;
	case 0b110:
		state.player->animIndices = state.player->animLU;
		break;
	case 0b111:
		state.player->animIndices = state.player->animLUR;
		break;
	default:
		state.player->animIndices = state.player->animD;
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
		state.player->Update(FIXED_TIMESTEP, state.objects, OBJECT_COUNT);
		deltaTime -= FIXED_TIMESTEP;
	}
	accumulator = deltaTime;
}


void Render()
{
	glClear(GL_COLOR_BUFFER_BIT);

	for (int i = 0; i < OBJECT_COUNT; i++) {
		state.objects[i].Render(&program);
	}

	state.player->Render(&program);

	if (state.objects[2].isActive) {
		DrawText(&program, state.fontTextureID, "Press Up", 0.25f, 0, glm::vec3(-(7.0f / 8.0f), state.player->position.y - 1,
			0));
	}

	if (state.player->lastCollision == WALL) {
		DrawText(&program, state.fontTextureID, "Mission Failed", 0.25f, 0, glm::vec3(-(13.0f / 8.0f), 4.0, 0));
	}


	if (state.player->lastCollision == GOAL) {
		DrawText(&program, state.fontTextureID, "Mission Successful", 0.25f, 0, glm::vec3(-(17.0f / 8.0f), 4.0, 0));
	}


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
