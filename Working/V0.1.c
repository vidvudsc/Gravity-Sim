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
        particles[i].radius = GetRandomValue(5, 50); // Random size in km
        particles[i].mass = particles[i].radius * 1000; // Assuming density is 1000 kg/km^3
        particles[i].color = RAYWHITE; // Initial color, will change based on velocity
    }
}

void UpdateParticles() {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        for (int j = 0; j < NUM_PARTICLES; j++) {
            if (i != j) {
                Vector2 delta = Vector2Subtract(particles[j].position, particles[i].position);
                float distance = Vector2Length(delta);
                if (distance == 0) continue;
                
                float force = (particles[i].mass * particles[j].mass) / (distance * distance);
                Vector2 direction = Vector2Normalize(delta);
                Vector2 forceVec = Vector2Scale(direction, force);
                
                particles[i].velocity = Vector2Add(particles[i].velocity, Vector2Scale(forceVec, 1 / particles[i].mass));
            }
        }
        
        particles[i].position = Vector2Add(particles[i].position, particles[i].velocity);
        
        float speed = Vector2Length(particles[i].velocity);
        particles[i].color = (Color){speed * 10, 0, 255 - speed * 10, 255};
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
            DrawText(TextFormat("Speed: %.2f km/s", Vector2Length(selectedParticle->velocity)), GetScreenWidth() - 300, 10, 20, RAYWHITE);
            DrawText(TextFormat("Mass: %.2f kg", selectedParticle->mass), GetScreenWidth() - 300, 40, 20, RAYWHITE);
            DrawText(TextFormat("Size: %.2f km", selectedParticle->radius), GetScreenWidth() - 300, 70, 20, RAYWHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
