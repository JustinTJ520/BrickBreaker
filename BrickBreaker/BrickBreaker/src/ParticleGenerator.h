#pragma once

#include <vector>

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Shader.h"
#include "Texture.h"
#include "GameObject.h"

// Represents a single particle and its state
struct Particle
{
	glm::vec2 Position, Velocity;
	glm::vec4 Color;
	float	  Life;

	Particle() : Position(0.0f), Velocity(0.0f), Color(1.0f), Life(0.0f) { }
};

// ParticleGenerator acts as a container for rendering a large number of
// particles by repeatedly spawning and updating particles and killing
// them after a given amount of time.
class ParticleGenerator
{
public:
	ParticleGenerator(Shader shader, Texture2D texture, unsigned int amount);
	// Update all particles
	void Update(float dt, GameObject& object, unsigned int newParticles, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
	// Render all particles
	void Draw();
private:
	// State
	std::vector<Particle> particles;
	unsigned int amount;
	// Render state
	Shader shader;
	Texture2D texture;
	unsigned int VAO;
	// Initialize buffer and vertex attributes
	void init();
	// Returns the first particle infex that's currently unused e.g. Life <= 0.0f or 0 if no particle is current inactive
	unsigned int firstUnusedParticle();
	// Respawns particle
	void respawnParticle(Particle& particle, GameObject& object, glm::vec2 offset = glm::vec2(0.0f, 0.0f));
};