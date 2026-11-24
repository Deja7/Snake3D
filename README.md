# Snake 3D

A 3D Snake game written in **C++** with **SDL2**, powered by a **custom CPU-based 3D software renderer** built from scratch — without OpenGL, DirectX, or any external 3D engine.

## Screenshots

<!-- Add your gameplay screenshots here (e.g. inside a screenshots/ folder):
| 3D Perspective Mode | 2D Top-Down Mode |
| :---: | :---: |
| ![3D Gameplay](screenshots/gameplay-3d.png) | ![2D Gameplay](screenshots/gameplay-2d.png) |
-->

## Highlights

### Custom 3D Software Renderer
- **3D Math & Transformations** ([`utility.h`](utility.h)): Custom `v2d` / `v3d` vector classes with XZ, XY, and YZ axis rotations and dot-product lighting calculations.
- **Software Rasterization** ([`main.cpp`](main.cpp)): Custom scanline triangle rasterizer rendering directly to an SDL pixel surface.
- **Depth Sorting & Clipping**: Painter's algorithm (`qsort` by triangle centroid distance to the camera) with near-plane clipping (`z <= 0.1`).
- **Procedural 3D Models & Flat Shading**: Procedurally generated beveled 3D voxel geometry (24 vertices, 30 triangles per segment) lit with per-face surface normals and point-light shading.
- **Smooth Camera System**: Interpolated 3rd-person follow camera with yaw smoothing, adjustable zoom, and instant switching between 3D perspective and 2D top-down orthographic views.

### Gameplay & Features
- **3D / 2D Camera Toggle**: Switch seamlessly between 3rd-person 3D perspective and classic top-down 2D gameplay at any time (`M`).
- **Animated Snake**: Slithering body animation and smooth linear interpolation (`lerp`) when growing new tail segments.
- **Teleportation Portals**: Randomly spawned numbered portal pairs that teleport each snake segment independently with cooldown protection.
- **Timed Bonus Red Dot**: Spawns periodically with an on-screen countdown bar; awards bonus points and randomly either shortens the snake or reduces movement speed.
- **Progressive Difficulty**: Snake movement speed automatically ramps up every 30 seconds.
- **Autoplayer AI Mode**: Autonomous simulation mode that evaluates horizontal-first and vertical-first collision-free Manhattan paths to target food and bonus red dots ([`autoplayer.h`](autoplayer.h)).
- **Save & Load System**: Full game state serialization (snake body, portals, score, timers, speed, board size) to `save.txt`.
- **High Score Leaderboard**: Persistent Top-3 leaderboard with in-game text input saved to `highscore.txt`.
- **External Configuration**: Board dimensions, speed, scoring, and bonus settings configurable via `settings.txt`.

## Controls

### Main Menu
| Key | Action |
| --- | --- |
| `N` | Start a new game |
| `A` | Launch Autoplayer AI mode |
| `Esc` | Quit |

### In-Game (3D Perspective Mode)
| Key | Action |
| --- | --- |
| `Left` / `Right` | Turn snake left / right (relative to heading) |
| `Up` / `Down` | Zoom camera in / out |
| `M` | Toggle 3D / 2D (top-down) view |
| `S` | Save current game to `save.txt` |
| `L` | Load saved game from `save.txt` |
| `N` | Restart game |
| `Esc` | Quit |

### In-Game (2D Top-Down Mode)
| Key | Action |
| --- | --- |
| `Arrow Keys` | Steer snake in absolute directions (`Up` / `Down` / `Left` / `Right`) |
| `M` | Switch back to 3D mode |
| `S` / `L` / `N` / `Esc` | Save / Load / Restart / Quit |

### Autoplayer Mode
| Key | Action |
| --- | --- |
| `Up` / `Down` | Zoom camera in / out |
| `M` | Toggle 3D / 2D view |
| `S` / `L` | Save / Load game state |
| `Esc` | Exit |

## Configuration

Game parameters can be customized in [`settings.txt`](settings.txt):

| Parameter | Default | Description |
| --- | --- | --- |
| `speed` | `1.5` | Initial snake movement speed (tiles/sec) |
| `length` | `3` | Initial snake length (segments) |
| `frequency` | `10` | Minimum interval (seconds) between bonus red dot spawns |
| `foodPts` | `1` | Points awarded for eating regular food |
| `dotPts` | `3` | Points awarded for collecting the bonus red dot |
| `boardX` | `16` | Board width (tiles) |
| `boardY` | `16` | Board height (tiles) |
| `speedUp` | `3.0` | Speedup scaling factor applied every 30 seconds |
| `speedDown` | `1.5` | Speed divisor when a bonus red dot triggers slowdown |

## Building & Running (Windows)

### Prerequisites
- **Visual Studio 2019** or **Visual Studio 2022** with the **Desktop development with C++** workload.
- **SDL2 2.0.10** is bundled directly in the repository (`SDL2-2.0.10/`), so no manual library setup is required.

### Steps
1. Clone the repository:
   ```bash
   git clone https://github.com/<your-username>/<your-repo>.git
   cd <your-repo>
   ```
2. Open `Snake3D.sln` in Visual Studio.
3. Select your target configuration (`Release` or `Debug`) and platform (`x64` or `Win32`).
4. Build and run (`F5`). A post-build step automatically copies the matching `SDL2.dll` into the output directory.