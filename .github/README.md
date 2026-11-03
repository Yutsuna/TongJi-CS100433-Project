<h1 align="center">
  CS100433<br>
  Computer Graphics
  <br>
  <img src="https://raw.githubusercontent.com/catppuccin/catppuccin/main/assets/palette/macchiato.png" width="600px"/>
  <br>
</h1>

<div align="center">
  <p></p>
  <div align="center">
  </div>
  <br>
</div>

<h2 align="center">
    Lumina - Project 1
</h2>

## Motivation

Lumina is a real-time Path Tracer with progressive refinement. The project motivation is to learn
how modern computer graphics render light. We want to explore the actor model paradigm, where
isolated actors communicate exclusively through messages.

## The Goal of the project


The goal of the project is to build a real-time and fluid interactive GPU path tracer in OpenGL,
where:
- All ray tracing executes in compute shaders on the GPU
- The surrounding pipeline is built on an actor system with work stealing schedulers and
lock-free mailboxes
- The user can interact live via ImGui panel, tweak lights, camera and renderer parameters
update without restarting via messages, and the progressive accumulation buffer adapts in
real time.
- Measured evidence of parallelism such as samples/sec and frame time scaling as a function
of worker-thread count, plus per-tile timing heatmaps rendered live.

## The scope of the project

The Scope of the project
- Compute-shader path tracing of small scenes.
- Scene is declarative and parsed.
- PBR materials: diffuse, metallic/roughness, dielectric refraction with Fresnel, emissive.
- Camera with user interactions and tweaks (FOV, depth of field, exposure).
- Actor model pipeline with tile queue workers, triple-buffered snapshot handoff to the
render thread.
- ImGui parameter panel with debug views (BVH depth, normals, material-only, per-tile
timing heatmap)

## Related CG techniques

- Path Tracing & the rendering equation: Monte Carlo integration of the light-transport
equation formulated by Kajiya [1] with Russian roulette path termination; we follow the
incremental approach of Shirley’s tutorial series for the baseline integrator [2].
- Progressive refinement: When the camera is static, samples accumulate into running
average; on movement, history is reprojected and reset. This temporal amortization follows
the progressive estimation framework of Hachisuka [3].
- Concurrency model: Actor model as defined bby Hewitt, Bishop and Steiger [4]: isolated
actors communicating exclusively via asynchronous messages, scheduled on a work-stealing
thread pool.
- Language & Norm: C++26, Epic Game’s Unreal Engine coding norm [5].
- Dependencies: managed with nix to ensure reproductible environment and build [6].
- Authorized dependencies that nix will fetch: gcc16, gnumake, GLFW3, glm, GLSL, imGUI.

## References
1. J. T. Kajiya, "The Rendering Equation," SIGGRAPH '86, 1986, pp. 143–150.
https://dl.acm.org/doi/10.1145/15922.15902
2. P. Shirley, Ray Tracing in One Weekend (free online book). https://raytracing.github.io
3. T. Hachisuka, S. Ogaki, H. W. Jensen, "Progressive Photon Mapping," ACM Trans. Graph. 27(5), 2008.
https://dl.acm.org/doi/10.1145/1409060.1409083
4. C. Hewitt, P. Bishop, R. Steiger, "A Universal Modular ACTOR Formalism for Artificial Intelligence,"
IJCAI '73, 1973, pp. 235–245. https://dl.acm.org/doi/10.5555/1624775.1624804
5. Epic Game’s Unreal Engine coding standard. https://dev.epicgames.com/documentation/unrealengine/epic-cplusplus-coding-standard-for-unreal-engine
6. Nix. https://nixos.org/

## How to build

This repository uses C++26 reflections. First, Install dependencies and build automatically using [**nix**](https://nixos.org/).
```bash
nix build
```

Or manually install dependencies (or use `nix develop`) and build with [**gnumake**](https://www.gnu.org/software/make/).
```bash
make
```
