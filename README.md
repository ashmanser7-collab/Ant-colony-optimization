# Ant Colony Optimization (ACO) — `ants.cpp`

This folder contains an interactive Ant Colony Optimization demo implemented with immediate-mode OpenGL and GLFW. The program lets you place nodes with the mouse, run ant simulations that deposit pheromone trails, and visualize pheromone strengths and found paths.

Key features
- Place nodes with left-click in the preparing stage.
- Remove nodes with right-click in the preparing stage.
- Press A in preparing stage to auto-generate nodes.
- Press ESC to finish preparing and start the simulation; press ESC again to restart preparing.
- Press P to toggle drawing the currently best (shortest) path.

Build & run
- Requires a C++ compiler and GLFW. Link against OpenGL and GLFW (opengl32 on Windows).

Example (MSYS2 / mingw g++ on Windows):

```powershell
g++ "ants.cpp" -o "ants.exe" -lglfw3 -lopengl32 -lgdi32 -std=c++17
.\\ants.exe
```

Notes
- The program uses immediate-mode OpenGL (glBegin/glEnd) and GLFW for input and windowing.
- Many simulation parameters are at the top of `ants.cpp` (node/ant counts, pheromone degradation, distance/pheromone weights). Tweak them to explore different behaviours.

Suggested improvements
- Add a CMake file and command-line options to configure parameters at runtime.
- Move rendering to modern OpenGL and decouple simulation from rendering.
