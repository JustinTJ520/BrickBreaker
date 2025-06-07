#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Resource_Manager.h"
#include "GameObject.h"

class GameLevel
{
public:
	// Level state
	std::vector<GameObject> Bricks;
	// Constructor
	GameLevel() { }
	// Loads level from file
	void Load(const char *file, unsigned int levelWidth, unsigned int levelHeight);
	// Render level
	void Draw(SpriteRenderer &renderer);
	// Check if the level is completed (all non-solid tiles are destroyed)
	bool IsCompleted();
private:
	// Initialize level from tile data
	void init(std::vector<std::vector<unsigned int>> tileData,
		unsigned int levelWidth, unsigned int levelHeight);
};