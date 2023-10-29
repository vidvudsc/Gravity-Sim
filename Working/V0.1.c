#include "raylib.h"
#include <math.h>
#include <stdbool.h>  // For the boolean variables
#include <stdio.h>


#define NUM_PARTICLES 10000
double simTime = 0.0;  // Accumulated simulation time

typedef struct {
    Vector2 pos;
    float strength;
} GravitySource;

typedef struct {
    Vector2 pos;
    Vector2 vel;
    float mass;
    float density;
    float size;
    Color color;
} Particle;

void renderGravitySource(GravitySource *source, Vector2 offset, float zoom) {
    DrawCircleV((Vector2){(source->pos.x - offset.x) * zoom, (source->pos.y - offset.y) * zoom}, 10 * zoom, WHITE);
}

void renderParticle(Particle *particle, Vector2 offset, float zoom) {
    float speed = sqrt(particle->vel.x * particle->vel.x + particle->vel.y * particle->vel.y);
    particle->color = ColorFromHSV(speed, 1.0f, 1.0f);
    
    DrawCircleV((Vector2){(particle->pos.x - offset.x) * zoom, (particle->pos.y - offset.y) * zoom}, particle->size * zoom, particle->color);
}

void updatePhysics(Particle *particle, GravitySource *source, float dt) {
    float distance_x = source->pos.x - particle->pos.x;
    float distance_y = source->pos.y - particle->pos.y;
    
    float distance = sqrt(distance_x * distance_x + distance_y * distance_y);
    
    float inverse_distance = 1.0f / distance;
    
    float normalized_x = inverse_distance * distance_x;
    float normalized_y = inverse_distance * distance_y;
    
    float inverse_square_dropoff = inverse_distance * inverse_distance;
    
    float gravitational_force = (source->strength * particle->mass * inverse_square_dropoff);
    
    float new_acceleration_x = normalized_x * gravitational_force;
    float new_acceleration_y = normalized_y * gravitational_force;

    // Calculate jerk (assuming the previous acceleration was zero)
    float jerk_x = (new_acceleration_x) / dt;
    float jerk_y = (new_acceleration_y) / dt;

    // Time step based on acceleration and jerk
    float dt_new_x = sqrt(new_acceleration_x / jerk_x);
    float dt_new_y = sqrt(new_acceleration_y / jerk_y);
    
    // Use the smaller time step for more accurate simulation
    float dt_new = fmin(dt_new_x, dt_new_y);
    
    // Update velocity and position based on new time step
    particle->vel.x += new_acceleration_x * dt_new;
    particle->vel.y += new_acceleration_y * dt_new;
    
    particle->pos.x += particle->vel.x * dt_new;
    particle->pos.y += particle->vel.y * dt_new;
}




float dt;  // Time step

int main(void) {
    const int screenWidth = 1920;
    const int screenHeight = 1080;

    InitWindow(screenWidth, screenHeight, "Raylib Gravity Example");

    ToggleFullscreen();

    SetTargetFPS(60);

    GravitySource source = {{800, 500}, 7000};
    Vector2 cameraOffset = {0, 0};
    float zoomLevel = 1.0f;

    Particle particles[NUM_PARTICLES];

    bool isPaused = false;
    bool showDebugInfo = false;
    double latency = 0.0;

    for (int i = 0; i < NUM_PARTICLES; i++) {
        float angle = (float)i / NUM_PARTICLES * 2 * PI;
        float radius = GetRandomValue(100, 400);

        particles[i].pos.x = source.pos.x + cos(angle) * radius;
        particles[i].pos.y = source.pos.y + sin(angle) * radius;

        particles[i].vel.x = -sin(angle) * 7;
        particles[i].vel.y = cos(angle) * 7;

        particles[i].density = GetRandomValue(1, 10);
        particles[i].size = GetRandomValue(1, 2);
        particles[i].mass = particles[i].density * particles[i].size;
    }

    while (!WindowShouldClose()) {
      dt = GetFrameTime();  // Get the time elapsed since the last frame
        // Pan and zoom controls
        if (IsKeyDown(KEY_RIGHT)) cameraOffset.x += 10;
        if (IsKeyDown(KEY_LEFT)) cameraOffset.x -= 10;
        if (IsKeyDown(KEY_DOWN)) cameraOffset.y += 10;
        if (IsKeyDown(KEY_UP)) cameraOffset.y -= 10;
        if (IsKeyDown(KEY_W)) {
            zoomLevel *= 1.01f;
            cameraOffset.x *= 1.01f;
            cameraOffset.y *= 1.01f;
        }
        if (IsKeyDown(KEY_S)) {
            zoomLevel /= 1.01f;
            cameraOffset.x /= 1.01f;
            cameraOffset.y /= 1.01f;
        }

        if (IsKeyPressed(KEY_P)) {
            isPaused = !isPaused;
        }

        if (IsKeyPressed(KEY_I)) {
            showDebugInfo = !showDebugInfo;
        }

        double startTime = GetTime();

        BeginDrawing();

        ClearBackground(BLACK);


        if (!isPaused) {
            simTime += dt;  
            for (int i = 0; i < NUM_PARTICLES; i++) {
                updatePhysics(&particles[i], &source, dt);
            }
        }

        // Moved this out of the if (!isPaused) block
        for (int i = 0; i < NUM_PARTICLES; i++) {
            renderParticle(&particles[i], cameraOffset, zoomLevel);
        }

        renderGravitySource(&source, cameraOffset, zoomLevel);

        double endTime = GetTime();
        latency = (endTime - startTime) * 1000;

        char debugText[100];
        if (showDebugInfo) {
            sprintf(debugText, "FPS: %d", GetFPS());
            DrawText(debugText, 10, 10, 20, WHITE);

            sprintf(debugText, "Latency: %.2f ms", latency);
            DrawText(debugText, 10, 40, 20, WHITE);

            sprintf(debugText, "Time Step: %.5f s", dt);
            DrawText(debugText, 10, 70, 20, WHITE);

            sprintf(debugText, "N =  %d ",NUM_PARTICLES);
            DrawText(debugText, 10, 100, 20, WHITE);

            sprintf(debugText, "Paused: %s", isPaused ? "Yes" : "No");
            DrawText(debugText, 10, 130, 20, WHITE);

            sprintf(debugText, "t = %.2f s", simTime);
            DrawText(debugText, screenWidth - 100, screenHeight - 30, 20, WHITE);
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
