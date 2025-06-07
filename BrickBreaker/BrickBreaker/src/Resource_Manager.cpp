#include "Resource_Manager.h"

#include <iostream>
#include <sstream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

// Instantiate static variables
std::map<std::string, Shader> ResourceManager::Shaders;

std::map<std::string, Texture2D> ResourceManager::Textures;

Shader ResourceManager::LoadShader(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile, std::string name)
{
	Shaders[name] = loadShaderFromFile(vShaderFile, fShaderFile, gShaderFile);
	return Shaders[name];
}

Shader ResourceManager::GetShader(std::string name)
{
	return Shaders[name];
}

Texture2D ResourceManager::LoadTexture(const char *file, bool alpha, std::string name)
{
	Textures[name] = loadTextureFromFile(file, alpha);
	return Textures[name];
}

Texture2D ResourceManager::GetTexture(std::string name)
{
	return Textures[name];
}

void ResourceManager::Clear()
{
	// Properly delete all shaders
	for (auto iter : Shaders)
		glDeleteProgram(iter.second.ID);
	// Properly delete all textures
	for (auto iter : Textures)
		glDeleteTextures(1, &iter.second.ID);
}

ResourceManager::ResourceManager()
{

}

Shader ResourceManager::loadShaderFromFile(const char *vShaderFile, const char *fShaderFile, const char *gShaderFile /*= nullptr*/)
{
	// 1. Retrieve the vertex/fragment source code from filePath
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	try
	{
		// Open files
		std::ifstream vertexShaderFile(vShaderFile);
		std::ifstream fragmentShaderFile(fShaderFile);

		if (!vertexShaderFile.is_open() || !fragmentShaderFile.is_open())
		{
			throw std::runtime_error("Failed to open vertex or fragment shader file.");
		}

		std::stringstream vShaderStream, fShaderStream;
		// Read file's buffer contents into streams
		vShaderStream << vertexShaderFile.rdbuf();
		fShaderStream << fragmentShaderFile.rdbuf();
		// Close file handlers
		vertexShaderFile.close();
		fragmentShaderFile.close();
		// Convert stream into string
		vertexCode = vShaderStream.str();
		fragmentCode = fShaderStream.str();
		// If geometry shader path is present, also load a geometry shader
		if (gShaderFile != nullptr)
		{
			std::ifstream geometryShaderFile(gShaderFile);

			if (!geometryShaderFile.is_open())
			{
				throw std::runtime_error("Failed to open geometry shader file.");
			}

			std::stringstream gShaderStream;
			gShaderStream << geometryShaderFile.rdbuf();
			geometryShaderFile.close();
			geometryCode = gShaderStream.str();
		}
	}
	catch (std::exception e)
	{
		std::cerr << "ERROR::SHADER: Failed to read shader files: " << e.what() << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();
	const char* gShaderCode = geometryCode.c_str();
	// 2. Now create shader object from source code
	Shader shader;
	shader.Compile(vShaderCode, fShaderCode, gShaderFile != nullptr ? gShaderCode : nullptr);
	return shader;
}

Texture2D ResourceManager::loadTextureFromFile(const char* file, bool alpha)
{
	// Create texture object
	Texture2D texture;
	if (alpha)
	{
		texture.Internal_Format = GL_RGBA;
		texture.Image_Format = GL_RGBA;
	}
	// Load image
	int width, height, nrChannels;
	unsigned char* data = stbi_load(file, &width, &height, &nrChannels, 0);

	if (!data) {
		std::cerr << "ERROR::TEXTURE: Failed to load texture file: " << file << std::endl;
		std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
	}

	// Now generate texture
	texture.Generate(width, height, data);
	// And finally free image data
	stbi_image_free(data);
	return texture;
}

