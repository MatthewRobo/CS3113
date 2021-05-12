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
		if (lastCollision->entityType == ENEMY) isActive = false;
		return true;
	}

	return false;
}

void Entity::CheckCollisionsY(Entity* objects, int objectCount)
{
	for (int i = 0; i < objectCount; i++)
	{
		Entity* object = &objects[i];
		if (CheckCollision(object))
		{
			float ydist = fabs(position.y - object->position.y);
			float penetrationY = fabs(ydist - (height / 2.0f) - (object->height / 2.0f));
			if (velocity.y > 0) {
				position.y -= penetrationY;
				velocity.y = 0;
				collidedTop = true;
			}
			else if (velocity.y < 0) {
				position.y += penetrationY;
				velocity.y = 0;
				collidedBottom = true;
			}
		}
	}
}

void Entity::CheckCollisionsX(Entity* objects, int objectCount)
{
	for (int i = 0; i < objectCount; i++)
	{
		Entity* object = &objects[i];
		if (CheckCollision(object))
		{
			float xdist = fabs(position.x - object->position.x);
			float penetrationX = fabs(xdist - (width / 2.0f) - (object->width / 2.0f));
			if (velocity.x > 0) {
				position.x -= penetrationX;
				if (object->entityType == WALL)
					velocity.x = -abs(velocity.x);
				else
					velocity.x = 0;
				collidedRight = true;
			}
			else if (velocity.x < 0) {
				position.x += penetrationX;
				if (object->entityType == WALL)
					velocity.x = abs(velocity.x);
				else
					velocity.x = 0;
				collidedLeft = true;
			}
		}
	}
}

void Entity::CheckCollisionsY(Map** map)
{
	// Probes for tiles
	glm::vec3 top = glm::vec3(position.x, position.y + (height / 2), position.z);
	glm::vec3 top_left = glm::vec3(position.x - (width / 2), position.y + (height / 2), position.z);
	glm::vec3 top_right = glm::vec3(position.x + (width / 2), position.y + (height / 2), position.z);

	glm::vec3 bottom = glm::vec3(position.x, position.y - (height / 2), position.z);
	glm::vec3 bottom_left = glm::vec3(position.x - (width / 2), position.y - (height / 2), position.z);
	glm::vec3 bottom_right = glm::vec3(position.x + (width / 2), position.y - (height / 2), position.z);
	for (int i = 0; i < 3; i++)
	{
		float penetration_x = 0;
		float penetration_y = 0;

		if (map[i]->IsSolid(top, &penetration_x, &penetration_y) && velocity.y > 0) {
			position.y -= penetration_y;
			velocity.y = i == 1 ? -abs(velocity.y) : 0;
			collidedTop = true;
		}
		/*
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
		*/
		if (map[i]->IsSolid(bottom, &penetration_x, &penetration_y) && velocity.y < 0) {
			position.y += penetration_y;
			velocity.y = i == 1 && velocity.x != 0 ? abs(velocity.y) : 0;
			collidedBottom = true;

			if (entityType == PLAYER && i != 1) {
				velocity.x = 0;
			}
		}
		/*
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
		*/
	}
}
void Entity::CheckCollisionsX(Map** map)
{
	// Probes for tiles
	glm::vec3 left  = glm::vec3(position.x - (width / 2), position.y, position.z);
	glm::vec3 right = glm::vec3(position.x + (width / 2), position.y, position.z);
	for (int i = 0; i < 3; i++)
	{
		float penetration_x = 0;
		float penetration_y = 0;

		if (map[i]->IsSolid(left, &penetration_x, &penetration_y) && velocity.x < 0) {
			position.x += penetration_x;
			velocity.x = i == 0 ? abs(velocity.x) : 0 ;
			collidedLeft = true;
		}

		if (map[i]->IsSolid(right, &penetration_x, &penetration_y) && velocity.x > 0) {
			position.x -= penetration_x;
			velocity.x = i == 0 ? -abs(velocity.x) : 0;
			collidedRight = true;
		}
	}
}

void Entity::AIWalker()
{
	switch (aiState) {
	case IDLE:
		aiState = WALKING;
		velocity.x = -speed;

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

void Entity::Update(float deltaTime, Entity* objects, int objectCount, Map** map, Entity* player)
{
	if (!isActive) return;
	collidedTop = false;
	collidedBottom = false;
	collidedLeft = false;
	collidedRight = false;

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


	switch (entityType) {
	case PLAYER:
		if (jump) {

			jump = false;
			jumpCharge = false;

//			position.x += movement.x * deltaTime;

			velocity.x = jumpPower * 0.50000000000f * movement.x;
			velocity.y = jumpPower * 0.86602540378f;

			jumpPower = 0;
		}

		velocity += acceleration * deltaTime;

		position.y += velocity.y * deltaTime; // Move on Y
		CheckCollisionsY(map);
		CheckCollisionsY(objects, objectCount);// Fix if needed

		position.x += velocity.x * deltaTime; // Move on X
		CheckCollisionsX(map);
		CheckCollisionsX(objects, objectCount);// Fix if needed

		if (!collidedBottom) {
			jumpPower = 0;
			jumpCharge = false;
			if (movement.x < 0) animIndices = anim7;
			if (movement.x > 0) animIndices = anim9;
		}
		else
		{
			if (movement.x < 0) animIndices = anim4;
			if (movement.x > 0) animIndices = anim6;

			if (jumpCharge) {
				if (movement.x < 0) animIndices = anim1;
				if (movement.x > 0) animIndices = anim3;

				if (jumpPower < jumpMin) jumpPower = jumpMin;
				else if ((jumpPower) < jumpMax) jumpPower += jumpSlope * deltaTime;
				else jumpPower = jumpMax;
			}
		}

		break;
	case ENEMY:
		velocity.y += movement.y * speed * deltaTime;
		velocity.x += movement.x * speed * deltaTime;

		velocity += acceleration * deltaTime;


		position.y += velocity.y * deltaTime; // Move on Y
		CheckCollisionsY(map);
		CheckCollisionsY(objects, objectCount);// Fix if needed

		position.x += velocity.x * deltaTime; // Move on X
		CheckCollisionsX(map);
		CheckCollisionsX(objects, objectCount);// Fix if needed


		AI(player);
		break;
	case TRAIL:
		position = player->position;
		animIndices = player->animIndices;
		break;
	case WALL:
		position = position;
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
