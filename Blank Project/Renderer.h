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
	void DrawNode(SceneNode* n);
	void DrawNodes();

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

	//Deffered rendering stuff
	GLuint bufferFBO; //FBO for our G-Buffer pass
	GLuint bufferColourTex; // Albedo goes here
	GLuint bufferNormalTex; // Normals go here
	GLuint bufferDepthTex; // Depth goes here

	Shader* sceneShader;
	Shader* shadowShader;

	GLuint shadowTex[6];
	GLuint shadowFBO[6];

	GLuint sceneDiffuse;
	GLuint sceneBump;
	float sceneTime;


	Light* light;
	Camera* camera;
	Frustum frameFrustum;

	vector<Mesh*> sceneMeshes;
	vector<Matrix4> sceneTransforms;
};
