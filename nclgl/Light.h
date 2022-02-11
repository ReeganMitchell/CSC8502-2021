#pragma once

#include "Vector3.h"
#include "Vector4.h"

class Light {
public:
	Light(){}
	Light(const Vector3& position, const Vector4& colour, float radius) {
		this->position = position;
		this->colour = colour;
		this->radius = radius;
		direction = Vector3(0,0,0);
	}
	Light(const Vector3& direction, const Vector4& colour) { //for creating directional light / sun
		this->direction = direction;
		this->colour = colour;
		this->position = Vector3(0, 4000, 0);
		this->radius = 0;
	}

	~Light(void) {};

	Vector3 GetPosition() const { return position; }
	void SetPosition(const Vector3& val) { position = val; }

	Vector3 GetDirection() const { return direction; }
	void SetDirection(const Vector3& val) { direction = val; }

	float GetRadius() const { return radius; }
	void SetRadius(float val) { radius = val; }

	Vector4 GetColour() const { return colour; }
	void SetColour(const Vector4& val) { colour = val; }

protected:
	Vector3 position;
	float radius;
	Vector4 colour;
	Vector3 direction;
};