#include "Camera.h"
#include "Window.h"
#include <algorithm>

void Camera::UpdateCamera(float dt)
{
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_W) | Window::GetKeyboard()->KeyDown(KEYBOARD_S) | Window::GetKeyboard()->KeyDown(KEYBOARD_A) | Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
		canMove = true;
	}
	if (canMove) {
		pitch -= (Window::GetMouse()->GetRelativePosition().y);
		yaw -= (Window::GetMouse()->GetRelativePosition().x);

		pitch = std::min(pitch, 90.0f);
		pitch = std::max(pitch, -90.0f);

		if (yaw < 0) {
			yaw += 360.0f;
		}
		if (yaw > 360.0f) {
			yaw -= 360.0f;
		}

		Matrix4 rotation = Matrix4::Rotation(yaw, Vector3(0, 1, 0));
		Vector3 forward = rotation * Vector3(0, 0, -1);
		Vector3 right = rotation * Vector3(1, 0, 0);

		float speed = 30.0f * dt;

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_W)) {
			position += forward * speed;
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_S)) {
			position -= forward * speed;
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_A)) {
			position -= right * speed;
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_D)) {
			position += right * speed;
		}

		if (Window::GetKeyboard()->KeyDown(KEYBOARD_SHIFT)) {
			position.y -= speed;
		}
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_SPACE)) {
			position.y += speed;
		}
	}
	else {
		if (nextPoint == 4) {
			canMove = true;
		}
		else {
			if (travel == 1000) {
				nextPoint++;
				travel = 0;
			}
			else {
				Vector3 newPos = GetPosition();
				newPos.x = positions[nextPoint - 1].x + ((travel / 1000) * (positions[nextPoint].x - positions[nextPoint - 1].x));
				newPos.y = positions[nextPoint - 1].y + ((travel / 1000) * (positions[nextPoint].y - positions[nextPoint - 1].y));
				newPos.z = positions[nextPoint - 1].z + ((travel / 1000) * (positions[nextPoint].z - positions[nextPoint - 1].z));
				SetPosition(newPos);

				SetYaw(yaws[nextPoint - 1]
					+ ((travel / 1000) * (yaws[nextPoint] - yaws[nextPoint - 1])));
				SetPitch(pitches[nextPoint - 1] + ((travel / 1000) * (pitches[nextPoint] - pitches[nextPoint - 1])));

				travel++;
			}
		}
	}
	
}

Matrix4 Camera::BuildViewMatrix()
{
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) * Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) * Matrix4::Translation(-position);
}
