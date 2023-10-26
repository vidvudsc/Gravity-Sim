#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <math.h>

#define NUM_PARTICLES 10000

bool followSelectedParticle = false;

typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    float mass;
    float radius;
    Color color;
    struct Particle* stuckWith;  // The particle it is stuck with, NULL if not stuck
} Particle;


Particle particles[NUM_PARTICLES];
Particle* selectedParticle = NULL;

Vector2 cameraOffset = {0};
float zoom = 1.0f;
bool paused = false;
bool showInfo = false;

void InitializeParticles() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        // Special case for the "star" particle at the center
        if (i == 0) {
            particles[i].position = (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2};
            particles[i].velocity = (Vector2){0, 0};
            particles[i].radius = 1;  // Large size in km
            particles[i].mass = particles[i].radius * 10000;  // Large mass
            particles[i].color = YELLOW;  // Star color
            continue;
        }
        
        particles[i].position = (Vector2){GetRandomValue(0, GetScreenWidth()), GetRandomValue(0, GetScreenHeight())};
        particles[i].velocity = (Vector2){0, 0};
        particles[i].radius = GetRandomValue(1, 10); // Random size in km
        particles[i].mass = particles[i].radius * 5500; // Assuming density is 5500 kg/km^3
        particles[i].color = RAYWHITE; // Initial color, will change based on velocity
    }
}


void UpdateParticles(float dt) {
    Vector2 delta, direction, forceVec;
    float distanceSquared, force, speed, invDistanceSquared;
    float gravitationalConstant = 0.67408;  // Adjusted for the simulation
    
    // Gravity calculations and velocity updates
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        Particle *particle_i = &particles[i];
        
        if (particle_i->mass <= 0) continue;  // Skip "dead" particles

        for (int j = i + 1; j < NUM_PARTICLES; ++j) {
            Particle *particle_j = &particles[j];
            
            if (particle_j->mass <= 0) continue;  // Skip "dead" particles
            
            delta = Vector2Subtract(particle_j->position, particle_i->position);
            distanceSquared = Vector2LengthSqr(delta);

            if (distanceSquared == 0) continue;

            invDistanceSquared = 1.0f / distanceSquared;
            force = gravitationalConstant * particle_i->mass * particle_j->mass * invDistanceSquared;
            
            direction = Vector2Normalize(delta);
            forceVec = Vector2Scale(direction, force);
            
            Vector2 accel_i = Vector2Scale(forceVec, 1.0f / particle_i->mass);
            Vector2 accel_j = Vector2Scale(forceVec, 1.0f / particle_j->mass);
            
            particle_i->velocity = Vector2Add(particle_i->velocity, Vector2Scale(accel_i, dt));
            particle_j->velocity = Vector2Subtract(particle_j->velocity, Vector2Scale(accel_j, dt));
        }
        
        particle_i->position = Vector2Add(particle_i->position, Vector2Scale(particle_i->velocity, dt));

        // Update color based on velocity
        speed = Vector2Length(particle_i->velocity);
        particle_i->color = (Color){(int)(255 * sin(0.016 * speed)), (int)(255 * sin(0.016 * speed - 2.0944)), (int)(255 * sin(0.016 * speed + 2.0944)), 255};
    }

    // Collision, sticking, and Elastic Collision logic
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        Particle *particle_i = &particles[i];

        if (particle_i->mass <= 0) continue;  // Skip "dead" particles

        for (int j = i + 1; j < NUM_PARTICLES; ++j) {
            Particle *particle_j = &particles[j];

            if (particle_j->mass <= 0) continue;  // Skip "dead" particles

            float distance = Vector2Distance(particle_i->position, particle_j->position);

            if (distance < (particle_i->radius + particle_j->radius)) {
                // Elastic collision logic here
                Vector2 deltaV = Vector2Subtract(particle_i->velocity, particle_j->velocity);
                Vector2 deltaP = Vector2Subtract(particle_i->position, particle_j->position);
                float dotProduct = Vector2DotProduct(deltaV, deltaP);
                
                if (dotProduct > 0) {
                    float collisionScale = dotProduct / Vector2LengthSqr(deltaP);
                    Vector2 collision = Vector2Scale(deltaP, 2 * collisionScale);
                    
                    particle_i->velocity = Vector2Subtract(particle_i->velocity, Vector2Scale(collision, particle_j->mass / particle_i->mass));
                    particle_j->velocity = Vector2Add(particle_j->velocity, Vector2Scale(collision, particle_i->mass / particle_j->mass));
                }
                
                // Stick particles i and j together
                particle_i->stuckWith = particle_j;
                particle_j->stuckWith = particle_i;
            }
            else if (particle_i->stuckWith == particle_j || particle_j->stuckWith == particle_i) {
                // Check if particles should unstick
                float kineticEnergy = 0.5f * (particle_i->mass * Vector2LengthSqr(particle_i->velocity) + particle_j->mass * Vector2LengthSqr(particle_j->velocity));
                float gravitationalPotentialEnergy = -gravitationalConstant * particle_i->mass * particle_j->mass / distance;

                if (kineticEnergy > -gravitationalPotentialEnergy) {
                    // Particles have enough energy to break apart
                    particle_i->stuckWith = NULL;
                    particle_j->stuckWith = NULL;
                }
            }
        }
    }
}

int main() {
    InitWindow(1920, 1080, "2D Gravity Simulation");
    SetTargetFPS(60);

    Camera2D camera = {
        .offset = cameraOffset,
        .target = { GetScreenWidth()/2, GetScreenHeight()/2 },
        .rotation = 0.0f,
        .zoom = zoom
    };

    InitializeParticles();

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_P)) paused = !paused;
        if (IsKeyPressed(KEY_I)) showInfo = !showInfo;

        if (IsKeyDown(KEY_RIGHT)) camera.offset.x -= 10;
        if (IsKeyDown(KEY_LEFT)) camera.offset.x += 10;
        if (IsKeyDown(KEY_UP)) camera.offset.y += 10;
        if (IsKeyDown(KEY_DOWN)) camera.offset.y -= 10;

        if (IsKeyDown(KEY_W)) camera.zoom += 0.01f;
        if (IsKeyDown(KEY_S)) camera.zoom -= 0.01f;

       


        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            Vector2 mousePosScreen = GetMousePosition();
            Vector2 mousePosWorld = GetScreenToWorld2D(mousePosScreen, camera);

            selectedParticle = NULL; // Reset selectedParticle
            
            if (followSelectedParticle) {
                followSelectedParticle = false; // Disable following if a particle is currently being followed
            } else {
                for (int i = 0; i < NUM_PARTICLES; i++) {
                    float distance = Vector2Distance(mousePosWorld, particles[i].position);
                    if (distance <= particles[i].radius) {
                        selectedParticle = &particles[i];
                        followSelectedParticle = true; // Enable following
                        break;
                    }
                }
            }
        }


        if (!paused) UpdateParticles(GetFrameTime());


        if (followSelectedParticle && selectedParticle) {
            camera.target = selectedParticle->position;
            camera.offset = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
        }







        BeginDrawing();
        ClearBackground(BLACK);
    
        BeginMode2D(camera);
        
        for (int i = 0; i < NUM_PARTICLES; i++) {
            DrawCircleV(particles[i].position, particles[i].radius, particles[i].color);
        }
        
        EndMode2D();

        if (showInfo) {
            DrawFPS(10, 12);
            DrawText(TextFormat("Latency: %f ms", GetFrameTime()*1000), 10, 30, 20, RAYWHITE);
            DrawText(paused ? "Simulation: PAUSED" : "Simulation: RUNNING", 10, 60, 20, RAYWHITE);
            DrawText("Color Mode: VELOCITY", 10, 90, 20, RAYWHITE);
        }

        if (selectedParticle) {
            DrawText(TextFormat("Speed: %.2f km/s", Vector2Length(selectedParticle->velocity)), GetScreenWidth() - 300, 12, 20, RAYWHITE);
            DrawText(TextFormat("Mass: %.2f kg", selectedParticle->mass), GetScreenWidth() - 300, 40, 20, RAYWHITE);
            DrawText(TextFormat("Size: %.2f km", selectedParticle->radius), GetScreenWidth() - 300, 70, 20, RAYWHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
