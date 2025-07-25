#include "Game.h"
#include "Resource_Manager.h"
#include "SpriteRenderer.h"
#include "GameObject.h"
#include "BallObject.h"
#include "ParticleGenerator.h"

// Game-related save data
SpriteRenderer *Renderer;
GameObject *Player;
BallObject* Ball;
ParticleGenerator* Particles;

Game::Game(unsigned int width, unsigned int height)
	: State(GAME_ACTIVE), Keys(), Width(width), Height(height)
{

}

Game::~Game()
{
	delete Renderer;
	delete Player;
	delete Ball;
	delete Particles;
}

void Game::Init()
{
	// Load shaders
	ResourceManager::LoadShader("BrickBreaker/res/Shaders/Sprite.vs", "BrickBreaker/res/Shaders/Sprite.frag", nullptr, "sprite");
	ResourceManager::LoadShader("BrickBreaker/res/Shaders/Particle.vs", "BrickBreaker/res/Shaders/Particle.frag", nullptr, "particle");
	// Configure shaders
	glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
		static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
	ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
	ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
	ResourceManager::GetShader("particle").Use().SetInteger("sprite", 0);
	ResourceManager::GetShader("particle").SetMatrix4("projection", projection);
	// Load textures
	ResourceManager::LoadTexture("BrickBreaker/res/Textures/background.jpg", false, "background");
	ResourceManager::LoadTexture("BrickBreaker/res/Textures/awesomeface.png", true, "face");
	ResourceManager::LoadTexture("BrickBreaker/res/Textures/block.png", false, "block");
	ResourceManager::LoadTexture("BrickBreaker/res/Textures/block_solid.png", false, "block_solid");
	ResourceManager::LoadTexture("BrickBreaker/res/Textures/paddle.png", true, "paddle");
	ResourceManager::LoadTexture("BrickBreaker/res/Textures/particle.png", true, "particle");
	// Set render specific controls
	Shader myShader;
	myShader = ResourceManager::GetShader("sprite");
	Renderer = new SpriteRenderer(myShader);
	Particles = new ParticleGenerator(ResourceManager::GetShader("particle"), ResourceManager::GetTexture("particle"), 500);
	// Load levels
	GameLevel one; one.Load("BrickBreaker/res/Levels/one.lvl", this->Width, this->Height / 2);
	GameLevel two; two.Load("BrickBreaker/res/Levels/two.lvl", this->Width, this->Height / 2);
	GameLevel three; three.Load("BrickBreaker/res/Levels/three.lvl", this->Width, this->Height / 2);
	GameLevel four; four.Load("BrickBreaker/res/Levels/four.lvl", this->Width, this->Height / 2);
	this->Levels.push_back(one);
	this->Levels.push_back(two);
	this->Levels.push_back(three);
	this->Levels.push_back(four);
	this->Level = 0;
	// Configure game objects
	glm::vec2 playerPos = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("paddle"));
	glm::vec2 ballPos = playerPos + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -BALL_RADIUS * 2.0f);
	Ball = new BallObject(ballPos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("face"));
}

void Game::ProcessInput(float dt)
{
	if (this->State == GAME_ACTIVE)
	{
		float velocity = PLAYER_VELOCITY * dt;
		// Move playerboard
		if (this->Keys[GLFW_KEY_A])
		{
			if (Player->Position.x >= 0.0f)
			{
				Player->Position.x -= velocity;
				if (Ball->Stuck)
					Ball->Position.x -= velocity;
			}
		}
		if (this->Keys[GLFW_KEY_D])
		{
			if (Player->Position.x <= this->Width - Player->Size.x)
			{
				Player->Position.x += velocity;
				if (Ball->Stuck)
					Ball->Position.x += velocity;
			}
		}
		if (this->Keys[GLFW_KEY_SPACE])
			Ball->Stuck = false;
	}
}

void Game::Update(float dt)
{
	if (this->State != GAME_ACTIVE) return;

	Ball->Move(dt, this->Width);
	// Check for collisions
	this->DoCollisions();

	// Update particles
	Particles->Update(dt, *Ball, 2, glm::vec2(Ball->Radius / 2.0f));

	if (Ball->Position.y >= this->Height) // Did ball reach bottom edge?
	{
		this->ResetLevel();
		this->ResetPlayer();
	}
}

void Game::Render()
{
	// Draw background
	Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);
	// Draw level
	this->Levels[this->Level].Draw(*Renderer);
	// Draw player
	Player->Draw(*Renderer);
	// Draw particles
	Particles->Draw();
	// Draw ball
	Ball->Draw(*Renderer);
}

void Game::ResetLevel()
{
	if (this->Level == 0)
		this->Levels[0].Load("BrickBreaker/res/Levels/one.lvl", this->Width, this->Height / 2);
	else if (this->Level == 1)
		this->Levels[1].Load("BrickBreaker/res/Levels/two.lvl", this->Width, this->Height / 2);
	else if (this->Level == 2)
		this->Levels[2].Load("BrickBreaker/res/Levels/three.lvl", this->Width, this->Height / 2);
	else if (this->Level == 3)
		this->Levels[3].Load("BrickBreaker/res/Levels/four.lvl", this->Width, this->Height / 2);
}

void Game::ResetPlayer()
{
	// reset player/ball stats
	Player->Size = PLAYER_SIZE;
	Player->Position = glm::vec2(this->Width / 2.0f - PLAYER_SIZE.x / 2.0f, this->Height - PLAYER_SIZE.y);
	Ball->Reset(Player->Position + glm::vec2(PLAYER_SIZE.x / 2.0f - BALL_RADIUS, -(BALL_RADIUS * 2.0f)), INITIAL_BALL_VELOCITY);
}

bool CheckCollision(GameObject& one, GameObject& two);
Collision CheckCollision(BallObject& one, GameObject& two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
	for (GameObject& box : this->Levels[this->Level].Bricks)
	{
		if (!box.Destroyed)
		{
			Collision collision = CheckCollision(*Ball, box);
			if (std::get<0>(collision)) // If collision is true
			{
				// Destroy box if not solid
				if (!box.IsSolid)
					box.Destroyed = true;
				// Collision resolution
				Direction dir = std::get<1>(collision);
				glm::vec2 diff_vector = std::get<2>(collision);
				if (dir == LEFT || dir == RIGHT) // Horizontal collision
				{
					Ball->Velocity.x = -Ball->Velocity.x; // Reverse horizontal velocity
					// Relocate
					float penetration = Ball->Radius - std::abs(diff_vector.x);
					if (dir == LEFT)
						Ball->Position.x += penetration; // Move ball right
					else
						Ball->Position.x -= penetration; // Move ball left
				}
				else // Vertical collision
				{
					Ball->Velocity.y = -Ball->Velocity.y; // Reverse vertical velocity
					// Relocate
					float penetration = Ball->Radius - std::abs(diff_vector.y);
					if (dir == UP)
						Ball->Position.y -= penetration; // Move ball back up
					else
						Ball->Position.y += penetration; // Move ball back down
				}
			}
		}
	}
	Collision result = CheckCollision(*Ball, *Player);
	if (!Ball->Stuck && std::get<0>(result))
	{
		// Check where it hit the board, and change velocity based on where it hit the board
		float centerBoard = Player->Position.x + Player->Size.x / 2.0f;
		float distance = (Ball->Position.x + Ball->Radius) - centerBoard;
		float percentage = distance / (Player->Size.x / 2.0f);
		// Then move accordingly
		float strength = 2.0f;
		glm::vec2 oldVelocity = Ball->Velocity;
		Ball->Velocity.x = INITIAL_BALL_VELOCITY.x * percentage * strength;
		//Ball->Velocity.y = -Ball->Velocity.y;
		Ball->Velocity = glm::normalize(Ball->Velocity) * glm::length(oldVelocity);
		// Fix sticky paddle
		Ball->Velocity.y = -1.0f * abs(Ball->Velocity.y);
	}
}

bool CheckCollision(GameObject& one, GameObject& two) // AABB - AABB collision
{
	// collision x-axis?
	bool collisionX = one.Position.x + one.Size.x >= two.Position.x &&
		two.Position.x + two.Size.x >= one.Position.x;
	// collision y-axis?
	bool collisionY = one.Position.y + one.Size.y >= two.Position.y &&
		two.Position.y + two.Size.y >= one.Position.y;
	// collision only if on both axes
	return collisionX && collisionY;
}

Collision CheckCollision(BallObject& one, GameObject& two) // AABB - Circle collision
{
	// get center point circle first 
	glm::vec2 center(one.Position + one.Radius);
	// calculate AABB info (center, half-extents)
	glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
	glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
	// get difference vector between both centers
	glm::vec2 difference = center - aabb_center;
	glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
	// add clamped value to AABB_center and we get the value of box closest to circle
	glm::vec2 closest = aabb_center + clamped;
	// retrieve vector between center circle and closest point AABB and check if length <= radius
	difference = closest - center;
	
	if (glm::length(difference) < one.Radius)
		return std::make_tuple(true, VectorDirection(difference), difference);
	else
		return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

Direction VectorDirection(glm::vec2 target)
{
	glm::vec2 compass[] = {
		glm::vec2(0.0f, 1.0f), // Up
		glm::vec2(1.0f, 0.0f), // Right
		glm::vec2(0.0f, -1.0f), // Down
		glm::vec2(-1.0f, 0.0f)  // Left
	};
	float max = 0.0f;
	unsigned int best_match = -1;
	for (unsigned int i = 0; i < 4; i++)
	{
		float dot_product = glm::dot(glm::normalize(target), compass[i]);
		if (dot_product > max)
		{
			max = dot_product;
			best_match = i;
		}
	}
	return (Direction)best_match;
}