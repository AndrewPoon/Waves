#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <map>

#include "ShaderProgram.h"
#include "Camera.h"
#include "Water.h"

const int width = 1200, height = 1080;


Camera camera(45.0f, 45.0f, 12.5f, glm::vec3(0, 0, 0));
std::map<int, bool> keys;
//same camera as assignment 5, so bind both cursor and keycall back to change view
void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
	camera.cursor_callback(window, xpos, ypos);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
		keys[GLFW_KEY_UP] = true;
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE)
		keys[GLFW_KEY_UP] = false;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
		keys[GLFW_KEY_DOWN] = true;
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE)
		keys[GLFW_KEY_DOWN] = false;

}
//change this for outer/inner tesslation level
float ol = 32.0f, il = 32.0f;
int main() {

	// Initialize keys
	keys[GLFW_KEY_UP] = false;
	keys[GLFW_KEY_DOWN] = false;

	if (!glfwInit()) {
		std::cout << "GLFW failed to init" << std::endl;
		return -1;
	}

	GLFWwindow* window = glfwCreateWindow(width, height, "Water", NULL, NULL);

	if (!window) {
		std::cout << "Window failed to init" << std::endl;
		return -1;
	}

	glfwMakeContextCurrent(window);


	if (glewInit() != GLEW_OK) {
		std::cout << "GLEW failed to init" << std::endl;
		return -1;
	}

	// OpenGL enables/init
	glEnable(GL_DEPTH_TEST);
	glfwSetCursorPosCallback(window, cursor_pos_callback);
	glfwSetKeyCallback(window, key_callback);
	ShaderProgram* tess_shader=new ShaderProgram();
	Water water(-20, 20, 1);

	glm::vec3 lightDir(1, -0.5, 1);
	glm::mat4 projection = glm::perspective(45.0f, width / (float)height, 0.1f, 100.0f);
	double last = glfwGetTime(), current;
	double timelapsed;
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();

		// time for updating mouse/water motion
		current = glfwGetTime();
		timelapsed = current - last;
		last = current;

		//get the new mvp from camera
		camera.update(timelapsed);
		glm::mat4 mvp = projection*camera.getViewMatrix();
		
		
		glClearColor(0.2f,0.2f,0.2f, 1);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		//Render the water texture and set the uniforms needed for shaders
		tess_shader->setUniform1f("TessLevelInner", ol);
		tess_shader->setUniform1f("TessLevelOuter", il);
		tess_shader->setUniform3fv("lightDir", lightDir);
		tess_shader->setUniform3fv("eyeDir", camera.camera_pos);
		tess_shader->setUniform1f("time", last);
		water.draw(*tess_shader,mvp);

		glfwSwapBuffers(window);
	}

	glfwTerminate();
	return 0;
}