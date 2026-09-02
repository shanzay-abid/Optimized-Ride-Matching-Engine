🚗 Optimized Ride-Matching & Dispatch Engine

An object-oriented C++ simulation of a modern ride-hailing dispatch pipeline. Instead of relying strictly on naive nearest-driver assignment, the engine integrates a normalized multi-factor optimization algorithm designed to balance passenger wait times, fleet workload equity, and driver fatigue.

---

### 📌 Key Highlights

* Multi-Factor Driver Matching: Dynamically scores candidate drivers using a weighted evaluation of proximity, shift workload, and fatigue levels.
* Polymorphic Pricing (Strategy Pattern): Supports seamless runtime switching between standard rates and dynamic surge multipliers.
* Live Fleet State Tracking: Monitors driver availability, updates map coordinates upon drop-off, and applies a realistic 45-second cooldown cycle.
* Fleet Analytics & Fairness Auditing: Computes real-time trip statistics, driver revenue distribution, and an algorithmic fairness index.
* File-Based Trip Logging: Appends trip manifests directly to persistent text logs with cross-platform timestamping.

---

### 🧠 Driver Selection Formula

Every eligible driver receives an evaluation score (0.0 to 1.0) calculated via min-max normalization:

Final Score = (Distance_norm × 0.60) + (Workload_norm × 0.20) + (Fatigue_norm × 0.20)

* 60% Proximity: Ensures responsive pickup times for passengers.
* 20% Workload Balance: Prevents assignment monopolies; provides a +0.05 score bonus to drivers with under 3 completed rides.
* 20% Fatigue Protection: Mitigates burnout by factoring consecutive trip fatigue into allocation.

---

### 🏗️ Architecture Overview

* User (Abstract Base): Defines common identity and 2D Euclidean coordinate behavior (Location).
  * Rider: Encapsulates passenger profiles and dynamic destination requests[cite: 1, 2].
  * Driver: Manages operational availability, driver rating, trip counts, and cooldown timers[cite: 1, 2].
* FareStrategy (Interface): Defines the pricing contract implemented by StandardPricing and SurgePricing[cite: 1, 2].
* RideMatchingEngine: Central orchestrator managing dynamic driver pools, ride execution, and analytical benchmarks[cite: 1, 2].
---

### 📄 Project Documentation

For an in-depth breakdown of the object-oriented design and mathematical modeling behind this engine, refer to the full report[cite: 1]:

* [View Project Documentation (PDF)](Project_Report.pdf) — Covers the class inheritance diagrams, state transitions, runtime normalization proofs, and system fairness benchmarks[cite: 1].
---

### 🚀 Getting Started

#### Terminal (g++ / Clang)

```bash
# Clone the repository
git clone [https://github.com/](https://github.com/)<your-username>/ride-matching-engine.git
cd ride-matching-engine

# Compile the source code
g++ -std=c++11 src/main.cpp -o ride_engine

# Run the simulator
./ride_engine

