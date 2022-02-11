#pragma once
#include "Matrix4.h"
#include "Vector3.h"

class Camera {
public:
	Camera(void) {
		yaw = 0.0f;
		pitch = 0.0f;

		canMove = false;
		nextPoint = 1;
	};

	Camera(float pitch, float yaw, Vector3 position) {
		this->pitch = pitch;
		this->yaw = yaw;
		this->position = position;

		canMove = false;
		nextPoint = 1;
	}
	~Camera(void) {};

	void UpdateCamera(float dt = 1.0f);

	Matrix4 BuildViewMatrix();

	Vector3 GetPosition() const { return position; }
	void SetPosition(Vector3 val) { position = val; }

	float GetYaw() const { return yaw; }
	void SetYaw(float y) { yaw = y; }

	float GetPitch() const { return pitch; }
	void SetPitch(float p) { pitch = p; }
protected:
	float yaw;
	float pitch;
	Vector3 position;

	bool canMove;

	Vector3 positions[4] = {
		Vector3(400.0f, 40.0f, 420.0f),
		Vector3(340.0f, 100.0f, 520.0f),
		Vector3(230.0f, 10.0f, 420.0f),
		Vector3(204.0f, 6.0f, 410.0f)
	};
	float yaws[4] = {
		-45.0f,40.0f,-5.0f,-45.0f
	};
	float pitches[4]{
		0.0,-30.0,-10.0,0.0
	};

	float travel = 0;
	int nextPoint;
};