# High-Performance Systems & Computational Physics Portfolio

**Brandon Sandhu** | Imperial College London (BSc Physics)
[GitHub](https://github.com/fbsSandhu) | [LinkedIn](https://www.linkedin.com/in/feteh-sandhu-748040359/)


---

## Project Overview

| Project | Tech Stack | Highlights | Link |
| :--- | :--- | :--- | :--- |
| **Low-Latency $O(1)$ Limit Order Book** | C++20, Google Benchmark, Google Test | Zero-allocation matching engine on the hot path utilising intrusive doubly-linked lists and memory pooling. | [`/limit-order-book`](./limit-order-book) |
| **C++ Systems & Memory Primitives** | C++20, Linux Systems | Custom SSO string layout (23-byte stack buffer), RAII smart pointers, and a thread-safe SPSC queue. | [`/cpp-primitives`](./Implement) |
| **Thermodynamics Particle Simulator** | Python, NumPy, Matplotlib | Vectorized 2D hard-sphere elastic collision engine modeling Maxwell-Boltzmann distributions across $10^6+$ time steps. | [`/thermo-simulator`](./Collision_Sim) |

---

## Repository Structure

```text
Public/
└── Collision_Sim     # Vectorized 2D Statistical Mechanics Engine
├── Implement/       # SSO String, Smart Pointers, SPSC Queue
├── limit-order-book/     # C++20 Order Matching Engine & Benchmarks
