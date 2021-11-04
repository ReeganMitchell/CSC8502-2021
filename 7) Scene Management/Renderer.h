#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <nclgl\Camera.h>
#include <nclgl\SceneNode.h>
#include <nclgl\CubeRobot.h>

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;
protected:
	void DrawNode(SceneNode* n);

	Mesh* cube;
	Shader* shader;
	Camera* camera;
	SceneNode* root;
};
