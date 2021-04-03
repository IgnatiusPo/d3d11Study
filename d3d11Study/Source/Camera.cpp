#include "Camera.h"

Camera::Camera(glm::vec3 position /*= glm::vec3(0.0f, 0.0f, 0.0f)*/,
	glm::vec3 front /*= glm::vec3(0.0f, -1.0f, 0.0f)*/, glm::vec3 up /*= glm::vec3(0.0f, 1.0f, 0.0f)*/,
	float yaw /*= YAW*/, float pitch /*= PITCH*/, float sensitivity /*= 0.1f*/, float zoom /*= 45.f*/, float cameraSpeed /*= 2.5f*/)
	: _position(position), _forward(front), _worldUp(glm::normalize(up)), _yaw(yaw), _pitch(pitch), _sensitivity(sensitivity), _zoom(zoom), _cameraSpeed(cameraSpeed)
{
	UpdateCameraVectors();
}

void Camera::ProcessKeyboardInput(CameraMovement direction, float deltaTime)
{
	switch (direction)
	{
	case CameraMovement::FORWARD:
		_position += _forward * _cameraSpeed * deltaTime;
		break;
	case CameraMovement::BACKWARD:
		_position -= _forward * _cameraSpeed * deltaTime;
		break;
	case CameraMovement::RIGHT:
		_position += _right * _cameraSpeed * deltaTime;
		break;
	case CameraMovement::LEFT:
		_position -= _right * _cameraSpeed * deltaTime;
		break;
	}
}

void Camera::ProcessMouseInput(float x, float y)
{
    static bool firstMouse = true;
	if (firstMouse)
	{
		_lastCameraX = (float)x;
		_lastCameraY = (float)y;
		firstMouse = false;
	}

	float xOffset =  _lastCameraX- (float)x;
	float yOffset = _lastCameraY - (float)y;
	_lastCameraX = (float)x;
	_lastCameraY = (float)y;

	xOffset *= _sensitivity;
	yOffset *= _sensitivity;

	_yaw += xOffset;
	_pitch += yOffset;

	if (_pitch > 89.0f)
		_pitch = 89.0f;
	if (_pitch < -89.0f)
		_pitch = -89.0f;
	UpdateCameraVectors();
}
void Camera::ProcessScrollInput(float yOffset, float& fov)
{
	if (fov >= 1.0f && fov <= 45.f)
		fov -= yOffset;
	else if (fov <= 1.0f)
		fov = 1.0f;
	else if (fov >= 45.0f)
		fov = 45.f;
}
void Camera::UpdateCameraVectors()
{
	//@todo normal rotation
	//forward vector
	glm::vec3 front;
	front.x = cos(glm::radians(_pitch)) * cos(glm::radians(_yaw));
	front.y = sin(glm::radians(_pitch));
	front.z = cos(glm::radians(_pitch)) * sin(glm::radians(_yaw));
	_forward = glm::normalize(front);
	//glm::mat3 rotX = glm::mat3(1.f);
	//rotX[0] = glm::vec3(1.f, 0.f, 0.f);
	//rotX[1] = glm::vec3(0.f, glm::cos(glm::radians(_pitch)), glm::sin(glm::radians(_pitch)));
	//rotX[2] = glm::vec3(0.f, -glm::sin(glm::radians(_pitch)), glm::cos(glm::radians(_pitch)));

	//glm::mat3 rotY = glm::mat3(1.f);
	//rotY[0] = glm::vec3(glm::cos(glm::radians(_yaw)), 0.f, -glm::sin(glm::radians(_yaw)));
	//rotY[1] = glm::vec3(0.f, 1.f, 0.f);
	//rotY[2] = glm::vec3(glm::sin(glm::radians(_yaw)), 0.f, glm::cos(glm::radians(_yaw)));

	//_forward = _forward * rotX * rotY;
	//_forward = glm::normalize(_forward);
	//right and up
	_right = glm::normalize(glm::cross( _worldUp, _forward));
	_up = glm::normalize(glm::cross( _forward, _right));
}

glm::mat4 Camera::GetViewMatrix() const
{
	return myLookAt(_position, _position + _forward, _up);
}
glm::mat4 myLookAt(const glm::vec3& eye, const glm::vec3& center, const glm::vec3& up)
{
	glm::vec3 zAxis = glm::normalize(center - eye);
	glm::vec3 xAxis = glm::normalize(glm::cross( glm::normalize(up), zAxis ));
	glm::vec3 yAxis = glm::cross( zAxis, xAxis );

	glm::mat4 matTranslation = glm::mat4(1.0f);
	// initialization Translation matrix
	matTranslation[3][0] = -eye.x;
	matTranslation[3][1] = -eye.y;
	matTranslation[3][2] = -eye.z;

	glm::mat4 matRotation = glm::mat4(1.0f);
	// initialization Rotation matrix
	matRotation[0][0] = xAxis.x;
	matRotation[1][0] = xAxis.y;
	matRotation[2][0] = xAxis.z;

	matRotation[0][1] = yAxis.x;
	matRotation[1][1] = yAxis.y;
	matRotation[2][1] = yAxis.z;

	matRotation[0][2] = zAxis.x;
	matRotation[1][2] = zAxis.y;
	matRotation[2][2] = zAxis.z;



	return matRotation * matTranslation;
}





