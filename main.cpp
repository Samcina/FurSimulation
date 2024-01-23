#include <iostream>
#include <random>
#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm.hpp>
#include <gtc/matrix_transform.hpp>
#include <gtc/constants.hpp>
#include <gtc/type_ptr.hpp>
#include "ShaderProgram.hpp"
#include "Camera.h"
#include "MeshObject.h"
#include "Texture.h"


// functions
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
GLfloat* createMasterHairs(const MeshObject& object);
GLuint generateTextureFromHairData(GLfloat* hairData);
GLuint createRandomness();


//variables
Camera camera(glm::vec3(10.0f, 0.0f, 0.0f), glm::vec3(0.f, 1.f, 0.f), 180, 0);
const GLuint WIDTH = 1920, HEIGHT = 1080;
float lastX = WIDTH / 2.0f;
float lastY = HEIGHT / 2.0f;
glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
bool movingLight = true;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0f;
glm::mat4 model;
float windMag = 1.f;
glm::vec4 windDir = { 0.f, -1.f, 1.f, 0.f };
float minWindStrength = 0.f;
float maxWindStrength = 1000.f;
float windStrength = 500.0f;
int segmentHairCount = 5;
float hairLen = 0.05f;
const int varsPerHair = 1;
int masterHairCount;


int main()
{
	if (!glfwInit()) {
		std::cout << "Failed to initialise GLFW" << std::endl;
		return 1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_SAMPLES, 4);

	GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Fur Simulation", NULL, NULL);

	if (window == nullptr) {
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glewExperimental = GL_TRUE;
	if (glewInit() != GLEW_OK) {
		std::cout << "Failed to initialize GLEW" << std::endl;
		return -1;
	}
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//models
	MeshObject mesh;
	mesh.readOBJ("");
	Texture mainTexture = Texture("");
	GLfloat* hairData = createMasterHairs(mesh);
	GLuint hairTexture_resting = generateTextureFromHairData(hairData);
	GLuint hairTextureLastFrame = generateTextureFromHairData(hairData);
	GLuint hairTextureCurrentFrame = generateTextureFromHairData(hairData);
	GLuint hairTextureSimulatedFrame = generateTextureFromHairData(NULL);
	GLuint randomTexture = createRandomness();

	MeshObject windSphere;
	windSphere.createSphere(0.5f, 10);
	glm::vec4 windPos = { 0.f, 0.f, 0.f, 1.f };
	Texture windTexture = Texture("");


	//shaders
	std::string plainVertexFile = "";
	std::string plainFragmentFile = "";
	ShaderProgram plainShader(plainVertexFile, "", "", "", plainFragmentFile);
	plainShader();
	std::string furVertexFile = "";
	std::string furTessellationControlFile = "";
	std::string furTessellationEvaluateFile = "";
	std::string furGeometryFile = "";
	std::string furFragmentFile = "";
	ShaderProgram furShader(furVertexFile, furTessellationControlFile, furTessellationEvaluateFile, furGeometryFile, furFragmentFile);
	furShader();
	std::string furSimulationComputeFile = "";
	ComputeShader furSimulationShader(furSimulationComputeFile);
	furSimulationShader();

	//set uniforms
	plainShader();
	GLint modelLocPlain = glGetUniformLocation(plainShader, "model");
	GLint viewLocPlain = glGetUniformLocation(plainShader, "view");
	GLint projLocPlain = glGetUniformLocation(plainShader, "projection");
	GLint lightPosLocPlain = glGetUniformLocation(plainShader, "lightPos");
	GLint lightColorLocPlain = glGetUniformLocation(plainShader, "lightColor");
	GLint mainTextureLocPlain = glGetUniformLocation(furShader, "mainTexture");
	glUniform1i(mainTextureLocPlain, 0);

	furShader();
	GLint modelLocFur = glGetUniformLocation(furShader, "model");
	GLint viewLocFur = glGetUniformLocation(furShader, "view");
	GLint projLocFur = glGetUniformLocation(furShader, "projection");
	GLint mainTextureLoc = glGetUniformLocation(furShader, "mainTexture");
	GLint hairDataTextureLoc = glGetUniformLocation(furShader, "hairDataTexture");
	GLint randomDataTextureLoc = glGetUniformLocation(furShader, "randomDataTexture");
	glUniform1i(mainTextureLoc, 0);
	glUniform1i(hairDataTextureLoc, 1);
	glUniform1i(randomDataTextureLoc, 2);
	GLint noOfHairSegmentLoc = glGetUniformLocation(furShader, "noOfHairSegments");
	GLint noOfVerticesLoc = glGetUniformLocation(furShader, "noOfVertices");
	GLint nrOfDataVariablesPerMasterHairLoc = glGetUniformLocation(furShader, "nrOfDataVariablesPerMasterHair");
	GLint cameraPosLocFur = glGetUniformLocation(furShader, "cameraPosition");
	GLint lightPosLocFur = glGetUniformLocation(furShader, "lightPos");
	GLint lightColorLocFur = glGetUniformLocation(furShader, "lightColor");

	furSimulationShader();
	GLint modelLocFurSim = glGetUniformLocation(furSimulationShader, "model");
	GLint hairSegmentLengthSimLoc = glGetUniformLocation(furSimulationShader, "hairSegmentLength");
	GLint windMagnitudeSimLoc = glGetUniformLocation(furSimulationShader, "windMagnitude");
	GLint windDirectionSimLoc = glGetUniformLocation(furSimulationShader, "windDirection");


	float rotAngle = 0.f;
	while (!glfwWindowShouldClose(window))
	{
		float currFrame = glfwGetTime();
		deltaTime = currFrame - lastFrame;
		lastFrame = currFrame;
		processInput(window);
		glClearColor(0.25f, 0.15f, 0.18f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		glm::mat4 view = camera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
		glm::mat4 lightModel;
		if (windStrength > 0.1f || movingLight)
			rotAngle += 0.05f;
		lightModel = glm::rotate(lightModel, glm::radians(rotAngle), glm::vec3(0.f, 1.f, 0.f));
		lightModel = glm::translate(lightModel, glm::vec3(0.f, 0.f, 5.f));
		if (windStrength > 0.1f)
			windDir = glm::normalize(windPos - lightModel * windPos);
		glm::vec3 lightPos(0.f, 0.f, 0.f);
		lightPos = glm::vec3(lightModel * glm::vec4(lightPos, 1.0f));
		windMag *= (pow(sin(currFrame * 0.05), 2) + 0.5);

		furSimulationShader();
		glBindImageTexture(0, hairTexture_resting, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(1, hairTextureLastFrame, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(2, hairTextureCurrentFrame, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
		glBindImageTexture(3, hairTextureSimulatedFrame, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
		glUniformMatrix4fv(modelLocFurSim, 1, GL_FALSE, glm::value_ptr(model));
		glUniform1f(hairSegmentLengthSimLoc, hairLen);
		glUniform1f(windMagnitudeSimLoc, windMag + windStrength);
		glUniform4f(windDirectionSimLoc, windDir.x, windDir.y, windDir.z, windDir.w);
		glDispatchCompute(1, masterHairCount, 1);

		if (windStrength > 0.1f || movingLight) {
			plainShader();
			glUniformMatrix4fv(modelLocPlain, 1, GL_FALSE, glm::value_ptr(lightModel));
			glUniformMatrix4fv(viewLocPlain, 1, GL_FALSE, glm::value_ptr(view));
			glUniformMatrix4fv(projLocPlain, 1, GL_FALSE, glm::value_ptr(projection));
			glUniform3f(lightPosLocPlain, lightPos.x, lightPos.y, lightPos.z);
			glUniform3f(lightColorLocPlain, lightColor.x, lightColor.y, lightColor.z);
			glActiveTexture(GL_TEXTURE0 + 0);
			glBindTexture(GL_TEXTURE_2D, windTexture.textureID);
			windSphere.render(false);
		}

		plainShader();
		glUniformMatrix4fv(modelLocPlain, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLocPlain, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLocPlain, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3f(lightPosLocPlain, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(lightColorLocPlain, lightColor.x, lightColor.y, lightColor.z);
		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, mainTexture.textureID);
		mesh.render(false);

		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		furShader();
		glUniformMatrix4fv(modelLocFur, 1, GL_FALSE, glm::value_ptr(model));
		glUniformMatrix4fv(viewLocFur, 1, GL_FALSE, glm::value_ptr(view));
		glUniformMatrix4fv(projLocFur, 1, GL_FALSE, glm::value_ptr(projection));
		glUniform3f(lightPosLocFur, lightPos.x, lightPos.y, lightPos.z);
		glUniform3f(lightColorLocFur, lightColor.x, lightColor.y, lightColor.z);
		glUniform3f(cameraPosLocFur, camera.Position.x, camera.Position.y, camera.Position.z);
		glUniform1f(noOfHairSegmentLoc, (float)segmentHairCount);
		glUniform1f(noOfVerticesLoc, (float)masterHairCount);
		glUniform1f(nrOfDataVariablesPerMasterHairLoc, (float)varsPerHair);

		glActiveTexture(GL_TEXTURE0 + 0);
		glBindTexture(GL_TEXTURE_2D, mainTexture.textureID);
		glActiveTexture(GL_TEXTURE0 + 1);
		glBindTexture(GL_TEXTURE_2D, hairTextureSimulatedFrame);
		glActiveTexture(GL_TEXTURE0 + 2);
		glBindTexture(GL_TEXTURE_2D, randomTexture);
		mesh.render(true);

		glCopyImageSubData(hairTextureCurrentFrame, GL_TEXTURE_2D, 0, 0, 0, 0,
			hairTextureLastFrame, GL_TEXTURE_2D, 0, 0, 0, 0,
			segmentHairCount, masterHairCount, 1);
		glCopyImageSubData(hairTextureSimulatedFrame, GL_TEXTURE_2D, 0, 0, 0, 0,
			hairTextureCurrentFrame, GL_TEXTURE_2D, 0, 0, 0, 0,
			segmentHairCount, masterHairCount, 1);
		glfwSwapBuffers(window);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void processInput(GLFWwindow* window) {
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
		model = glm::translate(model, glm::vec3(0.05f, 0.f, 0.f));
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
		model = glm::translate(model, glm::vec3(-0.05f, 0.f, 0.f));
	if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
		model = glm::translate(model, glm::vec3(0.f, -0.05f, 0.f));
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
		model = glm::translate(model, glm::vec3(0.f, 0.05f, 0.f));
	if (glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS)
		movingLight = true;
	if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
		movingLight = false;
	if (glfwGetKey(window, GLFW_KEY_PERIOD) == GLFW_PRESS)
		if (windStrength < maxWindStrength)
			windStrength += 10.f;
	if (glfwGetKey(window, GLFW_KEY_COMMA) == GLFW_PRESS)
		if (windStrength > minWindStrength)
			windStrength -= 10.f;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
	glfwGetWindowSize(window, &width, &height);
	glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}
	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;
	lastX = xpos;
	lastY = ypos;
	model = glm::translate(model, glm::vec3(0.f, yoffset * 0.01f, -xoffset * 0.01f));
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
	camera.ProcessMouseScroll(yoffset);
}

GLfloat* createMasterHairs(const MeshObject& object) {
	GLfloat* vertexArray = object.getVertexArray();
	masterHairCount = object.getNoOfVertices();
	int amountOfDataPerMasterHair = segmentHairCount * 4 * varsPerHair;
	GLfloat* hairData = new GLfloat[masterHairCount * amountOfDataPerMasterHair];
	int masterHairIndex = 0;
	int stride = 8;
	for (int i = 0; i < masterHairCount * stride; i = i + stride) {
		glm::vec4 rootPos = glm::vec4(vertexArray[i], vertexArray[i + 1], vertexArray[i + 2], 1.f);
		rootPos = model * rootPos;
		glm::vec4 rootNormal = glm::vec4(vertexArray[i + 3], vertexArray[i + 4], vertexArray[i + 5], 0.f);
		hairData[masterHairIndex++] = rootPos.x;
		hairData[masterHairIndex++] = rootPos.y;
		hairData[masterHairIndex++] = rootPos.z;
		hairData[masterHairIndex++] = rootPos.w;
		for (int hairSegment = 0; hairSegment < segmentHairCount - 1; hairSegment++) {
			glm::vec4 newPos = rootPos + hairSegment * hairLen * rootNormal;
			hairData[masterHairIndex++] = newPos.x;
			hairData[masterHairIndex++] = newPos.y;
			hairData[masterHairIndex++] = newPos.z;
			hairData[masterHairIndex++] = newPos.w;
		}
	}
	return hairData;
}

GLuint generateTextureFromHairData(GLfloat* hairData) {
	GLuint hairDataTextureID;
	glGenTextures(1, &hairDataTextureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hairDataTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, segmentHairCount * varsPerHair, masterHairCount, 0, GL_RGBA, GL_FLOAT, hairData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return hairDataTextureID;
}

GLuint createRandomness() {
	GLfloat* randomData = new GLfloat[2048 * 2048 * 3];
	std::random_device rd;
	std::mt19937* gen = new std::mt19937(rd());
	std::uniform_real_distribution<float>* dis = new std::uniform_real_distribution<float>(-1, 1);
	for (int i = 0; i < 2048 * 2048 * 3; i++) {
		randomData[i] = (*dis)(*gen) * 0.3f;
	}
	delete gen;
	delete dis;
	GLuint randomDataTextureID;
	glGenTextures(1, &randomDataTextureID);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, randomDataTextureID);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, 2048, 2048, 0, GL_RGB, GL_FLOAT, randomData);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return randomDataTextureID;
}

