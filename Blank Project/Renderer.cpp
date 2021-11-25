#include "Renderer.h"
#include <algorithm>
#include <nclgl\OGLRenderer.cpp>

Renderer::Renderer(Window &parent) : OGLRenderer(parent)	{
	root = new SceneNode();
	camera = new Camera(00.0f, 0.0f, (Vector3(0, 100, 750.0f)));
	quad = Mesh::GenerateQuad();
	tree = Mesh::LoadFromMeshFile("OffsetCube.msh");

	heightMap = new HeightMap(TEXTUREDIR"islandHeightmap.png");
	islandTextures = new GLuint[4];
	islandTextures[0] = SOIL_load_OGL_texture(TEXTUREDIR"islandSand.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	islandTextures[1] = SOIL_load_OGL_texture(TEXTUREDIR"islandMud.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	islandTextures[2] = SOIL_load_OGL_texture(TEXTUREDIR"islandGrass.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	islandTextures[3] = SOIL_load_OGL_texture(TEXTUREDIR"islandRock.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	islandBumpmaps = new GLuint[4];
	islandBumpmaps[0] = SOIL_load_OGL_texture(TEXTUREDIR"islandSandNormal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	islandBumpmaps[1] = SOIL_load_OGL_texture(TEXTUREDIR"islandMudNormal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	islandBumpmaps[2] = SOIL_load_OGL_texture(TEXTUREDIR"islandGrassNormal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	islandBumpmaps[3] = SOIL_load_OGL_texture(TEXTUREDIR"islandRockNormal.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	islandMask = SOIL_load_OGL_texture(TEXTUREDIR"islandMask.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(islandMask, true);
	for (int i = 0; i < 4; i++) {
		SetTextureRepeating(islandTextures[i], true);
		SetTextureRepeating(islandBumpmaps[i], true);
	}

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	SetTextureRepeating(waterTex, true);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	Vector3 dimensions = heightMap->GetHeightMapSize();
	camera->SetPosition(dimensions * Vector3(0.5, 2.0, 0.5));
	light = new Light(Vector3(2.0f,5.0f,2.0f), Vector4(1, 1, 1, 1));

	islandShader = new Shader("IslandVertex.glsl", "IslandFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "ReflectFragment.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);

	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	waterRotate = 0.0f;
	waterCycle = 0.0f;
	init = true;
}
Renderer::~Renderer(void)	{
	//delete root;
	delete camera;
	delete heightMap;
	//delete island;
	delete reflectShader;
	glDeleteTextures(4, islandTextures);
	glDeleteTextures(4, islandBumpmaps);
	glDeleteTextures(1, &waterTex);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;

	root->Update(dt);
}

void Renderer::RenderScene()	{
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawScene();
	DrawPostProcess();
	PresentScene();
}

void Renderer::PresentScene() //combine buffers
{

}

void Renderer::DrawPostProcess()
{

}

void Renderer::DrawScene()
{
	DrawSkybox();
	DrawHeightMap();
	DrawWater();
	BuildNodeLists(root);
	DrawNodes();
	ClearNodeLists();
}

void Renderer::DrawNode(SceneNode* n)
{
	/*if (n->GetMesh()) {
		BindShader(shader);
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);

		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)& n->GetColour());

		texture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), texture);
		n->Draw(*this);
	}*/
}

void Renderer::DrawNodes()
{
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::BuildNodeLists(SceneNode* from)
{
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); i++) {
		BuildNodeLists((*i));
	}
}

void Renderer::SortNodeLists()
{
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);

	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::ClearNodeLists()
{
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawWater()
{
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightMapSize();

	hSize.y /= 5;

	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
}

void Renderer::DrawSkybox()
{
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap()
{
	BindShader(islandShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(islandShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	//Bind Textures

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex1"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, islandTextures[0]);

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex2"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, islandTextures[1]);

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex3"), 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, islandTextures[2]);

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex4"), 3);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, islandTextures[3]);

	//Bind Bumps
	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex1"), 4);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, islandBumpmaps[0]);

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex2"), 5);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_2D, islandBumpmaps[1]);

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex3"), 6);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(GL_TEXTURE_2D, islandBumpmaps[2]);

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex4"), 7);
	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, islandBumpmaps[3]);

	//Bind mask

	glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "mask"), 8);
	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, islandMask);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	heightMap->Draw();
}