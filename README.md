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
`sudo apt-get install mininet`
2. **Python**: The script is compatible with Python 3.

3. **POX Controller** (optional): If you want to test the topology with POX, clone the POX repository and run the controller:
   Clone POX repository
   `git clone https://github.com/noxrepo/pox.git`
   `cd pox`
   Run POX controller
   `./pox.py log.level --DEBUG openflow.discovery openflow.spanning_tree forwarding.l2_learning`

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
`sudo python3 q1.py without-fixes`

With STP enabled  
`sudo python3 q1.py with-stp`  

With POX controller  
`sudo python3 q1.py with-pox` (If you are running the script in `with-pox` mode, ensure that the POX controller is running in another terminal before executing `q1.py`.)


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
`sudo python3 q2.py`

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


## Q3: Network routing:
### Distance Vector Routing Protocol Implementation

This project implements a distributed, asynchronous distance vector routing protocol for a network with 4 nodes. The implementation follows the Bellman-Ford algorithm (as discussed in the class) where each node maintains its own routing table and exchanges information only with its directly connected neighbors.

## Network Topology
- Node 0 is connected to nodes 1, 2, and 3 with costs 1, 3, and 7 respectively
- Node 1 is connected to nodes 0 and 2 with costs 1 and 1 respectively
- Node 2 is connected to nodes 0, 1, and 3 with costs 3, 1, and 2 respectively
- Node 3 is connected to nodes 0 and 2 with costs 7 and 2 respectively

## Files
- `node0.c`, `node1.c`, `node2.c`, `node3.c`: Implementation of routing protocol for each node
- `distance_vector.c`: Network simulation environment (provided)

## How to Compile
Since the code is written in an older C style, some of the modern compilers may throw some warnings regarding the older version of C. So, to compile without any warnings, use:
```bash
cc -Wno-implicit-int -Wno-implicit-function-declaration -Wno-return-type distance_vector.c node0.c node1.c node2.c node3.c -o dv
```

## How to Run
Execute the compiled program:
```bash
./dv
```

When prompted, enter a TRACE value:
- 0: No debug information
- 1: Basic debug information
- 2: Detailed debug information (recommended for observing the algorithm)

The simulation will execute the distance vector routing algorithm and terminate when there are no more routing packets in transit (i.e., when the algorithm has converged).

## Algorithm
The implementation uses the distance vector routing algorithm, which works as follows:
1. Each node initializes its distance table with direct costs to neighbors
2. Each node sends its distance vector to all directly connected neighbors
3. When a node receives a routing packet from a neighbor, it updates its distance table
4. If updates were made, it recalculates minimum costs and sends updates to neighbors
5. The algorithm continues until convergence (no more updates to any distance table)
