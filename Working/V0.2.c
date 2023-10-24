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
} Particle;

Particle particles[NUM_PARTICLES];
Particle* selectedParticle = NULL;

Vector2 cameraOffset = {0};
float zoom = 1.0f;
bool paused = false;
bool showInfo = false;

void InitializeParticles() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].position = (Vector2){GetRandomValue(0, GetScreenWidth()), GetRandomValue(0, GetScreenHeight())};
        particles[i].velocity = (Vector2){0, 0};
        particles[i].radius = GetRandomValue(1, 10); // Random size in km
        particles[i].mass = particles[i].radius * 5500; // Assuming density is 1000 kg/km^3
        particles[i].color = RAYWHITE; // Initial color, will change based on velocity
    }
}

void UpdateParticles() {
    Vector2 delta, direction, forceVec;
    float distanceSquared, force, speed, invDistanceSquared;
    
    for (int i = 0; i < NUM_PARTICLES; ++i) {
        Particle *particle_i = &particles[i];
        
        for (int j = i + 1; j < NUM_PARTICLES; ++j) {
            Particle *particle_j = &particles[j];
            
            delta = Vector2Subtract(particle_j->position, particle_i->position);
            distanceSquared = Vector2LengthSqr(delta); // Compute squared distance directly
            if (distanceSquared == 0) continue;
            
            invDistanceSquared = 1.0f / distanceSquared; // Pre-calculate the inverse of the distanceSquared
            force = particle_i->mass * particle_j->mass * invDistanceSquared;
            
            direction = Vector2Normalize(delta);
            forceVec = Vector2Scale(direction, force);
            
            Vector2 accel_i = Vector2Scale(forceVec, 1.0f / particle_i->mass);
            Vector2 accel_j = Vector2Scale(forceVec, 1.0f / particle_j->mass);
            
            particle_i->velocity = Vector2Add(particle_i->velocity, accel_i);
            particle_j->velocity = Vector2Subtract(particle_j->velocity, accel_j);
        }
        
        particle_i->position = Vector2Add(particle_i->position, particle_i->velocity);
        speed = Vector2Length(particle_i->velocity);
        particle_i->color = (Color){speed * 10, 0, 255 - speed * 10, 255};
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


        if (!paused) UpdateParticles();

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
