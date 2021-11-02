#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <nclgl\Camera.h>

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;

	void UpdateTextureMatrix(float rotation);
	
	void ToggleScissor();
	void ToggleStencil();
protected:
	Mesh* meshes[2];
	Shader* shader;
	GLuint textures[2];
	Vector3 positions[2];

	bool usingScissor;
	bool usingStencil;
};
