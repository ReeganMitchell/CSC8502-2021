#include "Renderer.h"
#include <nclgl/CubeRobot.h>
#include <algorithm>
#include <nclgl/MeshAnimation.h>
#include <nclgl/MeshMaterial.h>
#include <nclgl/Light.h>
#include <nclgl/Camera.h>

const int POST_PASSES = 10;
#define SHADOWSIZE 2048

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	camera = new  Camera(-30.0f, 315.0f, Vector3(-8.0f, 5.0f, 8.0f));
	light = new  Light(Vector3(0.0f, 10.0f, 0.0f),Vector4(1, 1, 1, 1), 250.0f);

	sceneShader = new Shader("ShadowSceneVertex.glsl", "ShadowSceneFragment.glsl");
	shadowShader = new Shader("ShadowVertex.glsl", "ShadowFragment.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "ReflectFragment.glsl");

	if (!sceneShader->LoadSuccess() || !shadowShader->LoadSuccess() || !skyboxShader->LoadSuccess()) {
		return;
	}

	glGenTextures(6, shadowTex);
	glGenFramebuffers(6, shadowFBO);

	for (int i = 0; i < 6; i++) {
		glBindTexture(GL_TEXTURE_2D, shadowTex[i]);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO[i]);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex[i], 0);
		glDrawBuffer(GL_NONE);
	}
	glBindTexture(GL_TEXTURE_2D, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	sceneMeshes.emplace_back(Mesh::GenerateQuad());
	sceneMeshes.emplace_back(Mesh::GenerateQuad());
	heightMap = new HeightMap(TEXTUREDIR"islandHeightmap.png");
	sceneMeshes.emplace_back(heightMap);
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cone.msh"));

	sceneDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sceneBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!sceneDiffuse || !sceneBump || !cubeMap || !waterTex) {
		return;
	}

	SetTextureRepeating(sceneDiffuse, true);
	SetTextureRepeating(sceneBump, true);
	SetTextureRepeating(waterTex, true);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vector3  heightmapSize = heightMap->GetHeightMapSize();

	sceneTransforms.resize(6);
	sceneTransforms[0] = Matrix4::Rotation(90, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(10, 10, 1));
	sceneTransforms[2] = Matrix4::Translation(Vector3(0,-20,0)) * Matrix4::Scale(Vector3(0.1, 0.5, 0.1));
	sceneTransforms[1] = Matrix4::Translation(Vector3(heightmapSize.x /20, 0, heightmapSize.z /20)) * Matrix4::Rotation(90, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(heightmapSize.x / 20, heightmapSize.x / 20, 1));
	sceneTime = 0.0f;
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	init = true;
}
Renderer::~Renderer(void) {
	glDeleteTextures(6, shadowTex);
	glDeleteFramebuffers(6, shadowFBO);

	for (auto& i : sceneMeshes) {
		delete i;
	}

	delete shadowShader;
	delete sceneShader;
	delete camera;
	delete light;
}

void  Renderer::UpdateScene(float dt) {
	sceneTime += dt;
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();

	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;

	for (int i = 3; i < 6; i++) {
		Vector3 t = Vector3(-10 + (5 * i), 2.0f + sin(sceneTime * i), 0);
		sceneTransforms[i] = Matrix4::Translation(t) * Matrix4::Rotation(sceneTime * 10 * i, Vector3(1, 0, 0));
	}
}

void Renderer::PresentScene()
{
	
}

void Renderer::DrawPostProcess()
{
	
}

void Renderer::DrawMainScene()
{
	BindShader(sceneShader);
	SetShaderLight(*light); 
	projMatrix = Matrix4::Perspective(1, 100, 1, 90);
	Matrix4 shadowMatrices[6] = {
		projMatrix * Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(1, 0, 0)),
		projMatrix * Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(-1, 0, 0)),
		projMatrix * Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, 1, 0)),
		projMatrix * Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, -1, 0)),
		projMatrix * Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, 0, 1)),
		projMatrix * Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, 0, -1))
	};
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f,(float)width / (float)height, 45.0f);
	shadowMatrix = shadowMatrices[0];

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0); 
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex1"), 2);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex2"), 3);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex3"), 4);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex4"), 5);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex5"), 6);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex6"), 7);
	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)& camera->GetPosition());

	glActiveTexture(GL_TEXTURE0); 
	glBindTexture(GL_TEXTURE_2D, sceneDiffuse); 
	glActiveTexture(GL_TEXTURE1); 
	glBindTexture(GL_TEXTURE_2D, sceneBump); 
	for (int i = 0; i < 6; i++) {
		glActiveTexture(GL_TEXTURE2 + i);
		glBindTexture(GL_TEXTURE_2D, shadowTex[i]);
	}

	for (int i = 2; i < 6; ++i) { 
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "shadowMatrix[0]"), 1, false, shadowMatrices[0].values);
		glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "shadowMatrix[1]"), 1, false, shadowMatrices[1].values);
		glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "shadowMatrix[2]"), 1, false, shadowMatrices[2].values);
		glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "shadowMatrix[3]"), 1, false, shadowMatrices[3].values);
		glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "shadowMatrix[4]"), 1, false, shadowMatrices[4].values);
		glUniformMatrix4fv(glGetUniformLocation(sceneShader->GetProgram(), "shadowMatrix[5]"), 1, false, shadowMatrices[5].values);
		sceneMeshes[i]->Draw(); 
	}
}

void Renderer::DrawShadowScene()
{
	projMatrix = Matrix4::Perspective(1, 100, 1, 90);
	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
	shadowMatrix = projMatrix * viewMatrix; //used  later
	Matrix4 viewMatrices[6] = {
		Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(1, 0, 0)),
		Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(-1, 0, 0)),
		Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, 1, 0)),
		Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, -1, 0)),
		Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, 0, 1)),
		Matrix4::BuildViewMatrix(light->GetPosition(), light->GetPosition() + Vector3(0, 0, -1))
	};
	for (int i = 0; i < 6; i++) {
		glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO[i]);

		glClear(GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

		BindShader(shadowShader);

		viewMatrix = viewMatrices[i];
		//shadowMatrix = shadowMatrices[i]; //used  later



		for (int i = 2; i < 6; ++i) {
			modelMatrix = sceneTransforms[i];
			UpdateShaderMatrices();
			sceneMeshes[i]->Draw();
		}
	}
	
	glColorMask(GL_TRUE , GL_TRUE , GL_TRUE , GL_TRUE );
	glViewport( 0, 0, width , height  );
	glBindFramebuffer(GL_FRAMEBUFFER , 0);
}

void Renderer::DrawHeightMap()
{
	BindShader(sceneShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneDiffuse);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneBump);

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
	glActiveTexture(GL_TEXTURE2);
	//glBindTexture(GL_TEXTURE_2D, shadowTex);

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();
	SetShaderLight(*light);
	heightMap->Draw();
}

void Renderer::DrawWater()
{
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix = sceneTransforms[1];
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	sceneMeshes[1]->Draw();
}

void Renderer::DrawSkybox()
{
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	sceneMeshes[0]->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	DrawShadowScene();
	//DrawHeightMap();
	DrawMainScene();
	DrawWater();
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

//void Renderer::DrawNodes()
//{
//	for (const auto& i : nodeList) {
//		DrawNode(i);
//	}
//	for (const auto& i : transparentNodeList) {
//		DrawNode(i);
//	}
//}


//void Renderer::DrawNode(SceneNode* n)
//{
//	if (n->GetMesh()) {
//
//		BindShader(sceneShader);
//		SetShaderLight(*light);
//		viewMatrix = camera->BuildViewMatrix();
//		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
//
//		glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
//		glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
//		glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
//		glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
//		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
//
//		glActiveTexture(GL_TEXTURE0);
//		glBindTexture(GL_TEXTURE_2D, sceneDiffuse);
//		glActiveTexture(GL_TEXTURE1);
//		glBindTexture(GL_TEXTURE_2D, sceneBump);
//		glActiveTexture(GL_TEXTURE2);
//		glBindTexture(GL_TEXTURE_2D, shadowTex);
//
//		n->Draw(*this);
//	}
//}