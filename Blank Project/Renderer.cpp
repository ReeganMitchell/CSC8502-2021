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
	root = new SceneNode();
	camera = new  Camera(0.0f, 315.0f, Vector3(200.0f, 6.0f, 500.0f));
	light = new  Light(Vector3(224.0f, 4.2f, 400.0f),Vector4(1, 1, 1, 1), 250.0f);

	sceneShader = new Shader("ShadowSceneVertex.glsl", "ShadowSceneFragment.glsl");
	shadowShader = new Shader("ShadowVertex.glsl", "ShadowFragment.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "ReflectFragment.glsl");
	islandShader = new Shader("ShadowSceneVertex.glsl", "islandFragment.glsl");

	gCorrection = false;
	renderShadows = false;

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


	sceneDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/big_table.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
	sceneBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg", TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

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

	if (!sceneDiffuse || !sceneBump || !cubeMap || !waterTex) {
		return;
	}

	SetTextureRepeating(sceneDiffuse, true);
	SetTextureRepeating(sceneBump, true);
	SetTextureRepeating(waterTex, true);

	//Initialise Particle generator
	//fire = new ParticleGenerator(particleShader, sceneDiffuse, 1000);



	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Vector3  heightmapSize = heightMap->GetHeightMapSize();

	sceneTransforms.resize(3);
	sceneTransforms[0] = Matrix4::Rotation(90, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(10, 10, 1));
	sceneTransforms[2] = Matrix4::Translation(Vector3(0,-20,0)) * Matrix4::Scale(Vector3(0.05, 0.3, 0.05));
	sceneTransforms[1] = Matrix4::Translation(Vector3(heightmapSize.x /40, 0.0f, heightmapSize.z /40)) * Matrix4::Rotation(90, Vector3(1, 0, 0)) * Matrix4::Scale(Vector3(heightmapSize.x / 40, heightmapSize.x / 40, 1));
	SceneNode* island = new SceneNode(sceneMeshes[2]);
	island->SetTransform(sceneTransforms[2]);
	island->SetShader(islandShader);
	island->SetBoundingRadius(2000.0f);
	root->AddChild(island);

	addProps();

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

void Renderer::ToggleGamma() {
	gCorrection = !gCorrection;
}

void  Renderer::UpdateScene(float dt) {
	sceneTime += dt;
	camera->UpdateCamera(dt);
	root->Update(dt);
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
	viewMatrix = camera->BuildViewMatrix();


	frameFrustum.FromMatrix(projMatrix * viewMatrix);

	waterRotate += dt * 2.0f;
	waterCycle += dt * 0.25f;
}

void Renderer::PresentScene()
{
	
}

void Renderer::DrawPostProcess()
{
	
}

void Renderer::DrawMainScene()
{
	renderShadows = false;
	//DrawHeightMap();
	DrawNodes();
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

		renderShadows = true;
		DrawNodes();
	}
	
	glColorMask(GL_TRUE , GL_TRUE , GL_TRUE , GL_TRUE );
	glViewport( 0, 0, width , height  );
	glBindFramebuffer(GL_FRAMEBUFFER , 0);
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
	glDisable(GL_FRAMEBUFFER_SRGB);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	BuildNodeLists(root);
	SortNodeLists();

	DrawSkybox();
	DrawShadowScene();
	if (gCorrection) {
		glEnable(GL_FRAMEBUFFER_SRGB);
	}
	DrawMainScene();
	DrawWater();

	ClearNodeLists();
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

void Renderer::DrawNodes()
{
	for (const auto& i : nodeList) {
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::addProps()
{
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("chair_04.msh"));//3
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("big_table.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("parasol.msh"));//5
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("fire_camp.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("float_02.msh"));//7
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("cooler.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("palm_tree.msh"));//9

	propTextures.emplace_back(SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/chair_04.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y));
	propTextures.emplace_back(SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/big_table.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y));
	propTextures.emplace_back(SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/parasol.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y));
	propTextures.emplace_back(SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/fire_camp.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y));
	propTextures.emplace_back(SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/float_02.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y));
	propTextures.emplace_back(SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/cooler.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y));
	propTextures.emplace_back(SOIL_load_OGL_texture(TEXTUREDIR"beach props/textures/treediffuse.png", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y));


	//Place objects into scene graph

	SceneNode* firepit = new SceneNode(sceneMeshes[6]);
	firepit->SetTransform(Matrix4::Translation(Vector3(224.0f, 2.2f, 400.0f)) * Matrix4::Rotation(8,Vector3(0,0,1)) * Matrix4::Scale(Vector3(0.5,0.5,0.5) * 2));
	firepit->SetShader(sceneShader);
	firepit->SetTexture(propTextures[3]);
	firepit->SetBoundingRadius(15.0f);
	root->AddChild(firepit);

	SceneNode* cooler = new SceneNode(sceneMeshes[8]);
	cooler->SetTransform(Matrix4::Translation(Vector3(228.0f, 2.1f, 408.0f)) * Matrix4::Scale(Vector3(1,1,1) * 3));
	cooler->SetShader(sceneShader);
	cooler->SetTexture(propTextures[5]);
	cooler->SetBoundingRadius(5.0f);
	root->AddChild(cooler);

	SceneNode* parasol1 = new SceneNode(sceneMeshes[5]);
	parasol1->SetTransform(Matrix4::Translation(Vector3(229.0f, 2.0f, 409.0f)) * Matrix4::Rotation(-8, Vector3(0, 0, 1)) * Matrix4::Scale(Vector3(1, 1, 1) * 3));
	parasol1->SetShader(sceneShader);
	parasol1->SetTexture(propTextures[5]);
	parasol1->SetBoundingRadius(5.0f);
	root->AddChild(parasol1);

	SceneNode* chair1 = new SceneNode(sceneMeshes[3]);
	chair1->SetTransform(Matrix4::Translation(Vector3(222.0f, 2.0f, 380.0f)) * Matrix4::Rotation(8, Vector3(0, 0, 1)) * Matrix4::Rotation(-90, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(2, 2, 2) * 3));
	chair1->SetShader(sceneShader);
	chair1->SetTexture(propTextures[0]);
	chair1->SetBoundingRadius(5.0f);
	root->AddChild(chair1);

	SceneNode* floater = new SceneNode(sceneMeshes[7]);
	floater->SetTransform(Matrix4::Translation(Vector3(190.0f, 0.0f, 390.0f)) * Matrix4::Scale(Vector3(1.5, 1.5, 1.5) * 3));
	floater->SetShader(sceneShader);
	floater->SetTexture(propTextures[4]);
	floater->SetBoundingRadius(5.0f);
	root->AddChild(floater);

	Matrix4 treeTransforms[8] = {
		Matrix4::Translation(Vector3(295.0f, 22.0f, 380.0f)) * Matrix4::Rotation(-90, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(2, 1.5, 2) * 3),
		Matrix4::Translation(Vector3(305.0f, 22.0f, 403.0f)) * Matrix4::Rotation(-37, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(2, 1.5, 2) * 3),
		Matrix4::Translation(Vector3(295.0f, 17.0f, 390.0f)) * Matrix4::Rotation(21, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(2, 1.5, 2) * 3),
		Matrix4::Translation(Vector3(290.0f, 13.0f, 423.0f)) * Matrix4::Rotation(69, Vector3(0, 1, 0)) * Matrix4::Scale(Vector3(2, 1.5, 2) * 3),
		Matrix4::Translation(Vector3(345.0f, 22.0f, 380.0f))* Matrix4::Rotation(-90, Vector3(0, 1, 0))* Matrix4::Scale(Vector3(2, 1.5, 2) * 3),
		Matrix4::Translation(Vector3(365.0f, 22.0f, 403.0f))* Matrix4::Rotation(-37, Vector3(0, 1, 0))* Matrix4::Scale(Vector3(2, 1.5, 2) * 3),
		Matrix4::Translation(Vector3(325.0f, 17.0f, 390.0f))* Matrix4::Rotation(21, Vector3(0, 1, 0))* Matrix4::Scale(Vector3(2, 1.5, 2) * 3),
		Matrix4::Translation(Vector3(310.0f, 13.0f, 423.0f))* Matrix4::Rotation(69, Vector3(0, 1, 0))* Matrix4::Scale(Vector3(2, 1.5, 2) * 3)
	};

	SceneNode* treeCluster = new SceneNode();
	treeCluster->SetTransform(Matrix4::Translation(Vector3(0.0f, 0.0f, 0.0f)));
	treeCluster->SetBoundingRadius(2000.0f);

	for (int i = 0; i < 8; i++) {
		SceneNode* tree = new SceneNode(sceneMeshes[9]);
		tree->SetTransform(treeTransforms[i]);
		tree->SetShader(sceneShader);
		tree->SetTexture(propTextures[6]);
		tree->SetBoundingRadius(25.0f);
		tree->SetColour(Vector4(1, 1, 1, 0));
		treeCluster->AddChild(tree);
	}
	root->AddChild(treeCluster);
}


void Renderer::DrawNode(SceneNode* n)
{
	if (n->GetMesh()) {
		if (renderShadows) {
			modelMatrix = n->GetWorldTransform();
			UpdateShaderMatrices();
			n->Draw(*this);
		}
		else {
			Shader* s = n->GetShader();
			BindShader(s);
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
			projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);
			shadowMatrix = shadowMatrices[0];

			glUniform1i(glGetUniformLocation(s->GetProgram(), "diffuseTex"), 0);
			//glUniform1i(glGetUniformLocation(s->GetProgram(), "bumpTex"), 1);
			glUniform1i(glGetUniformLocation(s->GetProgram(), "shadowTex1"), 2);
			glUniform1i(glGetUniformLocation(s->GetProgram(), "shadowTex2"), 3);
			glUniform1i(glGetUniformLocation(s->GetProgram(), "shadowTex3"), 4);
			glUniform1i(glGetUniformLocation(s->GetProgram(), "shadowTex4"), 5);
			glUniform1i(glGetUniformLocation(s->GetProgram(), "shadowTex5"), 6);
			glUniform1i(glGetUniformLocation(s->GetProgram(), "shadowTex6"), 7);
			glUniform3fv(glGetUniformLocation(s->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

			if (n->GetTexture()) {
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, n->GetTexture());
			}
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, sceneBump);
			for (int i = 0; i < 6; i++) {
				glActiveTexture(GL_TEXTURE2 + i);
				glBindTexture(GL_TEXTURE_2D, shadowTex[i]);
			}

			modelMatrix = n->GetTransform();
			UpdateShaderMatrices();
			glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "shadowMatrix[0]"), 1, false, shadowMatrices[0].values);
			glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "shadowMatrix[1]"), 1, false, shadowMatrices[1].values);
			glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "shadowMatrix[2]"), 1, false, shadowMatrices[2].values);
			glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "shadowMatrix[3]"), 1, false, shadowMatrices[3].values);
			glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "shadowMatrix[4]"), 1, false, shadowMatrices[4].values);
			glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "shadowMatrix[5]"), 1, false, shadowMatrices[5].values);

			if (s == islandShader) {
				//Bind Textures

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex1"), 8);
				glActiveTexture(GL_TEXTURE8);
				glBindTexture(GL_TEXTURE_2D, islandTextures[0]);

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex2"), 9);
				glActiveTexture(GL_TEXTURE9);
				glBindTexture(GL_TEXTURE_2D, islandTextures[1]);

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex3"), 10);
				glActiveTexture(GL_TEXTURE10);
				glBindTexture(GL_TEXTURE_2D, islandTextures[2]);

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "diffuseTex4"), 11);
				glActiveTexture(GL_TEXTURE11);
				glBindTexture(GL_TEXTURE_2D, islandTextures[3]);

				//Bind Bumps
				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex1"), 12);
				glActiveTexture(GL_TEXTURE12);
				glBindTexture(GL_TEXTURE_2D, islandBumpmaps[0]);

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex2"), 13);
				glActiveTexture(GL_TEXTURE13);
				glBindTexture(GL_TEXTURE_2D, islandBumpmaps[1]);

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex3"), 14);
				glActiveTexture(GL_TEXTURE14);
				glBindTexture(GL_TEXTURE_2D, islandBumpmaps[2]);

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "bumpTex4"), 15);
				glActiveTexture(GL_TEXTURE15);
				glBindTexture(GL_TEXTURE_2D, islandBumpmaps[3]);

				//Bind mask

				glUniform1i(glGetUniformLocation(islandShader->GetProgram(), "mask"), 16);
				glActiveTexture(GL_TEXTURE16);
				glBindTexture(GL_TEXTURE_2D, islandMask);
			}

			n->Draw(*this);
		}
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
