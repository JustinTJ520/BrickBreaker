#include <iostream>

#include "Game.h"
#include "Resource_Manager.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

// GLFW declarations
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);

// Width of screen
const unsigned int SCREEN_WIDTH = 800;
// Height of the screen
const unsigned int SCREEN_HEIGHT = 600;

Game Breakout(SCREEN_WIDTH, SCREEN_HEIGHT);

glm::mat4 projection = glm::ortho(0.0f, 800.0f, 600.0f, 0.0f, -1.0f, 1.0f);

int main(int argc, char* argv[])
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
	glfwWindowHint(GLFW_RESIZABLE, false);

	GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Breakout", nullptr, nullptr);
	glfwMakeContextCurrent(window);

	//glad: load all OpenGL function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetKeyCallback(window, key_callback);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// OpenGL configuration
	glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;

	ImGui::StyleColorsDark();

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init("#version 330");

	// Initialize game
	Breakout.Init();

	Breakout.State = GAME_PAUSE;

	// DeltaTime variables
	float deltaTime = 0.0f;
	float lastFrame = 0.0f;

	while (!glfwWindowShouldClose(window))
	{
		// Calculate delta time
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		glfwPollEvents();

		// Manage user input
		Breakout.ProcessInput(deltaTime);

		// Update game state
		Breakout.Update(deltaTime);

		// Render
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		Breakout.Render();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		static int selectedLevel = 0;

		if (Breakout.State == GAME_PAUSE)
		{
			ImGui::Begin("Pause Menu");

			ImGui::Text("Controls:\nA = Move Left\nD = Move Right\nSpace = Pause or Start Game\nESC = Quit Game\n");

			ImGui::Text("Currently Playing Level %d\n", Breakout.Level + 1);

			ImGui::Text("Select your level:");

			if (ImGui::Button("Level 1"))
			{
				selectedLevel = 0;
				Breakout.Level = selectedLevel;
				Breakout.ResetLevel();
				Breakout.ResetPlayer();
			}

			if (ImGui::Button("Level 2"))
			{
				selectedLevel = 1;
				Breakout.Level = selectedLevel;
				Breakout.ResetLevel();
				Breakout.ResetPlayer();
			}

			if (ImGui::Button("Level 3"))
			{
				selectedLevel = 2;
				Breakout.Level = selectedLevel;
				Breakout.ResetLevel();
				Breakout.ResetPlayer();
			}

			if (ImGui::Button("Level 4"))
			{
				selectedLevel = 3;
				Breakout.Level = selectedLevel;
				Breakout.ResetLevel();
				Breakout.ResetPlayer();
			}

			ImGui::End();
		}

		ImGui::Render();

		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		glfwSwapBuffers(window);
	}

	// Delete all resources as loaded using the resource manager
	ResourceManager::Clear();

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode)
{
	// When a user presses the escape key, we set the WindowShouldClose property to true, closing the application
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (key >= 0 && key < 1024)
	{
		if (action == GLFW_PRESS)
			Breakout.Keys[key] = true;
		else if (action == GLFW_RELEASE)
			Breakout.Keys[key] = false;
	}
	if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
	{
		if (Breakout.State == GAME_ACTIVE)
			Breakout.State = GAME_PAUSE;
		else Breakout.State = GAME_ACTIVE;
	}
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// Make sure the viewport matches the new window dimensions; Note that width and
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}