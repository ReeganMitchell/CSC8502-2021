#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <nclgl\Camera.h>
#include <nclgl\SceneNode.h>
#include <nclgl\Frustum.h>
#include <nclgl\HeightMap.h>

class Camera;
class SceneNode;
class Mesh;
class Shader;
class HeightMap;
class MeshMaterial;
class MeshAnimation;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;
protected:
	Shader* shader;
	Camera* camera;
	GLuint terrainTex;
	HeightMap* heightMap;
	Mesh* mesh;
	MeshMaterial* material;
	vector<GLuint> matTextures;
	MeshAnimation* anim;

	int currentFrame;
	float FrameTime;
};
