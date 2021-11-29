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
class Light;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);

	void RenderScene() override;
	void UpdateScene(float dt) override;
protected:
	void PresentScene();
	void DrawPostProcess();
	void DrawMainScene();
	void DrawShadowScene();

	void DrawHeightMap();
	void DrawWater();
	void DrawSkybox();

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex;

	GLuint sceneDiffuse;
	GLuint sceneBump;
	float sceneTime;

	GLuint shadowFBO;

	Light* light;
	Camera* camera;

	vector<Mesh*> sceneMeshes;
	vector<Matrix4> sceneTransforms;
};
