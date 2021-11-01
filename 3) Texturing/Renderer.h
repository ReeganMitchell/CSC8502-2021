#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <nclgl\Camera.h>

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	virtual void RenderScene();

	void UpdateTextureMatrix(float rotation);
	void ToggleRepeating();
	void ToggleFiltering();
protected:
	Mesh* triangle;
	Shader* shader;
	GLuint texture;
	bool filtering;
	bool repeating;
};
