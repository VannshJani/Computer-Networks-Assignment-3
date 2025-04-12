from mininet.topo import Topo
from mininet.net import Mininet
from mininet.node import DefaultController
from mininet.cli import CLI
from mininet.link import TCLink
from mininet.log import setLogLevel, info
import time

class NATTopo(Topo):
    def build(self):
        # Add switches
        s1, s2, s3, s4 = [self.addSwitch(f's{i}') for i in range(1, 5)]

        # Public hosts h3 to h8 (172.16.10.0/24)
        h3 = self.addHost('h3', ip='172.16.10.4/24')
        h4 = self.addHost('h4', ip='172.16.10.5/24')
        h5 = self.addHost('h5', ip='172.16.10.6/24')
        h6 = self.addHost('h6', ip='172.16.10.7/24')
        h7 = self.addHost('h7', ip='172.16.10.8/24')
        h8 = self.addHost('h8', ip='172.16.10.9/24')

        # Host-switch connections
        self.addLink(h3, s2, cls=TCLink, delay='5ms')
        self.addLink(h4, s2, cls=TCLink, delay='5ms')
        self.addLink(h5, s3, cls=TCLink, delay='5ms')
        self.addLink(h6, s3, cls=TCLink, delay='5ms')
        self.addLink(h7, s4, cls=TCLink, delay='5ms')
        self.addLink(h8, s4, cls=TCLink, delay='5ms')

        # Inter-switch links (7ms)
        for a, b in [(s1, s2), (s2, s3), (s3, s4), (s4, s1), (s1, s3)]:
            self.addLink(a, b, cls=TCLink, delay='7ms')

        # NAT and private hosts
        h9 = self.addHost('h9')
        h1 = self.addHost('h1')
        h2 = self.addHost('h2')

        # Private links to NAT
        self.addLink(h1, h9, intfName2='h9-eth0', cls=TCLink, delay='5ms')
        self.addLink(h2, h9, intfName2='h9-eth1', cls=TCLink, delay='5ms')
        self.addLink(h9, s1, intfName1='h9-eth2', cls=TCLink, delay='5ms')


def configure_initial_ips(net):
    h1, h2, h9 = net.get('h1', 'h2', 'h9')

    # Assign internal IPs — using two subnets
    h1.setIP('10.1.1.2/24', intf='h1-eth0')
    h2.setIP('10.1.2.2/24', intf='h2-eth0')
    h9.setIP('10.1.1.1/24', intf='h9-eth0')
    h9.setIP('10.1.2.1/24', intf='h9-eth1')
    h9.setIP('172.16.10.10/24', intf='h9-eth2')

    # Default routes for internal hosts
    h1.cmd('ip route add default via 10.1.1.1')
    h2.cmd('ip route add default via 10.1.2.1')


def fix_ping(net):
    h1, h2, h9 = net.get('h1', 'h2', 'h9')

    # Enable IP forwarding
    h9.cmd('sysctl -w net.ipv4.ip_forward=1')

    # NAT (MASQUERADE)
    h9.cmd('iptables -t nat -A POSTROUTING -o h9-eth2 -j MASQUERADE')

    # Forwarding rules
    h9.cmd('iptables -A FORWARD -i h9-eth0 -o h9-eth2 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth1 -o h9-eth2 -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth2 -o h9-eth0 -m state --state RELATED,ESTABLISHED -j ACCEPT')
    h9.cmd('iptables -A FORWARD -i h9-eth2 -o h9-eth1 -m state --state RELATED,ESTABLISHED -j ACCEPT')

    # Port forwarding
    h9.cmd('iptables -t nat -A PREROUTING -i h9-eth2 -p tcp --dport 5001 -j DNAT --to-destination 10.1.1.2:5001')
    h9.cmd('iptables -A FORWARD -p tcp -d 10.1.1.2 --dport 5001 -j ACCEPT')

    h9.cmd('iptables -t nat -A PREROUTING -i h9-eth2 -p tcp --dport 5002 -j DNAT --to-destination 10.1.2.2:5002')
    h9.cmd('iptables -A FORWARD -p tcp -d 10.1.2.2 --dport 5002 -j ACCEPT')

    # Default route for public hosts
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
    

    print("i) h1 (server) <-> h6 (client)")
    h1.cmd('iperf3 -s -D') 
    result1 = h6.cmd(f'iperf3 -c {h1.IP()} -t 120')
    print(result1)
    h1.cmd('pkill iperf3')  # Stop the server

    # Test 2: h8 server, h2 client
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
    for i in range(1, 5):
        sw = net.get(f's{i}')
        sw.cmd(f'ovs-vsctl set bridge s{i} stp_enable=true')
    time.sleep(30)


def run():
    setLogLevel('info')
    topo = NATTopo()
    net = Mininet(topo=topo, link=TCLink, controller=DefaultController)
    net.start()

    configure_initial_ips(net)
    enable_stp(net)

    run_tests("Updated topology with h9 as NAT", net)

    fix_ping(net)

    run_tests("After fix_ping() (changes to make tests succeed)", net)

    CLI(net)
    net.stop()


if __name__ == '__main__':
    run()
