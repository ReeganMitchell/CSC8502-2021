#include "Camera.h"
#include "Window.h"
#include <algorithm>
#include <cmath>

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

		float speed = 200.0f * dt;

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
		if (travel == 1000) {
			if (endYaw == 45.0f) {
				canMove = true;
			}
			else {
				startPosition = Vector3(16386 * 0.8, 2040, 16386 * 0.2);
				endPosition = Vector3(16386 * 0.8, 2040, 16386 * 0.8);
				startYaw = 135.0f;
				endYaw = 45.0f;
				travel = 0;
			}
		}
		else {
			Vector3 newPos = GetPosition();
			newPos.x = startPosition.x + ((travel / 1000) * (endPosition.x - startPosition.x));
			newPos.z = startPosition.z + ((travel / 1000) * (endPosition.z - startPosition.z));
			SetYaw(startYaw
				+ ((travel / 1000) * (endYaw - startYaw)));
			SetPosition(newPos);
			travel++;
		}
		
	}
	
}

Matrix4 Camera::BuildViewMatrix()
{
	return Matrix4::Rotation(-pitch, Vector3(1, 0, 0)) * Matrix4::Rotation(-yaw, Vector3(0, 1, 0)) * Matrix4::Translation(-position);
}

void Camera::FollowPath()
{

}
