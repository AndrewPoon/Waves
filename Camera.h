#ifndef CAMERA_H
#define CAMERA_H
#include <iostream>
#include <algorithm>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/string_cast.hpp>

//get the keys from Main
extern std::map<int, bool> keys;
struct Camera {
	//Spherical coordinate stuff
	float theta; //x-z plane angle
	float phi; //elevation angle
	float radius;

	float cam_speed;
	float cam_rotation_speed;
	glm::vec3 camera_pos;
	glm::vec3 camera_dir;

	//initalize the camera and have it target at origin
	Camera(float theta, float phi, float radius, glm::vec3 camera_dir) :
		theta(theta), phi(phi), radius(radius), cam_speed(5.0f), cam_rotation_speed(0.5f), camera_dir(camera_dir), camera_pos({ 0, 0, 0 }) {}

	void update(float deltaTime) {
		//change the radius based on key pressed. max 50 and min 0.1
		if (keys[GLFW_KEY_DOWN])
			radius = std::min(radius + cam_speed * deltaTime, 50.0f);
		if (keys[GLFW_KEY_UP])
			radius = std::max(radius - cam_speed * deltaTime, 0.1f);

		// Calculate the camera position based on the angles and radius
		camera_pos.x = radius * sin(glm::radians(theta)) * sin(glm::radians(phi));
		camera_pos.y = radius * cos(glm::radians(phi));
		camera_pos.z = radius * cos(glm::radians(theta)) * sin(glm::radians(phi));
		camera_pos += camera_dir;
	}

	glm::mat4 getViewMatrix() {
		return glm::lookAt(camera_pos, camera_dir, glm::vec3(0, 1, 0));
	}

	// mx and my to keep track old mouse position
	//-1 to signify the mouse hasn't move yet
	double mx = -1, my = -1;
	void cursor_callback(GLFWwindow* window, double xpos, double ypos) {
		//when the mouse is released, reset the old mouse position to -1 and it can change again 
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE) {
			mx = -1; my = -1;
			return;
		}
		//get the delta
		float dx = xpos - mx;
		float dy = ypos - my;

		// need this to check if it is our first move, or else camera will jump around 
		//based on the point clicked
		if (mx == -1 || my == -1) {
			dx = 0;
			dy = 0;
		}
		//change the angles
		theta -= dx * cam_rotation_speed;
		phi -= dy * cam_rotation_speed;
		//clamp phi between 0.1-179
		if (phi > 179.0f)
			phi = 179.0f;
		if (phi < 0.1f)
			phi = 0.1f;
		//update old mouse position
		mx = xpos; my = ypos;
	}
};

#endif