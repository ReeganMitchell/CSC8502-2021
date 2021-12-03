#pragma once
#include "OGLRenderer.h"

struct Particle {
    Vector2 Position, Velocity;
    Vector4 Colour;
    float Life;

    Particle() : Position(0.0f,0.0f), Velocity(0.0f, 0.0f), Colour(1.0f, 1.0f, 1.0f, 1.0f), Life(0.0f) { }
};

class ParticleGenerator
{
public:
    // constructor
    ParticleGenerator(Shader* shader, GLuint texture, unsigned int amount);
    // update all particles
    void Update(float dt, unsigned int newParticles, Vector2 offset = Vector2(0.0f, 0.0f));
    // render all particles
    void Draw();
protected:
    // state
    std::vector<Particle> particles;
    Vector2 pos;
    unsigned int amount;
    // render state
    Shader* shader;
    GLuint texture;
    unsigned int VAO;
    // initializes buffer and vertex attributes
    void init();
    // returns the first Particle index that's currently unused e.g. Life <= 0.0f or 0 if no particle is currently inactive
    unsigned int firstUnusedParticle();
    // respawns particle
    void respawnParticle(Particle& particle, Vector2 offset = Vector2(0.0f, 0.0f));
};