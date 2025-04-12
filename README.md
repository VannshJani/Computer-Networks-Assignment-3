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
- **Modes**:
  - Without fixes (no STP).
  - With STP enabled on switches.
  - Using POX controller with discovery and learning.

---

### Prerequisites

1. **Mininet**: Ensure Mininet is installed. You can install it using:
sudo apt-get install mininet
2. **Python**: The script is compatible with Python 3.

3. **POX Controller** (optional): If you want to test the topology with POX, clone the POX repository and run the controller:
   Clone POX repository
   git clone https://github.com/noxrepo/pox.git
   cd pox
   Run POX controller
   ./pox.py log.level --DEBUG openflow.discovery openflow.spanning_tree forwarding.l2_learning

### Usage

Follow these steps to run the code:

#### Step 1: Clone the Repository
Clone this repository to your local machine:
#### Step 2: Run the Script
The script supports three modes:
- `without-fixes`: Run tests without enabling STP (Spanning Tree Protocol).
- `with-stp`: Run tests with STP enabled on switches.
- `with-pox`: Run tests using POX controller.

Run the script using one of the following commands:  
Without fixes (no STP)  
sudo python3 q1.py without-fixes

With STP enabled  
sudo python3 q1.py with-stp  

With POX controller  
sudo python3 q1.py with-pox (If you are running the script in `with-pox` mode, ensure that the POX controller is running in another terminal before executing `q1.py`.)


## Part-2: NAT Configuration & Performance Testing

### Features
- **Network Architecture**:
  - Public network (`172.16.10.0/24`) with hosts `h3-h8`
  - Private networks (`10.1.1.0/24` and `10.1.2.0/24`) with hosts `h1-h2`
  - NAT host (`h9`) with three interfaces
- **Configurations**:
  - Automatic STP enablement on all switches
  - IP forwarding + MASQUERADE NAT rules
  - Port forwarding:
    - TCP/5001 → `h1:5001`
    - TCP/5002 → `h2:5002`
- **Tests**:
  - Bidirectional ping tests (internal ↔ external)
  - 120-second iperf3 throughput tests
  - Pre/post NAT configuration comparisons

---

### Usage

#### Step 1: Run the Script
sudo python3 q2.py

#### Step 2: Automatic Test Sequence
The script executes these phases:
1. **Topology Setup** (30 sec STP convergence)
2. **Initial Tests** (pre-NAT configuration):
   - Internal → External pings
   - External → Internal pings (expected failures)
   - Runs three iperf3 tests:
     - Internal server ↔ External client
     - External server ↔ Internal client
     - Repeat first test for consistency check
3. **NAT Configuration**:
   - Enables IP forwarding
   - Sets MASQUERADE rules
   - Configures port forwarding
4. **Post-NAT Tests**:
   - Repeats ping tests with expected success
   - Runs three iperf3 tests:
     - Internal server ↔ External client
     - External server ↔ Internal client
     - Repeat first test for consistency check
5. **CLI Access**:
   - Opens Mininet CLI for manual exploration

While generating results for the report, each file (q1.py and q2.py) were run 3 times to maintain consistency checks.
