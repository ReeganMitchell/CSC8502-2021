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

	//Scene management stuff
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	SceneNode* root;

	//heightmap stuff
	HeightMap* heightMap;

	//skybox stuff
	Shader* skyboxShader;
	GLuint cubeMap;

	//water stuff
	GLuint waterTex;
	Shader* reflectShader;
	float waterRotate;
	float waterCycle;

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex;

	GLuint sceneDiffuse;
	GLuint sceneBump;
	float sceneTime;

	GLuint shadowFBO;

	Light* light;
	Camera* camera;
	Frustum frameFrustum;

	vector<Mesh*> sceneMeshes;
	vector<Matrix4> sceneTransforms;
};
