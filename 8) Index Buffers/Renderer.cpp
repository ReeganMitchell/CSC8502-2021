#include "Renderer.h"
#include <nclgl/CubeRobot.h>
#include <algorithm>

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	
	camera = new Camera(0.0f, 0.0f, (Vector3(0, 100, 750.0f)));
	heightMap = new HeightMap(TEXTUREDIR"noise.png");

	Vector3 dimensions = heightMap->GetHeightMapSize();
	camera->SetPosition(dimensions * Vector3(0.5,2,0.5));

	terrainTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	shader = new Shader("TexturedVertex.glsl", "TexturedFragment.glsl");
	if (!shader->LoadSuccess() || !terrainTex) {
		return;
	}

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);
	SetTextureRepeating(terrainTex, true);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	init = true;
}
Renderer::~Renderer(void) {
	delete shader;
	delete camera;
	delete heightMap;
	glDeleteTextures(1, &terrainTex);
}

void  Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
}

void Renderer::RenderScene() {
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);


	BindShader(shader);
	UpdateShaderMatrices();

	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTex);
	heightMap->Draw();
}
