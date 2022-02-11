#pragma once
#include "../NCLGL/OGLRenderer.h"
#include <nclgl\Camera.h>
#include <nclgl\SceneNode.h>
#include <nclgl\Frustum.h>
#include <nclgl\HeightMap.h>
#include <nclgl/ParticleGenerator.h>

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
	void ToggleGamma();
protected:
	void PresentScene();
	void DrawPostProcess();
	void DrawMainScene();
	void DrawShadowScene();
	void DrawWater();
	void DrawSkybox();

	//Scene management stuff
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNode(SceneNode* n);
	void DrawNodes();

	void addProps();
	vector<GLuint> propTextures;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;
	SceneNode* root;
	bool renderShadows;

	//heightmap stuff
	void DrawHeightMap();
	HeightMap* heightMap;
	Shader* islandShader;
	GLuint* islandTextures;
	GLuint* islandBumpmaps;
	GLuint islandMask;

	//skybox stuff
	Shader* skyboxShader;
	GLuint cubeMap;

	//water stuff
	GLuint waterTex;
	Shader* reflectShader;
	float waterRotate;
	float waterCycle;

	//particle stuff
	ParticleGenerator* fire;
	Shader* particleShader;

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

	bool gCorrection;

	Light* light;
	Camera* camera;
	Frustum frameFrustum;

	vector<Mesh*> sceneMeshes;
	vector<Matrix4> sceneTransforms;
};
