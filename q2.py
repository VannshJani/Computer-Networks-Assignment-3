from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import DefaultController
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import setLogLevel, info
import time

class NATTopo(Topo):
    def build(self):
        # Create 4 switches for the core network
        s1, s2, s3, s4 = [self.addSwitch(f's{i}') for i in range(1, 5)]

        # Add public hosts with IPs in the 172.16.10.0/24 subnet
        h3 = self.addHost('h3', ip='172.16.10.4/24')
        h4 = self.addHost('h4', ip='172.16.10.5/24')
        h5 = self.addHost('h5', ip='172.16.10.6/24')
        h6 = self.addHost('h6', ip='172.16.10.7/24')
        h7 = self.addHost('h7', ip='172.16.10.8/24')
        h8 = self.addHost('h8', ip='172.16.10.9/24')

        # Connect public hosts to switches with 5ms delay
        self.addLink(h3, s2, cls=TCLink, delay='5ms')
        self.addLink(h4, s2, cls=TCLink, delay='5ms')
        self.addLink(h5, s3, cls=TCLink, delay='5ms')
        self.addLink(h6, s3, cls=TCLink, delay='5ms')
        self.addLink(h7, s4, cls=TCLink, delay='5ms')
        self.addLink(h8, s4, cls=TCLink, delay='5ms')

        # Connect switches in a ring with an extra s1-s3 link (7ms delay)
        for a, b in [(s1, s2), (s2, s3), (s3, s4), (s4, s1), (s1, s3)]:
            self.addLink(a, b, cls=TCLink, delay='7ms')

        # Add NAT host (h9) and two internal hosts (h1, h2)
        h9 = self.addHost('h9')
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')

        # Connect internal hosts to NAT using specific interfaces
        self.addLink(h1, h9, intfName2='h9-eth0', cls=TCLink, delay='5ms')
        self.addLink(h2, h9, intfName2='h9-eth1', cls=TCLink, delay='5ms')
        self.addLink(h9, s1, intfName1='h9-eth2', cls=TCLink, delay='5ms')


def configure_initial_ips(net):
    h1, h2, h9 = net.get('h1', 'h2', 'h9')

    # Assign IP addresses to internal hosts and NAT (h9)
    h1.setIP('10.1.1.2/24', intf='h1-eth0')
    h2.setIP('10.1.2.2/24', intf='h2-eth0')
    h9.setIP('10.1.1.1/24', intf='h9-eth0')
    h9.setIP('10.1.2.1/24', intf='h9-eth1')
    h9.setIP('172.16.10.10/24', intf='h9-eth2')  # Public-facing IP

    # Set default routes for internal hosts through NAT
    h1.cmd('ip route add default via 10.1.1.1')
    h2.cmd('ip route add default via 10.1.2.1')


def fix_ping(net):
    h1, h2, h9 = net.get('h1', 'h2', 'h9')

    # Enable IP forwarding on NAT
    h9.cmd('sysctl -w net.ipv4.ip_forward=1')

    # Set up NAT with iptables (masquerade outgoing traffic)
    h9.cmd('iptables -t nat -A POSTROUTING -o h9-eth2 -j MASQUERADE')

    # Allow forwarding from internal interfaces to public
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth2 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth1 -o h9-eth2 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth2 -o h9-eth0 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth2 -o h9-eth1 -m state --state RELATED,ESTABLISHED -j ACCEPT')

    # Set up port forwarding for iperf3 servers (h1 and h2)
    h9.cmd('iptables -t nat -A PREROUTING -i h9-eth2 -p tcp --dport 5001 -j DNAT --to-destination 10.1.1.2:5001')
    h9.cmd('iptables -A FORWARD -p tcp -d 10.1.1.2 --dport 5001 -j ACCEPT')

    h9.cmd('iptables -t nat -A PREROUTING -i h9-eth2 -p tcp --dport 5002 -j DNAT --to-destination 10.1.2.2:5002')
    h9.cmd('iptables -A FORWARD -p tcp -d 10.1.2.2 --dport 5002 -j ACCEPT')

    # Public hosts use NAT (h9) as their default gateway
    for i in range(3, 9):
        h = net.get(f'h{i}')
        h.cmd('ip route add default via 172.16.10.10')


def run_tests(label, net):
    info(f"\n=== {label} ===\n")
    h1, h2, h3, h5, h6, h8 = net.get('h1', 'h2', 'h3', 'h5', 'h6', 'h8')

    print("1. Internal to External Pings")
    print("h1 -> h5")
    print(h1.cmd('ping -c 4 %s' % h5.IP()))
    print("h2 -> h3")
    print(h2.cmd('ping -c 4 %s' % h3.IP()))

    print("2. External to Internal Pings")
    print("h8 -> h1")
    print(h8.cmd('ping -c 4 %s' % h1.IP()))
    print("h6 -> h2")
    print(h6.cmd('ping -c 4 %s' % h2.IP()))

    print("3. Iperf3 Tests (120s each)")

    # Test 1: h6 (client) to h1 (server)
    print("i) h1 (server) <-> h6 (client)")
    h1.cmd('iperf3 -s -D')  # Start iperf3 in daemon mode
    result1 = h6.cmd(f'iperf3 -c {h1.IP()} -t 120')
    print(result1)
    h1.cmd('pkill iperf3')  # Stop the server

    # Test 2: h2 (client) to h8 (server)
    print("ii) h8 (server) <-> h2 (client)")
    h8.cmd('iperf3 -s -D')
    result2 = h2.cmd(f'iperf3 -c {h8.IP()} -t 120')
    print(result2)
    h8.cmd('pkill iperf3')

    # Test 3: Repeat Test 1 again
    print("iii) h1 (server) <-> h6 (client) again")
    h1.cmd('iperf3 -s -D')
    result3 = h6.cmd(f'iperf3 -c {h1.IP()} -t 120')
    print(result3)
    h1.cmd('pkill iperf3')


def enable_stp(net):
    info("\n=== Enabling STP ===\n")
    # Enable Spanning Tree Protocol on all switches
    for i in range(1, 5):
        sw = net.get(f's{i}')
        sw.cmd(f'ovs-vsctl set bridge s{i} stp_enable=true')
    time.sleep(30)  # Wait for STP to converge


def run():
    setLogLevel('info')  # Show Mininet output
    topo = NATTopo()
    net = Mininet(topo=topo, link=TCLink, controller=DefaultController)
    net.start()

    # Assign IPs to hosts
    configure_initial_ips(net)

    # Enable STP to avoid loops in switch connections
    enable_stp(net)

    # Run tests before NAT fix
    run_tests("Updated topology with h9 as NAT", net)

    # Apply NAT and forwarding rules
    fix_ping(net)

    # Run tests again after NAT fix
    run_tests("After fix_ping() (changes to make tests succeed)", net)

    CLI(net)
    net.stop()


if __name__ == '__main__':
    run()
