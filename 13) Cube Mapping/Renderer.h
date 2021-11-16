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
	void DrawScene();

	void DrawHeightMap();
	void DrawWater();
	void DrawSkybox();

	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;

	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;

	GLuint bufferFBO;
	GLuint processFBO;
	GLuint bufferColourTex[2];
	GLuint bufferDepthTex;

	HeightMap* heightMap;
	Mesh* quad;
	Light* light;
	Camera* camera;

	int currentFrame;
	float FrameTime;
	float waterRotate;
	float waterCycle;
};
