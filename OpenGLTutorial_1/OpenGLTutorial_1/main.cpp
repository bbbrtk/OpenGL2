#define _CRT_SECURE_NO_DEPRECATE

// libs
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include "stdio.h"
#include <iostream>
#include <stdlib.h>
#include <ctime>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "objectsCoords.h"

// classes
#include "Shader.h"
#include "Camera.h"


// functions inits
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);

void DrawBuilding2(unsigned int textureWall2_fb, unsigned int VAO, unsigned int textureWall2_rl, unsigned int textureWall2_tb);

void DrawBuilding3(unsigned int textureWall3_fb, unsigned int VAO, unsigned int textureWall3_rl, unsigned int textureWall3_tb);

void DeleteVAOVBO(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO, unsigned int &skyboxVAO, unsigned int &VAO2, unsigned int &VAO3, unsigned int &VAO4, unsigned int &lightVAO);

void DrawBuliding1(unsigned int textureWall1_fb, unsigned int VAO, unsigned int textureWall1_rl, unsigned int textureWall1_tb);

void DrawBuliding4(unsigned int textureWall4_fb, unsigned int VAO, unsigned int textureWall4_rl, unsigned int textureWall4_tb);


/*
SETTINGS
screen param
camera class and camera movement param
timing
lighting
*/
const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 900;

Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

float deltaTime = 0.0f;	// time between current frame and last frame
float lastFrame = 0.0f;

glm::vec3 lightPos(-1.0f, 7.0f, -1.0f);

/*
GLFW
initialize
configure
*/
void initGLFW() {
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
}


/*
WINDOW
join functions/callbacks to window:
1. framebuffer_size_callback -- on every window resizing
2. mouse_callback -- on every mouse moving
3. scroll_callback -- one every scrolling
-- it doesn't work without joining
tell GLFW to capture mouse
*/
void windowObjectConfigure(GLFWwindow* window) {
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}


/*
GLAD
manages function pointers for OpenGL,
so initialize GLAD before calling any OpenGL function (but not GLFW!)
*/
int initGLAD() {
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}
}


/*
TEXTURE
load and create a texture
*/
unsigned int loadTexture(const char * textureName) {

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char *data = stbi_load(textureName, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << textureName << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}


/*
TEXTURE 3D
load and create a 3D texture
*/
unsigned int loadCubemap(std::vector<std::string> faces)
{
	unsigned int textureID;
	glActiveTexture(GL_TEXTURE0);
	glEnable(GL_TEXTURE_CUBE_MAP);
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}


/*
generate cubes
choose position on Z-axis, X-axis on area
take random number as height of the building (Y-axis)
create cube and pass it on to the vector of vec3
*/
void generateCity(std::vector<glm::vec4>* cubePositions, int sizeOfCity) {
	for (int k = 0; k < sizeOfCity; k++) { // z
		if (k % 2 == 0) {
			for (int j = 0; j < sizeOfCity; j++) {  // x
				if (j % 2 == 0) {
					int height = 4;
					int lowest = 2;
					int buildingHeight = std::rand() % height + lowest; // <lowest, lowest+height>
					float texture = 0.0f;
					if (buildingHeight % height == 1) {
						texture = 1.0f;
					}
					else if (buildingHeight % height == 2) {
						texture = 2.0f;
					}
					else if (buildingHeight % height == 3) {
						texture = 3.0f;
					}
					else {
						texture = 0.0f;
					}
					for (int i = 0; i < buildingHeight; i++) {  // y
						cubePositions->push_back(glm::vec4((float)j, (float)i, (float)k, texture));
					}
				}
			}
		}
	}
}


/*
generate flat street crossings
choose position on Z-axis, X-axis on area
Y-axis always 0 (ground level)
create square and pass it on to the vector of vec3
*/
void generateCrossings(std::vector<glm::vec3>* streetPositions, int sizeOfCity) {
	for (int k = 0; k < sizeOfCity; k++) { // z
		if (k % 2 != 0) {
			for (int j = 0; j < sizeOfCity; j++) {  // x
				if (j % 2 != 0) {
					streetPositions->push_back(glm::vec3((float)j, 0.0f, (float)k));
				}
			}
		}
	}
}


/*
generate flat street
choose position on Z-axis, X-axis on area
Y-axis always 0 (ground level)
create square and pass it on to the vector of vec3
*/
void generateStreet(std::vector<glm::vec3>* streetPositions, int sizeOfCity) {
	for (int k = 0; k < sizeOfCity; k++) { // z
		if (k % 2 == 0) {
			for (int j = 0; j < sizeOfCity; j++) {  // x
				if (j % 2 != 0) {
					streetPositions->push_back(glm::vec3((float)j, 0.0f, (float)k));
				}
			}
		}
	}
}


/*
generate flat street
choose position on Z-axis, X-axis on area
Y-axis always 0 (ground level)
create square and pass it on to the vector of vec3
*/
void generateStreet2(std::vector<glm::vec3>* streetPositions, int sizeOfCity) {
	for (int k = 0; k < sizeOfCity; k++) { // z
		if (k % 2 != 0) {
			for (int j = 0; j < sizeOfCity; j++) {  // x
				if (j % 2 == 0) {
					streetPositions->push_back(glm::vec3((float)j, 0.0f, (float)k));
				}
			}
		}
	}
}


/*
VBO:	create / bind and select type / configure
VAO:	create / bind (connects to VBO)
EBO:	create / bind (connects to VAO and VBO) / configure (for using indices)
*/
void configureVAO_VBO_EBO(unsigned int * VAO, unsigned int * VBO, unsigned int * EBO) {
	// always add '*' before each parameter
	glGenVertexArrays(1, *&VAO);  // xd
	glGenBuffers(1, *&VBO);
	//glGenBuffers(1, &EBO);

	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize3, verticesTab3, GL_STATIC_DRAW);
	glBindVertexArray(*VAO);

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}


/*
VAO - vertex array object creation
*/
void configureAnotherVAO(unsigned int * VAO, const void * verticesTab, size_t verticesSize) {
	// always add '*' before each parameter
	glGenVertexArrays(1, *&VAO);  // xd
	glBindVertexArray(*VAO);
	glBufferData(GL_ARRAY_BUFFER, verticesSize, verticesTab, GL_STATIC_DRAW);
}


/*
clear and delete VAO VBO EBO
*/
void cleanVAO_VBO_EBO(unsigned int * VAO, unsigned int * VBO, unsigned int * EBO) {
	glDeleteVertexArrays(1, *&VAO);
	glDeleteBuffers(1, *&VBO);
	//glDeleteBuffers(1, &EBO);
}


/*
set how OpenGL should interpret objectCoords.cpp data
*/
void howInterpretVertexData(GLuint sizeOfRow, GLuint vertexCoordsNumber,
	GLuint normalCoordsNumber, GLuint textureCoordsNumber) {
	// position attribute:
	// 0 - attribute location, 3 - number of vertex coords, 5* - size of row
	//glVertexAttribPointer(0, vertexCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)0);
	glVertexAttribPointer(0, vertexCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// normal attribute
	// 1 - attribute location, 3 - number of vertex coords, 8* - size of row
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	//glEnableVertexAttribArray(1);

	// texture coord attribute
	// 1 - attribute location, 2 - number of vertex coords, 5* - size of row, 3* - offset
	//glVertexAttribPointer(1, textureCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(1, textureCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
}



/*
set how OpenGL should interpret objectCoords.cpp data
*/
void howInterpretVertexData(GLuint sizeOfRow, GLuint vertexCoordsNumber,
	GLuint normalCoordsNumber, GLuint textureCoordsNumber,
	GLuint vertexOffset, GLuint normalOffset, GLuint textureOffset) {
	// position attribute:
	// 0 - attribute location, 3 - number of vertex coords, 5* - size of row
	//glVertexAttribPointer(0, vertexCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)0);
	glVertexAttribPointer(0, vertexCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)vertexOffset);
	glEnableVertexAttribArray(0);

	// normal attribute
	// 1 - attribute location, 3 - number of vertex coords, 8* - size of row
	glVertexAttribPointer(1, normalCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)(normalOffset * sizeof(float)));
	glEnableVertexAttribArray(1);

	// texture coord attribute
	// 1 - attribute location, 2 - number of vertex coords, 5* - size of row, 3* - offset
	//glVertexAttribPointer(1, textureCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)(3 * sizeof(float)));
	glVertexAttribPointer(2, textureCoordsNumber, GL_FLOAT, GL_FALSE, sizeOfRow * sizeof(float), (void*)(textureOffset * sizeof(float)));
	glEnableVertexAttribArray(2);
}

std::vector <glm::vec4> roofsPositions;

void GetRoofsPositions(std::vector <glm::vec4> cubePositions)
{
	std::vector <glm::vec4> temp;
	for (unsigned int i = 0; i < cubePositions.size(); i++)//copy all cubes positions and adds 1 to height
	{
		temp.push_back(glm::vec4(cubePositions[i].x - 0.5, cubePositions[i].y + 1, cubePositions[i].z - 0.5, cubePositions[i].w));
	}

	for (unsigned int i = 0; i < temp.size(); i++)
	{
		for (unsigned int j = 0; j < temp.size(); j++)
		{
			if (temp[i].x == temp[j].x && temp[i].z == temp[j].z && (temp[i].y > temp[j].y))
			{
				temp[j].x = -1;
			}
		}
	}

	for (unsigned int i = 0; i < temp.size(); i++)
	{
		if (temp[i].x > -1)
			roofsPositions.push_back(temp[i]);
	}
}


void checkColisions()
{
	float blockSize = 1;
	glm::vec3 pos = camera.Position;
	//moving compared piont forward
	float alpha = atan(camera.Front.z / camera.Front.x);
	float x = cos(alpha)*camera.cameraDistance;
	float z = sin(alpha)*camera.cameraDistance;
	//pos.x += x;
	//pos.z += z;



	float tolerance = 0.25;

	for (unsigned int i = 0; i < roofsPositions.size(); i++)
	{
		glm::vec4 block = roofsPositions[i];
		float blockHeight = roofsPositions[i].y;
		bool isOnTheRoof = false;
		//	blockHeight += camera.characterHeight;
		//if character is inside a block
		if (((block.x + blockSize) > pos.x) && (block.x < pos.x) && ((block.z + blockSize) > pos.z) && (block.z < pos.z))
		{
			if (pos.y > blockHeight + tolerance) //character is over te block
			{
				camera.isOnTheRoof = false;
				camera.nearestRoofYPosition = blockHeight;
				break;
			}
			else if (pos.y > blockHeight - tolerance)//character is on the roof
			{
				camera.isOnTheRoof = isOnTheRoof = true;
				camera.jumpHeight = 0;
				break;
			}
			else //character is inside a block - restart the game
			{
				camera.SetUpCharacterMovementParameters();
				break;
			}
		}
		if (!isOnTheRoof)
		{
			camera.isOnTheRoof = false;
			camera.nearestRoofYPosition = 0;
		}


	}
}

void DrawBuliding4(unsigned int textureWall4_fb, unsigned int VAO, unsigned int textureWall4_rl, unsigned int textureWall4_tb)
{
	// back
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall4_fb);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// front
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall4_fb);
	glDrawArrays(GL_TRIANGLES, 6, 6);

	// left
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall4_rl);
	glDrawArrays(GL_TRIANGLES, 12, 6);

	// right
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall4_rl);
	glDrawArrays(GL_TRIANGLES, 18, 6);

	// bottom
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall4_tb);
	glDrawArrays(GL_TRIANGLES, 24, 6);

	// top
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall4_tb);
	glDrawArrays(GL_TRIANGLES, 30, 6);
}

void DrawBuliding1(unsigned int textureWall1_fb, unsigned int VAO, unsigned int textureWall1_rl, unsigned int textureWall1_tb)
{
	// back
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall1_fb);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// front
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall1_fb);
	glDrawArrays(GL_TRIANGLES, 6, 6);

	// left
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall1_rl);
	glDrawArrays(GL_TRIANGLES, 12, 6);

	// right
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall1_rl);
	glDrawArrays(GL_TRIANGLES, 18, 6);

	// bottom
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall1_tb);
	glDrawArrays(GL_TRIANGLES, 24, 6);

	// top
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall1_tb);
	glDrawArrays(GL_TRIANGLES, 30, 6);
}

void DrawBuilding3(unsigned int textureWall3_fb, unsigned int VAO, unsigned int textureWall3_rl, unsigned int textureWall3_tb)
{
	// back
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall3_fb);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// front
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall3_fb);
	glDrawArrays(GL_TRIANGLES, 6, 6);

	// left
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall3_rl);
	glDrawArrays(GL_TRIANGLES, 12, 6);

	// right
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall3_rl);
	glDrawArrays(GL_TRIANGLES, 18, 6);

	// bottom
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall3_tb);
	glDrawArrays(GL_TRIANGLES, 24, 6);

	// top
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall3_tb);
	glDrawArrays(GL_TRIANGLES, 30, 6);
}

void DrawBuilding2(unsigned int textureWall2_fb, unsigned int VAO, unsigned int textureWall2_rl, unsigned int textureWall2_tb)
{
	// back
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall2_fb);
	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 6);

	// front
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall2_fb);
	glDrawArrays(GL_TRIANGLES, 6, 6);

	// left
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall2_rl);
	glDrawArrays(GL_TRIANGLES, 12, 6);

	// right
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall2_rl);
	glDrawArrays(GL_TRIANGLES, 18, 6);

	// bottom
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall2_tb);
	glDrawArrays(GL_TRIANGLES, 24, 6);

	// top
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureWall2_tb);
	glDrawArrays(GL_TRIANGLES, 30, 6);
}

void DeleteVAOVBO(unsigned int &VAO, unsigned int &VBO, unsigned int &EBO, unsigned int &skyboxVAO, unsigned int &VAO2, unsigned int &VAO3, unsigned int &VAO4, unsigned int &lightVAO)
{
	cleanVAO_VBO_EBO(&VAO, &VBO, &EBO);
	glDeleteVertexArrays(1, &skyboxVAO);
	glDeleteBuffers(1, &skyboxVAO);
	glDeleteVertexArrays(1, &VAO2);
	glDeleteVertexArrays(1, &VAO3);
	glDeleteVertexArrays(1, &VAO4);
	glDeleteVertexArrays(1, &lightVAO);
}

/*
_________________________________________________________
_________________________________________________________
_________________________________________________________
*/
int main() {

	// init GLFW lib
	initGLFW();

	// make the context of our window the main context on the current thread 
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "project_1", NULL, NULL);
	if (window == NULL) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
	}

	// init window functions
	windowObjectConfigure(window);

	// init GLAD lib
	initGLAD();

	// color output 
	std::system("COLOR 02");

	// enable z-buffer
	glEnable(GL_DEPTH_TEST);

	// create shader object
	Shader ourShader("vertexshader.vs", "fragmentshader.fs");
	Shader lampShader("lamp.vs", "lamp.fs");
	Shader lightingShader("lighting_maps.vs", "lighting_maps.fs");
	Shader skyboxShader("skybox.vs", "skybox.fs");

	// vectors for models
	srand(time(NULL));
	std::vector <glm::vec4> cubePositions;  // !!!
	std::vector <glm::vec3> crossingPositions;
	std::vector <glm::vec3> streetPositions;
	std::vector <glm::vec3> street2Positions;

	// set size and generate City
	int sizeOfCity = camera.sizeOfCity;
	generateCity(&cubePositions, sizeOfCity);
	generateCrossings(&crossingPositions, sizeOfCity);
	generateStreet(&streetPositions, sizeOfCity);
	generateStreet2(&street2Positions, sizeOfCity);

	GetRoofsPositions(cubePositions);

	// building-cube VAO, main VBO, EBO for indices (not used)
	unsigned int VBO, VAO, VAO2, VAO3, VAO4, EBO;
	configureVAO_VBO_EBO(&VAO, &VBO, &EBO);
	howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);

	// crossing, streets
	configureAnotherVAO(&VAO2, verticesTab2, verticesSize2);
	howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);
	configureAnotherVAO(&VAO3, verticesTab2, verticesSize2);
	howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);
	configureAnotherVAO(&VAO4, verticesTab2, verticesSize2);
	howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);

	// light cube VAO
	unsigned int lightVAO;
	glGenVertexArrays(1, &lightVAO);
	glBindVertexArray(lightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// skybox VAO
	unsigned int skyboxVAO, skyboxVBO;
	configureAnotherVAO(&skyboxVAO, skyboxVertices, skyboxVerticesSize);
	howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);

	// load texture
	unsigned int textureCrossing, textureStreet, textureStreet2;
	unsigned int textureWall1_rl, textureWall1_fb, textureWall1_tb;
	unsigned int textureWall2_rl, textureWall2_fb, textureWall2_tb;
	unsigned int textureWall3_rl, textureWall3_fb, textureWall3_tb;
	unsigned int textureWall4_rl, textureWall4_fb, textureWall4_tb;

	// crossing and street
	textureCrossing = loadTexture("textures/crossingL7.jpg");
	textureStreet = loadTexture("textures/streetL1.jpg"); // vertical
	textureStreet2 = loadTexture("textures/streetL2.jpg"); // horizontal
														   // level 1 building
	textureWall1_fb = loadTexture("textures/level1/wall1_1.jpg"); // front + back
	textureWall1_rl = loadTexture("textures/level1/wall1_2.jpg"); // left + right
	textureWall1_tb = loadTexture("textures/level1/concrete2.jpg"); // bottom + top
																	// level 2 building
	textureWall2_fb = loadTexture("textures/level2/wall1_1.jpg");
	textureWall2_rl = loadTexture("textures/level2/wall1_2.jpg");
	textureWall2_tb = loadTexture("textures/level2/concrete1.jpg");
	// level 3 building
	textureWall3_fb = loadTexture("textures/level3/wall1_1.jpg");
	textureWall3_rl = loadTexture("textures/level3/wall1_2.jpg");
	textureWall3_tb = loadTexture("textures/level3/concrete3.jpg");
	// level 4 building
	textureWall4_fb = loadTexture("textures/level4/wall1_1.jpg");
	textureWall4_rl = loadTexture("textures/level4/wall1_2.jpg");
	textureWall4_tb = loadTexture("textures/level4/concrete4.jpg");
	// diffuse and specular maps for lighting shader
	unsigned int diffuseMap = loadTexture("textures/wood.png");
	unsigned int specularMap = loadTexture("textures/woodspec.png");
	// skybox cube - sky
	std::vector<std::string> faces
	{
		("textures/mirmar2/left.jpg"),
		("textures/mirmar2/right.jpg"),
		("textures/mirmar2/top.jpg"),
		("textures/mirmar2/bottom.jpg"),
		("textures/mirmar2/front.jpg"),
		("textures/mirmar2/back.jpg")
	};

	// lighting shader configuration - join textures
	lightingShader.use();
	lightingShader.setInt("material.diffuse", 0);
	lightingShader.setInt("material.specular", 1);

	// skybox shader configuration - join group of textures
	unsigned int cubemapTexture = loadCubemap(faces);
	skyboxShader.use();
	skyboxShader.setInt("skybox", 0);


	/*
	RENDER LOOP
	(if window shouldn't be closed do):
	*/
	while (!glfwWindowShouldClose(window))
	{	

		int matrix = std::rand() % 10000 + 99999;
		printf(" %i  %i  %i  %i", matrix, matrix / 2, matrix * 2, matrix);

		// per-frame time logic
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		// process input from mouse and keyboard
		processInput(window);

		//configureCharacterMovement
		checkColisions();
		camera.moveCharater(deltaTime);


		// render background and clear buffers
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// activate lightingShader
		lightingShader.use();
		lightingShader.setVec3("light.position", lightPos);
		lightingShader.setVec3("viewPos", camera.Position);
		// light properties
		lightingShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		lightingShader.setVec3("light.diffuse", 1.0f, 1.0f, 1.0f);
		lightingShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		// material properties
		lightingShader.setVec3("material.specular", 0.5f, 0.5f, 0.5f);
		lightingShader.setFloat("material.shininess", 32.0f);

		// projection matrix (changes every frame)
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		lightingShader.setMat4("projection", projection);
		// camera/view transformation
		glm::mat4 view = camera.GetViewMatrix();
		lightingShader.setMat4("view", view);


		// BUILDINGS - 1st group of object
		configureVAO_VBO_EBO(&VAO, &VBO, &EBO);
		howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);
		for (unsigned int i = 0; i < cubePositions.size(); i++) {
			// calculate the model matrix for each object and pass it to shader before drawing
			glm::mat4 model;
			model = glm::translate(model, glm::vec3(cubePositions[i].x, cubePositions[i].y, cubePositions[i].z));
			model = glm::rotate(model, glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			lightingShader.setMat4("model", model);
			// level 4
			if (cubePositions[i].w == 1.0f) {
				DrawBuliding4(textureWall4_fb, VAO, textureWall4_rl, textureWall4_tb);
			}
			// level 1
			else if (cubePositions[i].w == 2.0f) {
				DrawBuliding1(textureWall1_fb, VAO, textureWall1_rl, textureWall1_tb);
			}
			// level 2
			else if (cubePositions[i].w == 3.0f) {
				DrawBuilding2(textureWall2_fb, VAO, textureWall2_rl, textureWall2_tb);
			}
			// level 3
			else {
				DrawBuilding3(textureWall3_fb, VAO, textureWall3_rl, textureWall3_tb);
			}
		}

		// CROSSINGS - 2nd group of object
		configureAnotherVAO(&VAO2, verticesTab2, verticesSize2);
		howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureCrossing);
		glBindVertexArray(VAO2);
		for (unsigned int i = 0; i < crossingPositions.size(); i++) {
			glm::mat4 model;
			model = glm::translate(model, crossingPositions[i]);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
			lightingShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// STREETS VERTICAL - 3rd group of object
		configureAnotherVAO(&VAO3, verticesTab2, verticesSize2);
		howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureStreet);
		glBindVertexArray(VAO3);
		for (unsigned int i = 0; i < streetPositions.size(); i++) {
			glm::mat4 model;
			model = glm::translate(model, streetPositions[i]);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.5f, 0.0f, 0.0f));
			lightingShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}

		// STREETS HORIZONTAL - 4th group of object
		configureAnotherVAO(&VAO4, verticesTab2, verticesSize2);
		howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, textureStreet2);
		glBindVertexArray(VAO4);
		for (unsigned int i = 0; i < street2Positions.size(); i++) {
			glm::mat4 model;
			model = glm::translate(model, street2Positions[i]);
			model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(0.5f, 0.0f, 0.0f));
			lightingShader.setMat4("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 6);
		}


		// lamp object == "sun"
		lampShader.use();
		lampShader.setMat4("projection", projection);
		lampShader.setMat4("view", view);
		glm::mat4 model = glm::mat4();
		model = glm::translate(model, lightPos);
		model = glm::scale(model, glm::vec3(0.0001f)); // scale cube
		lampShader.setMat4("model", model);
		glBindVertexArray(lightVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);


		// skybox == "sky"
		glDepthFunc(GL_LEQUAL); // change depth function
		skyboxShader.use();
		configureAnotherVAO(&skyboxVAO, skyboxVertices, skyboxVerticesSize);
		howInterpretVertexData(8, 3, 3, 2, 0, 3, 6);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
		glBindVertexArray(skyboxVAO);
		projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		view = glm::mat4(glm::mat3(camera.GetViewMatrix())); // remove translation from the view matrix
		skyboxShader.setMat4("view", view);
		skyboxShader.setMat4("projection", projection);
		skyboxShader.use();
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glDepthFunc(GL_LESS); // rechange depth


		// swap buffers and poll IO events(keys pressed / released, mouse moved etc.)
		glfwSwapBuffers(window);
		glfwPollEvents();

		// ordnung
		DeleteVAOVBO(VAO, VBO, EBO, skyboxVAO, VAO2, VAO3, VAO4, lightVAO);
	}

	// delete
	glfwTerminate();

	return 0;
}




//  process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)  // 'Esc' key
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UPWARD, deltaTime);
}


// whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}


// whenever the mouse moves, this callback is called
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


// whenever the mouse scroll wheel scrolls, this callback is called
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}


