#include "ParticleGenerator.h"
#include "OGLRenderer.cpp"

ParticleGenerator::ParticleGenerator(Shader* shader, GLuint texture, unsigned int amount)
{
    this->shader = shader;
    this->texture = texture;
    this->amount = amount;
}

void ParticleGenerator::Update(float dt, unsigned int newParticles, Vector2 offset)
{
	for (unsigned int i = 0; i < newParticles; i++) {
		int unusedParticle = this->firstUnusedParticle();
		this->respawnParticle(this->particles[unusedParticle], offset);
	}

    for (unsigned int i = 0; i < this->amount; ++i)
    {
        Particle& p = this->particles[i];
        p.Life -= dt; // reduce life
        if (p.Life > 0.0f)
        {	// particle is alive, thus update
            p.Position.x -= p.Velocity.x * dt;
            p.Position.y -= p.Velocity.y * dt;
            p.Colour.w -= dt * 2.5f;
        }
    }
}

void ParticleGenerator::Draw()
{
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    glBindVertexArray(this->VAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

unsigned int ParticleGenerator::firstUnusedParticle()
{
	return 0;
}

void ParticleGenerator::respawnParticle(Particle& particle, Vector2 offset)
{
}
