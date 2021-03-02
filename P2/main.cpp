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

#define BALL_SIZE glm::vec3(0.25f, 0.25f, 0.25f)
#define PADDLE_SIZE glm::vec3(0.25f, 1.0f, 1.0f)
#define PADDLE_DISTANCE glm::vec3(4.75f, 0.0f, 0.0f)
SDL_Window* displayWindow;
bool gameIsRunning = true;

ShaderProgram program;
glm::mat4 viewMatrix, modelMatrix, p1Matrix, p2Matrix, netMatrix, itemMatrix, pointMatrix, projectionMatrix;
glm::vec3 p1_position, p1_movement = glm::vec3(0, 0, 0);
glm::vec3 p2_position, p2_movement = glm::vec3(0, 0, 0);
glm::vec3 item_position, item_movement = glm::vec3(0, 0, 0);
glm::vec3 point_movement, point_gravity = glm::vec3(0, 0, 0);
glm::vec3 point_position = glm::vec3(-100, 0, 0);
bool p1Point = false;
float player_speed = 3.25;
float ball_speed = 4.0;
GLuint whiteTextureID;
GLuint pointTextureID;

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

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

	stbi_image_free(image);
	return textureID;
}

void Initialize()
{
	SDL_Init(SDL_INIT_VIDEO);
	displayWindow = SDL_CreateWindow("Pong!", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480,
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
	p1Matrix = glm::mat4(1.0f);
	p2Matrix = glm::mat4(1.0f);
	itemMatrix = glm::mat4(1.0f);
	netMatrix = glm::mat4(1.0f);
	projectionMatrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

	program.SetProjectionMatrix(projectionMatrix);
	program.SetViewMatrix(viewMatrix);
	// program.SetColor(1.0f, 0.0f, 0.0f, 1.0f);

	glUseProgram(program.programID);

	glClearColor(0.086f, 0.086f, 0.114f, 1.0f);
	glEnable(GL_BLEND);

	// Good setting for transparency
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	item_movement = glm::vec3(1, 2, 0);

	whiteTextureID = LoadTexture("white.png");
	pointTextureID = LoadTexture("victory.png");
}

void ProcessInput()
{
	p1_movement = glm::vec3(0, 0, 0);
	p2_movement = glm::vec3(0, 0, 0);

	const Uint8* keys = SDL_GetKeyboardState(NULL);
	if (keys[SDL_SCANCODE_W]) {
		p1_movement.y += 1.0f;
	}
	if (keys[SDL_SCANCODE_S]) {
		p1_movement.y += -1.0f;
	}

	if (keys[SDL_SCANCODE_UP]) {
		p2_movement.y += 1.0f;
	}
	if (keys[SDL_SCANCODE_DOWN]) {
		p2_movement.y += -1.0f;
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_QUIT:
		case SDL_WINDOWEVENT_CLOSE:
			gameIsRunning = false;
			break;
		case SDL_KEYDOWN:
			switch (event.key.keysym.sym) {
			default:
				if (ball_speed <= 1.0f) ball_speed = 4.0f;
				//  case SDLK_RIGHT:
				//      p1_movement.x = 1.0f;
				//      break;
				//  case SDLK_LEFT:
				//      p1_movement.x = -1.0f;
				//      break;
				//  case SDLK_SPACE:
				//      //  PlayerJump();
				//      break;
			}
			break;
		}
	}
}

float lastTicks = 0.0f;

void Update()
{
	float ticks = (float)SDL_GetTicks() / 1000.0f;
	float deltaTime = ticks - lastTicks;
	lastTicks = ticks;

	p1Matrix = glm::mat4(1.0f);
	p2Matrix = glm::mat4(1.0f);
	itemMatrix = glm::mat4(1.0f);
	pointMatrix = glm::mat4(1.0f);

	item_movement = glm::normalize(item_movement);
	// Add (direction * units per second * elapsed time)
	p1_position += p1_movement * player_speed * deltaTime;
	p1_position.x = -PADDLE_DISTANCE.x;
	p2_position += p2_movement * player_speed * deltaTime;
	p2_position.x = PADDLE_DISTANCE.x;
	item_position += item_movement * ball_speed * deltaTime;
	point_movement += point_gravity * deltaTime;
	point_position += point_movement * deltaTime;

	if (p1_position.y >  3.75f - (PADDLE_SIZE.y / 2.0f)) p1_position.y =  3.75f - (PADDLE_SIZE.y / 2.0f);
	if (p1_position.y < -3.75f + (PADDLE_SIZE.y / 2.0f)) p1_position.y = -3.75f + (PADDLE_SIZE.y / 2.0f);
	if (p2_position.y >  3.75f - (PADDLE_SIZE.y / 2.0f)) p2_position.y =  3.75f - (PADDLE_SIZE.y / 2.0f);
	if (p2_position.y < -3.75f + (PADDLE_SIZE.y / 2.0f)) p2_position.y = -3.75f + (PADDLE_SIZE.y / 2.0f);

	float x1_dist = fabs(p1_position.x - item_position.x) - ((PADDLE_SIZE.x + BALL_SIZE.x) / 2.0f);
	float y1_dist = fabs(p1_position.y - item_position.y) - ((PADDLE_SIZE.y + BALL_SIZE.y) / 2.0f);
	float x2_dist = fabs(p2_position.x - item_position.x) - ((PADDLE_SIZE.x + BALL_SIZE.x) / 2.0f);
	float y2_dist = fabs(p2_position.y - item_position.y) - ((PADDLE_SIZE.y + BALL_SIZE.y) / 2.0f);

	if (item_position.y + BALL_SIZE.y / 2.0f > 3.75f) {
		item_movement.y = -fabs(item_movement.y);
	}

	if (item_position.y - BALL_SIZE.y / 2.0f < -3.75f) {
		item_movement.y = fabs(item_movement.y);
	}

	if (x1_dist <= 0.0f && y1_dist <= 0.0f) {
		if (item_movement.x < 0) ball_speed += 0.2f;
		item_movement = glm::vec3(0.5f, item_position.y - p1_position.y, 0);
	}
	if (x2_dist <= 0.0f && y2_dist <= 0.0f) {
		if (item_movement.x > 0) ball_speed += 0.2f;
		item_movement = glm::vec3(-0.5f, item_position.y - p2_position.y, 0);
	}

	if (item_position.x < -6.0f || item_position.x > 6.0f) {
		if (item_position.x > 0) {
			point_gravity.x  = -3 * ball_speed;
			point_movement.x =  2 * ball_speed;
			point_position.x = -6.0f;
			p1Point = true;
			item_position.x  = -0.125f;
		} else {
			point_gravity.x  =  3 * ball_speed;
			point_movement.x = -2 * ball_speed;
			point_position.x = 6.0f;
			p1Point = false;
			item_position.x  =  0.125f;
		}
		ball_speed = 0.0;

	}

	player_speed = ball_speed * 3.25f / 4.0f;

	p1Matrix = glm::translate(p1Matrix, p1_position);
	p1Matrix = glm::scale(p1Matrix, PADDLE_SIZE);

	p2Matrix = glm::translate(p2Matrix, p2_position);
	p2Matrix = glm::scale(p2Matrix, PADDLE_SIZE);

	itemMatrix = glm::translate(itemMatrix, item_position);
	itemMatrix = glm::scale(itemMatrix, BALL_SIZE);

	netMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.025f, 12.0f, 1.0f));
	pointMatrix = glm::translate(pointMatrix, point_position);
	if (p1Point) pointMatrix = glm::rotate(pointMatrix, -1.57079632679f, glm::vec3(0.0f, 0.0f, 1.0f));
	else pointMatrix = glm::rotate(pointMatrix, 1.57079632679f, glm::vec3(0.0f, 0.0f, 1.0f));
	pointMatrix = glm::scale(pointMatrix, 1.5f * glm::vec3(608.0f / 160.0f, 1.0f, 1.0f));
	// p1Matrix = glm::rotate(p1Matrix, -p1_position.x, glm::vec3(0.0f, 0.0f, 1.0f));
}

void Render()
{
	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
	float texCoords[] = { 0.0,  1.0, 1.0,  1.0, 1.0, 0.0,  0.0,  1.0, 1.0, 0.0,  0.0, 0.0 };

	glClear(GL_COLOR_BUFFER_BIT);

	glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program.positionAttribute);

	glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program.texCoordAttribute);

	program.SetModelMatrix(p1Matrix);
	glBindTexture(GL_TEXTURE_2D, whiteTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(p2Matrix);
	glBindTexture(GL_TEXTURE_2D, whiteTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(itemMatrix);
	glBindTexture(GL_TEXTURE_2D, whiteTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(netMatrix);
	glBindTexture(GL_TEXTURE_2D, whiteTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	program.SetModelMatrix(pointMatrix);
	glBindTexture(GL_TEXTURE_2D, pointTextureID);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program.positionAttribute);
	glDisableVertexAttribArray(program.texCoordAttribute);

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
