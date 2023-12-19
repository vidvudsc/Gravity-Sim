#include "raylib.h"
#include "raymath.h"
#include <math.h>
#include <stdio.h>

#define NUM_PARTICLES 1000
#define G_CONST 1


typedef struct {
    Vector2 pos;
    Vector2 vel;
    float mass;
    float size;
    Color color;
} Particle;

float CalculateParticleSize(float mass) {
    return sqrtf(mass); // Non-linear mapping of mass to size
}

void InitializeParticles(Particle particles[]) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        particles[i].pos = (Vector2){GetRandomValue(0, 1920), GetRandomValue(0, 1080)};
        particles[i].vel = (Vector2){GetRandomValue(-50, 50) / 10.0f, GetRandomValue(-50, 50) / 10.0f};
        particles[i].mass = GetRandomValue(1, 10);
        particles[i].size = CalculateParticleSize(particles[i].mass);
    }
}

void renderParticle(Particle *particle, Vector2 offset, float zoom) {
    float speed = Vector2Length(particle->vel);
    particle->color = ColorFromHSV(speed, 1.0f, 1.0f);
    DrawCircleV((Vector2){(particle->pos.x - offset.x) * zoom, (particle->pos.y - offset.y) * zoom}, particle->size * zoom, particle->color);
}


void updatePhysics(Particle particles[], float dt) {
    for (int i = 0; i < NUM_PARTICLES; i++) {
        Vector2 netForce = {0, 0};

        for (int j = 0; j < NUM_PARTICLES; j++) {
            if (i != j) {
                float dx = particles[j].pos.x - particles[i].pos.x;
                float dy = particles[j].pos.y - particles[i].pos.y;

                float distanceSquared = dx * dx + dy * dy;
                if (distanceSquared < 1e-3) continue; // Avoid division by zero

                float distance = sqrt(distanceSquared);
                float forceMagnitude = (G_CONST * particles[i].mass * particles[j].mass) / distanceSquared;

                netForce.x += forceMagnitude * dx / distance;
                netForce.y += forceMagnitude * dy / distance;
            }
        }

        // Update velocities and positions
        particles[i].vel.x += (netForce.x / particles[i].mass) * dt;
        particles[i].vel.y += (netForce.y / particles[i].mass) * dt;

        particles[i].pos.x += particles[i].vel.x * dt;
        particles[i].pos.y += particles[i].vel.y * dt;
    }
}

int main(void) {
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "N-Body Simulation");
    SetTargetFPS(60);

    Particle particles[NUM_PARTICLES];
    Vector2 cameraOffset = {0, 0};
    float zoomLevel = 1.0f;
    bool isPaused = false;
    double simTime = 0.0;
    double latency = 0.0;
    bool showDebugInfo = false;

    InitializeParticles(particles);

    Vector2 lastMousePos = GetMousePosition();

    while (!WindowShouldClose()) {
        float baseDt = GetFrameTime();


        // Handle simulation pause
        if (IsKeyPressed(KEY_P)) {
            isPaused = !isPaused;
        }


        // Update simulation
        if (!isPaused) {
            updatePhysics(particles, baseDt);
            simTime += baseDt;
        }

        // Handle panning and zooming
        if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
            Vector2 mouseDelta = Vector2Subtract(GetMousePosition(), lastMousePos);
            cameraOffset.x -= mouseDelta.x;
            cameraOffset.y -= mouseDelta.y;
        }
        zoomLevel *= 1.0f + GetMouseWheelMove() * 0.05f;

        lastMousePos = GetMousePosition();

        // Toggle debug info
        if (IsKeyPressed(KEY_I)) {
            showDebugInfo = !showDebugInfo;
        }

        // Render
        BeginDrawing();
        ClearBackground(BLACK);
        for (int i = 0; i < NUM_PARTICLES; i++) {
            renderParticle(&particles[i], cameraOffset, zoomLevel);
        }

        // Debug Info
        char debugText[100];
        if (showDebugInfo) {
            sprintf(debugText, "FPS: %d", GetFPS());
            DrawText(debugText, 10, 10, 20, WHITE);

            sprintf(debugText, "Latency: %.2f ms", latency);
            DrawText(debugText, 10, 40, 20, WHITE);

            sprintf(debugText, "Time Step: %.5f s", baseDt);
            DrawText(debugText, 10, 70, 20, WHITE);

            sprintf(debugText, "N =  %d", NUM_PARTICLES);
            DrawText(debugText, 10, 100, 20, WHITE);

            sprintf(debugText, "Paused: %s", isPaused ? "True" : "False");
            DrawText(debugText, 10, 130, 20, WHITE);

            sprintf(debugText, "t = %.2f s", simTime);
            DrawText(debugText, screenWidth - 100, screenHeight - 30, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
