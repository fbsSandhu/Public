# High-Performance Systems & Computational Physics Portfolio

**Brandon Sandhu** | Imperial College London (BSc Physics)
[GitHub](https://github.com/fbsSandhu) | [LinkedIn](https://www.linkedin.com/in/feteh-sandhu-748040359/)


---

# Portfolio

## Projects

| System | Focus | Core Architecture |
| :--- | :--- | :--- |
| **$O(1)$ Limit Order Book** | Low-Latency Matching Engine | 3-level hierarchical bitmap for $O(1)$ price discovery, cache-line aligned intrusive order lists, zero heap allocations on the hot path (~50ns match latency). |
| **TCP Feed Handler & SPSC Pipeline** | Network Ingestion & Thread Handoff | Linux `epoll` reactor loop feeding a lock-free single-producer single-consumer ring buffer using bitmasked power-of-2 wrapping and atomic acquire/release semantics. |
| **Thermodynamics Particle Simulator** | Scientific Computing & Statistical Mechanics | Vectorized 2D hard-sphere elastic collision engine modeling Maxwell-Boltzmann velocity distributions and ideal gas state equations across $10^6+$ steps. |
