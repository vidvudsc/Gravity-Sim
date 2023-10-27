    #include "raylib.h"
    #include "raymath.h"
    #include <stdlib.h>
    #include <stdio.h>
    #include <math.h>

    #define NUM_PARTICLES 1000
    #define G 0.07

    bool followSelectedParticle = false;

    #define NAME_SIZE 1600 // Maximum size for the name, including null-terminator

    typedef struct Particle {
    Vector2 position;
    Vector2 velocity;
    float mass;
    float radius;
    Color color;
    char name[NAME_SIZE]; // New field for name
    bool active; // New field for active state
} Particle;


    Particle particles[NUM_PARTICLES];
    Particle* selectedParticle = NULL;

    Vector2 cameraOffset = {0};
    float zoom = 0.1f;  

    bool paused = false;
    bool showInfo = false;


    void InitializeParticles() {
        for (int i = 0; i < NUM_PARTICLES; i++) {
            // Special case for the "star" particle at the center
            if (i == 0) {
                particles[i].position = (Vector2){GetScreenWidth() / 2, GetScreenHeight() / 2};
                particles[i].velocity = (Vector2){0, 0};
                particles[i].radius = 100;  // Large size in km
                particles[i].mass = particles[i].radius * 1000000;  // Large mass
                particles[i].color = YELLOW;  // Star color
                continue;
            }
            particles[i].active = true; // All particles are initially active

            
            particles[i].position = (Vector2){GetRandomValue(0, GetScreenWidth()), GetRandomValue(0, GetScreenHeight())};
            particles[i].velocity = (Vector2){0, 0};
            particles[i].radius = GetRandomValue(1, 10); // Random size in km
            particles[i].mass = particles[i].radius * 5500; // Assuming density is 5500 kg/km^3
            particles[i].color = RAYWHITE; // Initial color, will change based on velocity
            snprintf(particles[i].name, NAME_SIZE, "Particle %d", i+1); // Initialize the name
        }
    }

void mergeParticles(Particle* p1, Particle* p2) {
    // Calculate new position based on weighted averages
    p1->position.x = (p1->mass * p1->position.x + p2->mass * p2->position.x) / (p1->mass + p2->mass);
    p1->position.y = (p1->mass * p1->position.y + p2->mass * p2->position.y) / (p1->mass + p2->mass);

    // Calculate new velocity based on conservation of momentum
    p1->velocity = Vector2Add(Vector2Scale(p1->velocity, p1->mass), Vector2Scale(p2->velocity, p2->mass));
    p1->velocity = Vector2Scale(p1->velocity, 1.0f / (p1->mass + p2->mass));

    // Update mass and radius
    p1->mass += p2->mass;
    p1->radius = sqrt(p1->radius * p1->radius + p2->radius * p2->radius); // Hypothetical combined radius

    // Deactivate the second particle
    p2->active = false;
}


    void UpdateParticles() {
        Vector2 delta, direction, forceVec;
        float distanceSquared, force, speed, invDistanceSquared;
        
        for (int i = 0; i < NUM_PARTICLES; ++i) {
            Particle *particle_i = &particles[i];
            
            for (int j = i + 1; j < NUM_PARTICLES; ++j) {
                Particle *particle_j = &particles[j];
                
                // Gravity calculations
                delta = Vector2Subtract(particle_j->position, particle_i->position);
                distanceSquared = Vector2LengthSqr(delta);
                if (distanceSquared == 0) continue;
                
                invDistanceSquared = 1.0f / distanceSquared;
                force = G * particle_i->mass * particle_j->mass * invDistanceSquared;
                
                direction = Vector2Normalize(delta);
                forceVec = Vector2Scale(direction, force);
                
                Vector2 accel_i = Vector2Scale(forceVec, 1.0f / particle_i->mass);
                Vector2 accel_j = Vector2Scale(forceVec, 1.0f / particle_j->mass);
                
                particle_i->velocity = Vector2Add(particle_i->velocity, accel_i);
                particle_j->velocity = Vector2Subtract(particle_j->velocity, accel_j);

                // Elastic collision calculations
                float distance = sqrt(distanceSquared);
                if (distance < particle_i->radius + particle_j->radius) {
                    Vector2 relVelocity = Vector2Subtract(particle_i->velocity, particle_j->velocity);
                    float velAlongNormal = Vector2DotProduct(relVelocity, direction);
                    if (particle_i->active && particle_j->active) {
                        mergeParticles(particle_i, particle_j);
                    }
                    
                    if (velAlongNormal > 0) continue;

                    float e = 1.0f;  // Coefficient of restitution (elasticity)
                    float j = -(1 + e) * velAlongNormal;
                    j /= (1.0f / particle_i->mass) + (1.0f / particle_j->mass);

                    Vector2 impulse = Vector2Scale(direction, j);
                    particle_i->velocity = Vector2Add(particle_i->velocity, Vector2Scale(impulse, 1.0f / particle_i->mass));
                    particle_j->velocity = Vector2Subtract(particle_j->velocity, Vector2Scale(impulse, 1.0f / particle_j->mass));
                }
            }
            
            // Update positions
            particle_i->position = Vector2Add(particle_i->position, particle_i->velocity);
            
            // Color particles based on velocity
            speed = Vector2Length(particle_i->velocity);
            particle_i->color = (Color){(int)(255 * sin(0.016 * speed)), (int)(255 * sin(0.016 * speed - 2.0944)), (int)(255 * sin(0.016 * speed + 2.0944)), 255};
        }
    }


    float dt;  // Time step

    int main() {
        InitWindow(1920, 1080, "2D Gravity Simulation");
        SetTargetFPS(60);

        Camera2D camera = {
            .offset = cameraOffset,
            .target = { GetScreenWidth()/2, GetScreenHeight()/2 },
            .rotation = 0.0f,
            .zoom = zoom  // <-- set the initial zoom here
        };


        InitializeParticles();

        while (!WindowShouldClose()) {
            if (IsKeyPressed(KEY_P)) paused = !paused;
            if (IsKeyPressed(KEY_I)) showInfo = !showInfo;

            if (IsKeyDown(KEY_RIGHT)) camera.offset.x -= 10;
            if (IsKeyDown(KEY_LEFT)) camera.offset.x += 10;
            if (IsKeyDown(KEY_UP)) camera.offset.y += 10;
            if (IsKeyDown(KEY_DOWN)) camera.offset.y -= 10;

            if (IsKeyDown(KEY_W)) camera.zoom += 0.001f;
            if (IsKeyDown(KEY_S)) camera.zoom -= 0.001f;


        

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

            dt = 1.0f / GetFPS();
            if (!paused) UpdateParticles();

            if (followSelectedParticle && selectedParticle) {
                camera.target = selectedParticle->position;
                camera.offset = (Vector2){ GetScreenWidth() / 2, GetScreenHeight() / 2 };
            }



            BeginDrawing();
            ClearBackground(BLACK);
        
            BeginMode2D(camera);
            
            for (int i = 0; i < NUM_PARTICLES; i++) {
                if (particles[i].active) {
                DrawCircleV(particles[i].position, particles[i].radius, particles[i].color);
                }
                
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
                DrawText(TextFormat("Name: %s", selectedParticle->name), GetScreenWidth() - 300, 100, 20, RAYWHITE); // Display name
            }

            EndDrawing();
        }

        CloseWindow();
        return 0;
    }
