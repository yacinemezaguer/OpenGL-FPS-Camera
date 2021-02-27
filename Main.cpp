#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Shader.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>

constexpr auto HOR_SPEED = 0.05f;
constexpr auto VER_SPEED = HOR_SPEED;
constexpr auto MOUSE_SENSITIVITY = 0.05f;

bool processInput(GLFWwindow* window);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);

float* genCylinderVertices(unsigned int nbVertices, float radius, float height);
unsigned int* genCylinderIndices(unsigned int nbVertices);

struct UserData {
	double initCursorX = 0.0f;
	double initCursorY = 0.0f;

	double deltaX;
	double deltaY;

	glm::vec3 camPos = glm::vec3(0.0f, 0.0f, -6.0f);
	glm::vec3 targetPos = glm::vec3(0.0f, 0.0f, -5.0f);
	glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f);
	glm::vec3 rightVec = glm::vec3(1.0f, 0.0f, 0.0f);

	bool leftPressed;
};

int main() {

	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	int w_width = 800, w_height = 600;
	GLFWwindow* window = glfwCreateWindow(w_width, w_height, "OpenGL FPS Cylindre", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Impossible de créer la fenêtre GLFW" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	UserData wInfo;
	glfwSetWindowUserPointer(window, &wInfo);
	glfwSetMouseButtonCallback(window, mouse_button_callback);
	glfwSetScrollCallback(window, scroll_callback);
	glfwSetCursorPosCallback(window, cursor_position_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//Construction du cylindre
	unsigned int nbVertices = 15;
	float *vertices = genCylinderVertices(nbVertices, 0.5, 1.5f);
	unsigned int* indices = genCylinderIndices(nbVertices);

	//Create VAO and bind it
	unsigned int VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	//Create EBO and bind it
	unsigned int EBO;
	glGenBuffers(1, &EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, (4 * nbVertices + 2) * sizeof(unsigned int), indices, GL_STATIC_DRAW);

	//Create VBO and bind it
	unsigned int VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, 6*(2*nbVertices) * sizeof(float), vertices, GL_STATIC_DRAW);


	//shader program
	Shader ourShader("shader.vs", "shader.fs");

	//position attribute
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	//color attribute
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	
	glEnable(GL_DEPTH_TEST);

	//Initializing FPS counter
	double timeStamp = glfwGetTime();
	double sum = 0;
	int counter = 0;

	//Initializing cursor position
	glfwGetCursorPos(window, &(wInfo.initCursorX), &(wInfo.initCursorY));

	//render loop ---------------------------------------------------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {
	    if(counter >0) {
			if (counter > 100) {
				counter = 1;
				sum = 0;
			}
			sum += 1.0f/(glfwGetTime()-timeStamp);
			//std::cout << sum/(double) counter <<  " FPS" << std::endl;
	    }
	    counter++;
        timeStamp = glfwGetTime();

		bool changed = processInput(window);

		glClearColor(0.4f, 0.5f, 0.5f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		ourShader.use();
		glBindVertexArray(VAO);

		//Chagements de l'objet
		/*if (changed)
		{
			model = glm::rotate(model, glm::radians((float) (wInfo.deltaX / 5.0f)), glm::vec3(0.0f, 1.0f, 0.0f));
			model = glm::rotate(model, glm::radians((float) (wInfo.deltaY / 5.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
		}*/

		//model matrix
		glm::mat4 model = glm::mat4(1.0f);

		//view matrix
		glm::mat4 view = glm::lookAt(wInfo.camPos, wInfo.targetPos, wInfo.camUp);

		//projection matrix
		glfwGetWindowSize(window, &w_width, &w_height);
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)w_width / w_height, 0.1f, 100.0f);

		ourShader.setMat4f("model", model);
		ourShader.setMat4f("view", view);
		ourShader.setMat4f("projection", projection);

		//1st Cylindre

		float time = glfwGetTime();
		float redValue = sin(time) / 3.0f + 0.4f;
		float greenValue = sin(time + M_PI_2) / 2.0f + 0.5f;
		float blueValue = sin(time + M_PI) / 2.0f + 0.5f;

		int vertexColorLocation = glGetUniformLocation(ourShader.ID, "color");
		glUniform4f(vertexColorLocation, redValue, greenValue, blueValue, 1.0f);

		//Sides
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLE_STRIP, 2 * (nbVertices + 1), GL_UNSIGNED_INT, 0);

		//Faces
		glDrawElements(GL_TRIANGLE_FAN, nbVertices, GL_UNSIGNED_INT, (void*) (2 * (nbVertices + 1)*sizeof(unsigned int)) );
		glDrawElements(GL_TRIANGLE_FAN, nbVertices, GL_UNSIGNED_INT, (void*) ((3 * nbVertices + 2) * sizeof(unsigned int)));

		//Wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLE_STRIP, 2 * (nbVertices + 1), GL_UNSIGNED_INT, 0);

		//2nd Cylindre---------------------------------
		glUniform4f(vertexColorLocation, redValue, greenValue, blueValue, 1.0f);

		model = glm::mat4(1.0f);
        model = glm::rotate(model, glm::radians(-55.0f), glm::vec3(0.4f, 0.2f, -0.7f));
        model = glm::translate(model, glm::vec3(2.0f, 0.0f, 0.0f));
        ourShader.setMat4f("model", model);

        //Sides
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glDrawElements(GL_TRIANGLE_STRIP, 2 * (nbVertices + 1), GL_UNSIGNED_INT, 0);

		//Faces
		glDrawElements(GL_TRIANGLE_FAN, nbVertices, GL_UNSIGNED_INT, (void*) (2 * (nbVertices + 1)*sizeof(unsigned int)) );
		glDrawElements(GL_TRIANGLE_FAN, nbVertices, GL_UNSIGNED_INT, (void*) ((3 * nbVertices + 2) * sizeof(unsigned int)));

		//Wireframe
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		glUniform4f(vertexColorLocation, 1.0f, 1.0f, 1.0f, 1.0f);
		glDrawElements(GL_TRIANGLE_STRIP, 2 * (nbVertices + 1), GL_UNSIGNED_INT, 0);
		//-----------------------------------------------------------

		//swap buffers and check events
		glfwSwapBuffers(window);
		glfwPollEvents();
	}// ------------------------------------------------------------------------------------------------------------------------------

	glfwTerminate();

	delete[] vertices;
	delete[] indices;

	return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	glViewport(0, 0, width, height);
}

bool processInput(GLFWwindow* window)
{	//Retourne vrai si quelque chose a changé

	UserData* wInfo = (UserData*)glfwGetWindowUserPointer(window);
	glm::vec3 frontDirection = wInfo->targetPos - wInfo->camPos;

	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		//Exit
		glfwSetWindowShouldClose(window, true);
	}

	if(glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        //Left lateral movement
		wInfo->camPos = glm::vec3(glm::translate(glm::mat4(1.0), -HOR_SPEED * wInfo->rightVec) * glm::vec4(wInfo->camPos, 1.0f));
		wInfo->targetPos = glm::vec3(glm::translate(glm::mat4(1.0), -HOR_SPEED * wInfo->rightVec) * glm::vec4(wInfo->targetPos, 1.0f));
	}

	if(glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
		//Right lateral movement
		wInfo->camPos = glm::vec3(glm::translate(glm::mat4(1.0), HOR_SPEED * wInfo->rightVec) * glm::vec4(wInfo->camPos, 1.0f));
		wInfo->targetPos = glm::vec3(glm::translate(glm::mat4(1.0), HOR_SPEED * wInfo->rightVec) * glm::vec4(wInfo->targetPos, 1.0f));
	}

	if(glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        //Forward movement
		wInfo->camPos = glm::vec3(glm::translate(glm::mat4(1.0), HOR_SPEED *(frontDirection)) * glm::vec4(wInfo->camPos, 1.0f));
		wInfo->targetPos = glm::vec3(glm::translate(glm::mat4(1.0), HOR_SPEED * (frontDirection)) * glm::vec4(wInfo->targetPos, 1.0f));
	}

	if(glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        //Backward movement
		wInfo->camPos = glm::vec3(glm::translate(glm::mat4(1.0), -HOR_SPEED * (frontDirection)) * glm::vec4(wInfo->camPos, 1.0f));
		wInfo->targetPos = glm::vec3(glm::translate(glm::mat4(1.0), -HOR_SPEED * (frontDirection)) * glm::vec4(wInfo->targetPos, 1.0f));
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
		//Reset initial position
		wInfo->camPos = glm::vec3(0.0f, 0.0f, -6.0f);
		wInfo->targetPos = glm::vec3(0.0f, 0.0f, -5.0f);
		wInfo->camUp = glm::vec3(0.0f, 1.0f, 0.0f);
	}

	return false;
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
	UserData* wInfo = (UserData*) glfwGetWindowUserPointer(window);
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		wInfo->leftPressed = true;
	}
	else if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
		wInfo->leftPressed = false;
	}
}

float* genCylinderVertices(unsigned int nbVertices, float radius, float height)
{
	float* vertices = new float[2*6*nbVertices];
	float intervalle = 2.0 * M_PI / (float) nbVertices;

	int j = 0;
	for (float i = 0; i<2.0*M_PI - 0.014f; i += intervalle)
	{
		//position
		vertices[j] = (float) cos(i) * radius;
		vertices[j + 1] = (float) sin(i) * radius;
		vertices[j + 2] = -height/2.0;

		//color
		vertices[j + 3] = (float)(sin(i + 0.5) / 3.0 + 0.4f);
		vertices[j + 4] = (float)(cos(i + 1.0) / 2.0 + 0.5f);
		vertices[j + 5] = (float)(sin(i) / 2.0 + 0.5f);

		j+= 6;
	}

	for (float i = 0; i < 2.0 * M_PI - 0.014f; i += intervalle)
	{
		//position
		vertices[j] = (float)cos(i) * radius;
		vertices[j + 1] = (float)sin(i) * radius;
		vertices[j + 2] = height/2.0;

		//color
		vertices[j + 3] = (float)(sin(i + 0.5) / 3.0 + 0.4f);
		vertices[j + 4] = (float)(cos(i + 1.0) / 2.0 + 0.5f);
		vertices[j + 5] = (float)(sin(i) / 2.0 + 0.5f);

		j += 6;
	}

	return vertices;
}

unsigned int* genCylinderIndices(unsigned int nbVertices)
{
	unsigned int* indice = new unsigned int[4*nbVertices + 2];

	for (unsigned int i = 0; i < 2*nbVertices; i+=2)
	{
		indice[i] = i/2;
		indice[i + 1] = i/2 + nbVertices;
	}

	indice[2 * nbVertices] = 0;
	indice[2 * nbVertices +1] = nbVertices;

	for (int i = 0; i < nbVertices; i++) {
		indice[2 * nbVertices + 2 + i] = i;
	}

	for (int i = 0; i < nbVertices; i++) {
		indice[3 * nbVertices + 2 + i] = nbVertices + i;
	}

	return indice;
}

void scroll_callback(GLFWwindow * window, double xoffset, double yoffset)
{
	UserData* wInfo = (UserData*)glfwGetWindowUserPointer(window);

	wInfo->camPos = glm::vec3(glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0f, yoffset)) * glm::vec4(wInfo->camPos, 1.0f));
	wInfo->targetPos = glm::vec3(glm::translate(glm::mat4(1.0), glm::vec3(0.0f, 0.0F, yoffset)) * glm::vec4(wInfo->targetPos, 1.0f));
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
	UserData* wInfo = (UserData*)glfwGetWindowUserPointer(window);
	//std::cout << xpos << ", " << ypos << std::endl;

	wInfo->deltaX = xpos - wInfo->initCursorX;
	wInfo->deltaY = ypos - wInfo->initCursorY;
	wInfo->initCursorX = xpos;
	wInfo->initCursorY = ypos;

	wInfo->rightVec = glm::cross(wInfo->targetPos - wInfo->camPos, wInfo->camUp);

	/*std::cout << "camPos: " << glm::to_string(wInfo->camPos) << std::endl;
	std::cout << "targetPos: " << glm::to_string(wInfo->targetPos) << std::endl;
	std::cout << "camUp: " << glm::to_string(wInfo->camUp) << std::endl;
	std::cout << "right: " << glm::to_string(wInfo->rightVec) << std::endl;
	std::cout << "----" << std::endl;*/

	//Vertical rotation (Pitch)----------------------------------------------------------------------------------------

	//Translate camera to origin, then rotate (target), then retranslate back to camera position
	wInfo->targetPos = glm::translate(
		glm::rotate(
			glm::translate(glm::mat4(1.0f), wInfo->camPos), 
			glm::radians((float)(MOUSE_SENSITIVITY * -wInfo->deltaY)), wInfo->rightVec),
	-wInfo->camPos)
	* glm::vec4(wInfo->targetPos, 1.0f);

	wInfo->camUp = glm::rotate(glm::mat4(1.0f), glm::radians((float)(MOUSE_SENSITIVITY * -wInfo->deltaY)), wInfo->rightVec)
		* glm::vec4(wInfo->camUp, 1.0f);
	
	//Horizontal rotation (Yaw)----------------------------------------------------------------------------------------
	wInfo->targetPos = glm::translate(
		glm::rotate(
			glm::translate(glm::mat4(1.0f), wInfo->camPos),
			glm::radians((float)(MOUSE_SENSITIVITY * -wInfo->deltaX)), wInfo->camUp),
		-wInfo->camPos)
		* glm::vec4(wInfo->targetPos, 1.0f);
	//-----------------------------------------------------------------------------------------------------------------
}
