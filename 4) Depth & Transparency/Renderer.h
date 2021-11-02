#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <nclgl\Camera.h>

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	virtual void RenderScene();

	void UpdateTextureMatrix(float rotation);
	void ToggleObject();
	void ToggleDepth();
	void ToggleAlphaBlend();
	void ToggleBlendMode();
	void MoveObject(float by);
protected:
	Mesh* meshes[2];
	Shader* shader;
	GLuint textures[2];
	Vector3 positions[2];

	bool modifyObject;
	bool usingDepth;
	bool usingAlpha;
	int blendMode;
};
