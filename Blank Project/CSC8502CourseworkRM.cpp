#include "../NCLGL/window.h"
#include "Renderer.h"

int main()	{
	int numFrames = 0;
	Window w("CSC8502CourseworkRM", 1280, 720, false);

	if(!w.HasInitialised()) {
		return -1;
	}
	
	Renderer renderer(w);
	if(!renderer.HasInitialised()) {
		return -1;
	}

	while(w.UpdateWindow()  && !Window::GetKeyboard()->KeyDown(KEYBOARD_ESCAPE)){
		float delta = w.GetTimer()->GetTimeDeltaSeconds();

		renderer.UpdateScene(w.GetTimer()->GetTimeDeltaSeconds());

		renderer.RenderScene();
		renderer.SwapBuffers();
		if (Window::GetKeyboard()->KeyDown(KEYBOARD_F5)) {
			Shader::ReloadAllShaders();
		}

		if (Window::GetKeyboard()->KeyTriggered(KEYBOARD_1)) {
			renderer.ToggleGamma();
		}
		numFrames++;
		w.SetTitle("Framerate: " + std::to_string(1 / delta));
	}
	return 0;
}