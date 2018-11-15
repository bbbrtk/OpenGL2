#ifndef CAMERA_H
#define CAMERA_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>

// Defines several possible options for camera movement. Used as abstraction to stay away from window-system specific input methods
enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UPWARD
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 1.0f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;


// An abstract camera class that processes input and calculates the corresponding Euler Angles, Vectors and Matrices for use in OpenGL
class Camera
{
public:
	// Camera Attributes
	glm::vec3 Position;
	glm::vec3 StartPosition;
	glm::vec3 Front;
	glm::vec3 Up;
	glm::vec3 Right;
	glm::vec3 WorldUp;
	// Euler Angles
	float Yaw;
	float Pitch;
	// Camera options
	float MovementSpeed = 1.0;
	float MouseSensitivity;
	float Zoom;

	//character movement
	bool isOnTheRoof;
	bool isFallingDown;
	bool fastGetDown;
	const float maximumJumpHeight = 0.3;
	float jumpHeight;
	float fastFallingSpeed;
	float fallingSpeed = 1.5;
	const float maxSpeed = 3.0;
	float nearestRoofYPosition;
	const float characterHeight = 0.1;
	const float cameraDistance = 0.2;
	const int sizeOfCity = 30;

	void SetUpCharacterMovementParameters()
	{
		isOnTheRoof = true;
		isFallingDown = false;
		fastGetDown = false;
		fastFallingSpeed = MovementSpeed;

		StartPosition = Position = glm::vec3(sizeOfCity / 2, 12.0f, sizeOfCity / 2);
		nearestRoofYPosition = 0;
	}

	void moveCharater(float deltaTime)
	{
		float velocity = fallingSpeed * deltaTime;
		float horizontalVelocity = MovementSpeed * deltaTime;

		//go forward all the time
		glm::vec3 movementWithoutY = glm::vec3(velocity, 1, velocity);
		glm::vec3 blockY = glm::vec3(Front.x, 0, Front.z);

		Position += blockY * horizontalVelocity;

		if (!isOnTheRoof)
		{
			if (jumpHeight <= 0)//character is falling down
			{
				if (Position.y - velocity < nearestRoofYPosition)//to not to get into a block by the roof
					Position.y = nearestRoofYPosition;
				else
				{
					Position.y = Position.y - velocity;
				}
			}
			else if (jumpHeight > 0)
			{
				Position.y = Position.y + velocity;
				jumpHeight -= velocity;

			}
		}


	}

	// Constructor with vectors
	Camera(glm::vec3 position = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = StartPosition;
		//Position = position;
		WorldUp = up;
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
		SetUpCharacterMovementParameters();
	}
	// Constructor with scalar values
	Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(glm::vec3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM)
	{
		Position = StartPosition;
		//Position = glm::vec3(posX, posY, posZ);
		WorldUp = glm::vec3(upX, upY, upZ);
		Yaw = yaw;
		Pitch = pitch;
		updateCameraVectors();
		SetUpCharacterMovementParameters();
	}

	// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(Position, Position + Front, Up);
	}

	// Processes input received from any keyboard-like input system. Accepts input parameter in the form of camera defined ENUM (to abstract it from windowing systems)
	void ProcessKeyboard(Camera_Movement direction, float deltaTime)
	{
		glm::vec3 blockY = glm::vec3(Position.x, 1, Position.z);
		float velocity = MovementSpeed * deltaTime;
		if (direction == FORWARD)
		{
			if (MovementSpeed < maxSpeed && fallingSpeed < maxSpeed)
			{
				MovementSpeed += 0.01;
				fallingSpeed += 0.01;
			}

		}
		if (direction == BACKWARD)
		{
			if (MovementSpeed > 0 && fallingSpeed > 0)
			{
				MovementSpeed -= 0.01;
				fallingSpeed -= 0.01;
			}

		}
		/*
		if (direction == LEFT)
		Position -= Right * velocity;
		if (direction == RIGHT)
		Position += Right * velocity;*/
		if (direction == UPWARD)
			//	if(isOnTheRoof)
			jumpHeight = maximumJumpHeight;
	}

	// Processes input received from a mouse input system. Expects the offset value in both the x and y direction.
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true)
	{
		//Front.y = 1;

		xoffset *= MouseSensitivity;
		yoffset *= MouseSensitivity;

		Yaw += xoffset;
		Pitch += yoffset;

		// Make sure that when pitch is out of bounds, screen doesn't get flipped
		if (constrainPitch)
		{
			if (Pitch > 89.0f)
				Pitch = 89.0f;
			if (Pitch < -89.0f)
				Pitch = -89.0f;
		}

		// Update Front, Right and Up Vectors using the updated Euler angles
		updateCameraVectors();
	}

	// Processes input received from a mouse scroll-wheel event. Only requires input on the vertical wheel-axis
	void ProcessMouseScroll(float yoffset)
	{
		if (Zoom >= 1.0f && Zoom <= 45.0f)
			Zoom -= yoffset;
		if (Zoom <= 1.0f)
			Zoom = 1.0f;
		if (Zoom >= 45.0f)
			Zoom = 45.0f;
	}

private:
	// Calculates the front vector from the Camera's (updated) Euler Angles
	void updateCameraVectors()
	{
		// Calculate the new Front vector
		glm::vec3 front;
		front.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		front.y = sin(glm::radians(Pitch));
		front.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
		Front = glm::normalize(front);
		// Also re-calculate the Right and Up vector
		Right = glm::normalize(glm::cross(Front, WorldUp));  // Normalize the vectors, because their length gets closer to 0 the more you look up or down which results in slower movement.
		Up = glm::normalize(glm::cross(Right, Front));
	}
};
#endif
