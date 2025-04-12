from mininet.node import DefaultController, RemoteController
from mininet.topo import Topo
from mininet.net import Mininet
from mininet.cli import CLI
from mininet.log import setLogLevel, info
import time
import os
import sys
import re

class LoopTopo(Topo):
    """
    Defines a custom network topology:
    - 4 switches connected in a loop, with an extra link between s1 and s3.
    - 8 hosts, 2 connected to each switch.
    - 7ms delay between switches, 5ms delay from host to switch.
    """
    def build(self):
        # Create 4 switches: s1 to s4
        s1, s2, s3, s4 = [self.addSwitch(f's{i}') for i in range(1, 5)]
        
        # Create 8 hosts with IPs from 10.0.0.2 to 10.0.0.9
        hosts = []
        for i in range(1, 9):
            hosts.append(self.addHost(f'h{i}', ip=f'10.0.0.{i+1}/24'))
        
        # Connect switches to form a ring, with an extra s1-s3 link
        self.addLink(s1, s2, delay='7ms')
        self.addLink(s2, s3, delay='7ms')
        self.addLink(s3, s4, delay='7ms')
        self.addLink(s4, s1, delay='7ms')
        self.addLink(s1, s3, delay='7ms')  # creates an additional loop
        
        # Connect hosts to switches, 2 hosts per switch
        self.addLink(hosts[0], s1, delay='5ms')  # h1 to s1
        self.addLink(hosts[1], s1, delay='5ms')  # h2 to s1
        self.addLink(hosts[2], s2, delay='5ms')  # h3 to s2
        self.addLink(hosts[3], s2, delay='5ms')  # h4 to s2
        self.addLink(hosts[4], s3, delay='5ms')  # h5 to s3
        self.addLink(hosts[5], s3, delay='5ms')  # h6 to s3
        self.addLink(hosts[6], s4, delay='5ms')  # h7 to s4
        self.addLink(hosts[7], s4, delay='5ms')  # h8 to s4

def ensure_dir(directory):
    """Create the directory if it doesn't exist"""
    if not os.path.exists(directory):
        os.makedirs(directory)

def analyze_ping_result(output):
    """
    Parse the ping command output:
    - Returns whether ping was successful
    - Returns average delay if available
    - Otherwise, gives reason for failure
    """
    received_pattern = r'(\d+) received'
    received_match = re.search(received_pattern, output)
    
    if received_match and int(received_match.group(1)) > 0:
        # Ping got replies, try to extract average RTT
        rtt_pattern = r'min/avg/max/mdev = [\d.]+/([\d.]+)/[\d.]+/[\d.]+ ms'
        rtt_match = re.search(rtt_pattern, output)
        
        if rtt_match:
            avg_delay = float(rtt_match.group(1))
            return True, avg_delay, None
        else:
            return True, None, "Couldn't extract average delay"
    else:
        # Ping failed, try to figure out why
        if "Destination Host Unreachable" in output:
            reason = "Destination Host Unreachable - ARP resolution failure"
        elif "100% packet loss" in output:
            reason = "100% packet loss - Packets never reached destination"
        else:
            reason = "Unknown failure - Check packet captures"
        
        return False, None, reason

def run_ping_test(source, target, test_name, capture_dir, stp_status):
    """
    Runs a ping test between two hosts:
    - Repeats test 3 times with 30s gap
    - Captures packets using tcpdump
    - Summarizes success rate and delay
    """
    target_ip = target.IP()
    info(f"\n=== {test_name}: {source.name} → {target.name} ({target_ip}) ===\n")
    
    results = []
    
    for i in range(3):
        info(f"\nAttempt {i+1}:\n")
        
        # File names for packet captures
        source_capture = f'{capture_dir}/{source.name}_to_{target.name}_attempt{i+1}_{stp_status}.pcap'
        target_capture = f'{capture_dir}/{target.name}_from_{source.name}_attempt{i+1}_{stp_status}.pcap'
        
        # Start capturing traffic on both ends
        source.cmd(f'tcpdump -i {source.name}-eth0 -w {source_capture} "not icmp6" &')
        target.cmd(f'tcpdump -i {target.name}-eth0 -w {target_capture} "not icmp6" &')
        
        time.sleep(2)  # Give tcpdump time to start
        
        # Run the ping
        output = source.cmd(f'ping -c 4 {target_ip}')
        info(output)
        
        # Stop tcpdump
        source.cmd('pkill -f tcpdump')
        target.cmd('pkill -f tcpdump')
        
        # Analyze ping result
        success, delay, reason = analyze_ping_result(output)
        results.append((success, delay, reason))
        
        if i < 2:
            info("Waiting 30 seconds before next attempt...\n")
            time.sleep(30)
    
    # Summarize results
    successful_attempts = sum(1 for r in results if r[0])
    success_rate = (successful_attempts / len(results)) * 100 if results else 0
    
    successful_delays = [r[1] for r in results if r[0] and r[1] is not None]
    avg_delay = sum(successful_delays) / len(successful_delays) if successful_delays else None
    
    info(f"\n=== Summary for {test_name} ===\n")
    info(f"Success Rate: {success_rate:.1f}% ({successful_attempts}/{len(results)} attempts)\n")
    
    if avg_delay is not None:
        info(f"Average Delay: {avg_delay:.2f} ms\n")
    else:
        info("Average Delay: N/A (no successful pings)\n")
    
    return {
        'test_name': test_name,
        'success_rate': success_rate,
        'avg_delay': avg_delay,
        'results': results,
    }

def run():
    setLogLevel('info')  # Show log output from Mininet

    # Check for proper command-line argument
    if len(sys.argv) < 2 or sys.argv[1] not in ['without-fixes', 'with-stp', 'with-pox']:
        print("Usage: sudo python assignment_q1.py [without-fixes|with-stp|with-pox]")
        print("  without-fixes: Run tests without enabling STP")
        print("  with-stp: Run tests with STP enabled")
        print("  with-pox: Run tests using POX controller (with discovery + l2_learning)")
        sys.exit(1)
    
    mode = sys.argv[1]
    capture_dir = "./captures"
    ensure_dir(capture_dir)  # Make sure the capture directory exists

    # Choose controller based on mode
    if mode == 'with-pox':
        info("\n=== Running using POX Controller ===\n")
        net = Mininet(topo=LoopTopo(), controller=lambda name: RemoteController(name, ip='127.0.0.1', port=6633))
    else:
        net = Mininet(topo=LoopTopo(), controller=DefaultController)

    net.start()  # Start the network

    # Get the required hosts
    h1, h2, h3 = net.get('h1'), net.get('h2'), net.get('h3')
    h5, h7, h8 = net.get('h5'), net.get('h7'), net.get('h8')

    if mode == 'with-stp':
        info(f"\n=== Enabling STP on all switches ===\n")
        for i in range(1, 5):
            sw = net.get(f's{i}')
            sw.cmd(f'ovs-vsctl set bridge s{i} stp_enable=true')
        info("Waiting 30 seconds for STP to converge...\n")
        time.sleep(30)

        # Optional: show current STP state
        for i in range(1, 5):
            sw = net.get(f's{i}')
            info(sw.cmd(f'ovs-ofctl show s{i}'))

    elif mode == 'with-pox':
        info("Waiting 30 seconds for POX discovery to detect topology...\n")
        time.sleep(30)

    # Label to use in filenames
    stp_status = {
        'without-fixes': 'without_stp',
        'with-stp': 'with_stp',
        'with-pox': 'with_pox'
    }[mode]

    info(f"\n=== Running Ping Tests ({mode}) ===\n")
    
    # Run three sets of ping tests
    run_ping_test(h3, h1, "Ping h1 from h3", capture_dir, stp_status)
    run_ping_test(h5, h7, "Ping h7 from h5", capture_dir, stp_status)
    run_ping_test(h8, h2, "Ping h2 from h8", capture_dir, stp_status)

    net.stop()  # Clean up the network

if __name__ == '__main__':
    run()
