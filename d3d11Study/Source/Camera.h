#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"

enum class CameraMovement { FORWARD, BACKWARD, RIGHT, LEFT };
const float YAW = 90.0f;
const float PITCH = 0.0f;

glm::mat4 myLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up);

class Camera
{
private:
	float _lastCameraX;
	float _lastCameraY;
public:
	glm::vec3 _position;
	glm::vec3 _forward;
	glm::vec3 _right;
	glm::vec3 _up;
	glm::vec3 _worldUp;
	float _yaw;
	float _pitch;
	float _sensitivity;
	float _zoom;
	float _cameraSpeed;
public:
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 front = glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
		float yaw = YAW, float pitch = PITCH, float sensitivity = 0.1f, float zoom = 45.f, float cameraSpeed = 20.5f);

	void ProcessKeyboardInput(CameraMovement direction, float deltaTime);
	void ProcessMouseInput(float x, float y);
	void ProcessScrollInput(float yOffset, float& fov);
	glm::mat4 GetViewMatrix() const;

private:
	void UpdateCameraVectors();


};
