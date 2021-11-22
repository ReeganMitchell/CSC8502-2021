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
	void DrawNode(SceneNode* n);
	void DrawNodes();
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();

	void DrawWater();
	void DrawSkybox();
	void DrawHeightMap();

	Light* light;
	Mesh* quad;
	Mesh* cube;
	Shader* shader;
	Camera* camera;
	SceneNode* root;
	GLuint texture;

	//heightmap
	SceneNode* island;
	HeightMap* heightMap;
	GLuint* islandTextures;
	GLuint* islandBumpmaps;

	Shader* skyboxShader;
	Shader* reflectShader;
	GLuint waterTex;
	GLuint cubeMap;

	Frustum frameFrustum;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;

	float waterRotate;
	float waterCycle;
};
