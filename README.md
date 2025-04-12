# Computer Networks Assignment-3

## Part-1: Loop Topology with Ping Analysis

This repository contains the implementation of a network topology with loops and tools to analyze ping results under different configurations. The topology is created using Mininet, and tests are conducted to evaluate the behavior of the network under various conditions.

---

### Features

- **Topology Description**:
  - 4 switches (`s1-s4`) in a ring topology with an additional link between `s1` and `s3`, creating a loop.
  - 8 hosts (`h1-h8`) connected to switches.
  - Switch-to-switch links have **7ms latency**.
  - Host-to-switch links have **5ms latency**.
- **Ping Tests**:
  - Run ping tests between selected hosts.
  - Analyze success rates and average delays.
  - Capture packets for further analysis.
- **Modes**:
  - Without fixes (no STP).
  - With STP enabled on switches.
  - Using POX controller with discovery and learning.

---

### Prerequisites

1. **Mininet**: Ensure Mininet is installed. You can install it using:
