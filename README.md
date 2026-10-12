# Ray-Tracer

A Whitted-style recursive ray tracer in C++ that renders 3D scenes with
reflections, refraction, shadows, and textures. It includes a uniform-grid
acceleration structure and a multithreaded render loop that reaches a **9×
speedup** over the brute-force path, plus a companion software rasterizer.

> Built for a computer graphics course (CS130)

<img width="1512" height="852" alt="ray-tracer" src="https://github.com/user-attachments/assets/1b3abe2b-4a8b-4774-88ae-2ae25f1a9b2e" />

---

## What I implemented

- **Ray–surface intersection** for spheres and triangle meshes.
- **Phong shading** with shadow rays, so objects correctly block light from each
  other.
- **Recursive reflection** for mirror-like surfaces, up to a depth limit.
- **Transparent/glass materials** — refraction via Snell's law, total internal
  reflection, and a Schlick approximation for the Fresnel term.
- **Texture mapping** and a camera basis for positioning the view.
- **Uniform-grid acceleration** traversed with 3D-DDA voxel stepping, binning
  per-cell bounding boxes with early ray termination.
- **Multithreaded rendering** — a dynamic row queue driven by a `std::atomic`
  work counter hands each scanline to the next free thread, for a **9× speedup**.
- **Anti-aliasing** via jittered supersampling, using `thread_local std::mt19937`
  so sampling is race-free across threads.
- **A separate software rasterizer** with perspective-correct interpolation and a
  z-buffer, as a point of comparison with the ray-traced path.

## Tech stack

| Area | Tooling |
| --- | --- |
| Language | C++ |
| Concurrency | `std::thread` + `std::atomic` (dynamic row queue) |
| Acceleration | Uniform spatial grid, 3D-DDA traversal |
| Build | [CMake / Make] |
| Output | PNG |
| Dependencies | None beyond the standard library |

## How it works

For every pixel, the renderer casts a ray from the camera into the scene and
follows what it hits:

1. **Closest intersection** — find the nearest object the ray hits, using the
   uniform grid to skip empty space instead of testing every object.
2. **Shading** — compute the surface color with Phong, casting a shadow ray
   toward each light to check if it's blocked.
3. **Recursion** — for reflective or transparent surfaces, spawn reflected and
   refracted rays and repeat, up to the recursion-depth limit.
4. **Accumulate** — combine the samples (jittered supersampling) into the final
   pixel color.

### Parallelism

Rendering is embarrassingly parallel — each pixel is independent. The image is
split into rows, and a shared work queue hands the next unrendered row to whichever
thread is free, driven by a `std::atomic` counter. Dynamic scheduling keeps every
core busy to the end, instead of a static split that leaves some threads idle on
cheaper regions. The result is a **9× speedup** over brute-force rendering.

```
 ┌───────────────── work queue (rows 0..N) ─────────────────┐
 │  row 0   row 1   row 2   row 3   row 4   row 5   ...      │
 └───┬────────┬────────┬────────┬───────────────────────────┘
     ▼        ▼        ▼        ▼
  thread 1 thread 2 thread 3 thread 4   ← each grabs the next free row
```

## Build

```bash
git clone https://github.com/Jacob-GHub/Ray-Tracer.git
cd Ray-Tracer
cmake -B build
cmake --build build
```

## Usage

```bash
# Render a scene file to a PNG
./build/raytracer [scene_file]   
```


## Project structure

```
Ray-Tracer/
├── render_world.cpp      # render loop, closest intersection, cast ray
├── camera.cpp            # pixel-to-world ray generation
├── sphere.cpp            # ray-sphere intersection
├── mesh.cpp              # ray-triangle intersection
├── plane.cpp             # ray-plane intersection
├── phong_shader.cpp      # Phong shading + shadow rays
├── reflective_shader.cpp # recursive reflection
├── transparent_shader.cpp# Snell / TIR / Schlick Fresnel
├── flat_shader.cpp
├── texture.cpp           # texture mapping
├── acceleration.cpp      # uniform grid + 3D-DDA traversal
├── box.cpp               # AABB / slab test
├── rasterize.cpp         # companion software rasterizer
└── {hit,object,ray,misc}.h
```

## What I learned / possible next steps

- Getting a near-linear speedup meant moving from a static row split to a dynamic
  work queue so threads never sat idle on cheap regions — and fixing a data race
  in anti-aliasing by giving each thread its own RNG (`thread_local std::mt19937`).
- Possible extensions: swap the uniform grid for a BVH on scenes with uneven
  object density, or port the render loop to CUDA for a second speedup.

## License

[MIT]
