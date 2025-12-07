# 2D Multiplayer Racing Game

**Tech:** C++17, OpenGL, GLFW, ENet, Custom Engine (Static Library)

## Overview
A server-authoritative 2D multiplayer racing game built from the ground up in C++.  
The project demonstrates real-time networking, deterministic physics, collision handling, and custom rendering — all powered by a lightweight, modular engine developed as a static library.

## Features
- **Authoritative Server Simulation:** 64 Hz deterministic car physics with collision detection  
- **Client-Side Prediction & Server Reconciliation:** Minimizes perceived input latency, ensuring responsive controls  
- **Snapshot Interpolation & Smoothing:** Maintains smooth remote player movement under jitter and packet loss  
- **Networking:** Reliable and unreliable packet channels, dedicated asynchronous client networking thread  
- **Collision System:** Tile-based spatial partitioning reducing collision checks from O(N²) → O(N·K) (near-linear), broad-phase AABB and narrow-phase OBB detection  
- **Engine Architecture:** Modular engine with physics, rendering, networking, collision, and math utilities  

## Demo
https://github.com/user-attachments/assets/076540d8-0ab1-46ed-850e-e02de416440a

## Challenges & Solutions
- **Jittery remote players:** Solved using snapshot buffering, interpolation windows, and position/rotation smoothing  
- **Desync during high latency:** Implemented tick-based input replay for deterministic correction  
- **Collision checks causing lag:** Used tile-based spatial partitioning to query only surrounding tiles and pool nearby vertices

## Build & Run


