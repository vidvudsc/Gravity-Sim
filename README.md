2D Gravity Simulation in C using Raylib

Overview
This code is a simple 2D gravity simulation using Raylib and C. It initializes a number of particles with random positions, sizes, and masses. Then, it simulates their movements based on gravity and elastic collisions.

Features
Particle System: Handles NUM_PARTICLES number of particles in the simulation.
Physics: Simulates gravitational attraction between particles and updates their velocity accordingly.
Elastic Collision: Handles collision between particles and updates their velocity based on an elastic model.
Particle Info: Displays real-time info of the selected particle like speed, mass, and size.
Camera Controls: You can pan and zoom the camera view.
Pausing: The simulation can be paused and resumed.
FPS Counter: Displays current frames per second (FPS).
Dependencies
Raylib
Raymath
Standard C Libraries (stdlib.h, stdio.h, math.h)
How to Compile
To compile the code, you can use the following command:

gcc -Wall -Wextra sph.c -I C:\raylib\raylib\src -L C:\raylib\w64devkit\x86_64-w64-mingw32\lib -lraylib -lopengl32 -lgdi32 -lwinmm

Code Structure
Particle Structure
A Particle struct is defined that contains information about each particle: position, velocity, mass, radius, color, and a name.

Main Functions
InitializeParticles()
Initializes all particles. Sets a "star" particle at the center with a large mass and radius, and initializes the rest of the particles randomly.

UpdateParticles()
Updates the positions, velocities, and colors of all particles based on gravitational attraction and elastic collision logic.

main()
Initializes Raylib window and contains the main game loop. Handles drawing, camera updates, and user input.

Keyboard Controls
P: Pause/Unpause the simulation
I: Show/Hide additional info
Arrow Keys: Pan the camera
W/S: Zoom in/out
Mouse Controls
Left Click: Select a particle to follow
Additional Information
G is the gravitational constant in the simulation, adjust as needed.
dt (delta time) is calculated based on the FPS to maintain consistent simulation speed.

Enjoy :)
