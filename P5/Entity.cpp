#include "Entity.h"

Entity::Entity()
{
	position = glm::vec3(0);
	movement = glm::vec3(0);
	velocity = glm::vec3(0);
	acceleration = glm::vec3(0);
	speed = 0;

	modelMatrix = glm::mat4(1.0f);
}

bool Entity::CheckCollision(Entity* other)
{
	if (other == NULL) return false;
	if (other == this) return false;
	if (!isActive || !other->isActive) return false;

	float xdist = fabs(position.x - other->position.x) - ((width + other->width) / 2.0f);
	float ydist = fabs(position.y - other->position.y) - ((height + other->height) / 2.0f);

	if (xdist < 0 && ydist < 0) {
		lastCollision = other;
		return true;
	}

	return false;
}



void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
	collidedTop = false;
	collidedBottom = false;

	for (int i = 0; i < objectCount; i++) {
		Entity* object = &objects[i];
		float ydist = fabs(position.y - object->position.y);
		float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
		if (CheckCollision(object)) {
			if (velocity.y > 0) {
				collidedTop = true;
			}
			else if (velocity.y < 0) {
				collidedBottom = true;
			}
			switch (lastCollision->entityType) {
			case WALL: case START:
				if (collidedTop) {
					position.y -= penetrationY;
					velocity.y = 0;
				}
				else if (collidedBottom) {
					position.y += penetrationY;
					velocity.y = 0;
				}
				break;
			case ENEMY:
				if (collidedTop) {
					position.y -= penetrationY;
					velocity.y = 0;
					isActive = false;
				}
				else if (collidedBottom) {
					position.y += penetrationY;
					velocity.y = -velocity.y;
					lastCollision->isActive = false;
				}
				break;
			}
		}

	}
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
	collidedLeft = false;
	collidedRight = false;

	for (int i = 0; i < objectCount; i++) {
		Entity* object = &objects[i];
		float xdist = fabs(position.x - object->position.x);
		float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
		if (CheckCollision(object)) {
			if (velocity.x > 0) {

				collidedRight = true;
			}
			else if (velocity.x < 0) {

				collidedLeft = true;
			}

			switch (lastCollision->entityType) {
			case WALL: case START:
				if (collidedRight) {
					position.x -= penetrationX;
					velocity.x = 0;
				}
				else if (collidedLeft) {
					position.x += penetrationX;
					velocity.x = 0;
				}
				break;
			case ENEMY:
				isActive = false;
				if (collidedRight) {
					position.x -= penetrationX;
					velocity.x = 0;
				}
				else if (collidedLeft) {
					position.x += penetrationX;
					velocity.x = 0;
				}
				break;
			}

		}
	}
}

void Entity::CheckCollisionsY(Map* map)
{
	// Probes for tiles
	glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
	glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
	glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);

	glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
	glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
	glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);

	float penetration_x = 0;
	float penetration_y = 0;

	if (map->IsSolid(top, &penetration_x, &penetration_y) && velocity.y > 0) {
		position.y -= penetration_y;
		velocity.y = 0;
		collidedTop = true;
	}
	else if (map->IsSolid(top_left, &penetration_x, &penetration_y) && velocity.y > 0) {
		position.y -= penetration_y;
		velocity.y = 0;
		collidedTop = true;
	}
	else if (map->IsSolid(top_right, &penetration_x, &penetration_y) && velocity.y > 0) {
		position.y -= penetration_y;
		velocity.y = 0;
		collidedTop = true;
	}
	if (map->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y < 0) {
		position.y += penetration_y;
		velocity.y = 0;
		collidedBottom = true;
	}
	else if (map->IsSolid(bottom_left, &penetration_x, &penetration_y) && velocity.y < 0) {
		position.y += penetration_y;
		velocity.y = 0;
		collidedBottom = true;
	}
	else if (map->IsSolid(bottom_right, &penetration_x, &penetration_y) && velocity.y < 0) {
		position.y += penetration_y;
		velocity.y = 0;
		collidedBottom = true;
	}
}

void Entity::CheckCollisionsX(Map* map)
{
	// Probes for tiles
	glm::vec3 left = glm::vec3(position.x - (width / 2), position.y, position.z);
	glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);
	float penetration_x = 0;
	float penetration_y = 0;
	if (map->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
		position.x += penetration_x;
		velocity.x = 0;
		collidedLeft = true;
	}
	if (map->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
		position.x -= penetration_x;
		velocity.x = 0;
		collidedRight = true;
	}
}

void Entity::AIWalker()
{
	switch (aiState) {
	case IDLE:
		aiState = WALKING;
		velocity.x = -1;

		break;
	case WALKING:
		if (lastCollision == NULL) return;
		if (lastCollision->entityType == WALL) {
			if (collidedLeft) {
				velocity.x = 1;
			}
			if (collidedRight) {
				velocity.x = -1;
			}
		}
		break;
	}
}

void Entity::AIBouncer()
{
	switch (aiState) {
	case IDLE:
		aiState = WALKING;
		movement = glm::vec3(-1, -1, 0);
		break;
	case WALKING:
		if (lastCollision == NULL) return;
		if (lastCollision->entityType == WALL) {

			if (collidedLeft) {
				movement.x = 1;
			}
			if (collidedRight) {
				movement.x = -1;
			}
			if (collidedTop) {
				movement.y = -1;
			}
			if (collidedBottom) {
				movement.y = 1;
			}
		}
		break;
	}
}

void Entity::AIUnga(Entity* player)
{
	if (player == NULL) return;
	switch (aiState) {
	case IDLE:
		if (player->isActive) {
			aiState = ATTACKING;
		}
		break;
	case ATTACKING:
		if (!player->isActive) aiState = IDLE;
		movement = player->position - position;
		movement = glm::normalize(movement);
		break;
	}
}

void Entity::AI(Entity* player)
{
	switch (aiType) {
	case WALKER:
		AIWalker();
		break;

	case BOUNCER:
		AIBouncer();
		break;

	case UNGA:
		AIUnga(player);
		break;
	}
}

void Entity::Update(float deltaTime, Entity* objects, int objectCount, Map* map, Entity* player)
{
	collidedTop = false;
	collidedBottom = false;
	collidedLeft = false;
	collidedRight = false;

	if (!isActive) return;


	if (animIndices != NULL) {
	//	if (glm::length(movement) != 0) {
			animTime += deltaTime;

			if (animTime >= 0.0166f) {
				animTime = 0.0f;
				animIndex++;
				if (animIndex >= animFrames) {
					animIndex = 0;
				}
			}
	//	}
	//	else {
	//		animIndex = 0;
	//	}
	}

	if (jump) {
		jump = false;

		velocity.y += jumpPower;
	}

	switch (entityType) {
	case PLAYER:
		velocity.y += movement.y * deltaTime;
		velocity.x += movement.x * speed * deltaTime;

		velocity += acceleration * deltaTime;

		position.y += velocity.y * deltaTime; // Move on Y
		CheckCollisionsY(objects, objectCount);// Fix if needed
		CheckCollisionsY(map);

		position.x += velocity.x * deltaTime; // Move on X
		CheckCollisionsX(objects, objectCount);// Fix if needed
		CheckCollisionsX(map);

		break;
	case ENEMY:
		velocity.y += movement.y * speed * deltaTime;
		velocity.x += movement.x * speed * deltaTime;

		velocity += acceleration * deltaTime;

		position.y += velocity.y * deltaTime; // Move on Y
		// CheckCollisionsY(objects, objectCount);// Fix if needed

		position.x += velocity.x * deltaTime; // Move on X
		// CheckCollisionsX(objects, objectCount);// Fix if needed

		AI(player);
		break;
	}




	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
}

void Entity::DrawSpriteFromTextureAtlas(ShaderProgram* program, GLuint textureID, int index)
{
	float u = (float)(index % animCols) / (float)animCols;
	float v = (float)(index / animCols) / (float)animRows;

	float width = 1.0f / (float)animCols;
	float height = 1.0f / (float)animRows;

	float texCoords[] = { u, v + height, u + width, v + height, u + width, v,
						  u, v + height, u + width, v, u, v
	};

	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };

	glBindTexture(GL_TEXTURE_2D, textureID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}

void Entity::Render(ShaderProgram* program)
{
	if (!isActive) return;

	program->SetModelMatrix(modelMatrix);

	if (animIndices != NULL) {
		DrawSpriteFromTextureAtlas(program, textureID, animIndices[animIndex]);
		return;
	}

	float vertices[] = { -0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5 };
	float texCoords[] = { 0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0 };

	glBindTexture(GL_TEXTURE_2D, textureID);

	glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices);
	glEnableVertexAttribArray(program->positionAttribute);

	glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
	glEnableVertexAttribArray(program->texCoordAttribute);

	glDrawArrays(GL_TRIANGLES, 0, 6);

	glDisableVertexAttribArray(program->positionAttribute);
	glDisableVertexAttribArray(program->texCoordAttribute);
}
