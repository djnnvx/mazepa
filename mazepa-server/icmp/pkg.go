package icmp

import (
	"log"
	"net"
	"time"

	"golang.org/x/net/icmp"
	"golang.org/x/net/ipv4"
	"golang.org/x/net/ipv6"
)

const (
	NetworkIPv4   = "ip4:icmp"
	NetworkIPv6   = "ip6:ipv6-icmp"
	ListenAddrV4  = "0.0.0.0"
	ListenAddrV6  = "::"
	ReadTimeout   = 5 * time.Second
	MaxPacketSize = 1500
)

type Protocol int

const (
	ProtocolIPv4 Protocol = iota
	ProtocolIPv6
)

func DetectProtocol(addr string) Protocol {
	ip := net.ParseIP(addr)
	if ip == nil {
		return ProtocolIPv4
	}
	if ip.To4() != nil {
		return ProtocolIPv4
	}
	return ProtocolIPv6
}

func getNetworkConfig(proto Protocol) (network, listenAddr string, icmpProto int) {
	if proto == ProtocolIPv6 {
		return NetworkIPv6, ListenAddrV6, ipv6.ICMPTypeEchoRequest.Protocol()
	}
	return NetworkIPv4, ListenAddrV4, ipv4.ICMPTypeEcho.Protocol()
}

func getEchoTypes(proto Protocol) (echoRequest, echoReply icmp.Type) {
	if proto == ProtocolIPv6 {
		return ipv6.ICMPTypeEchoRequest, ipv6.ICMPTypeEchoReply
	}
	return ipv4.ICMPTypeEcho, ipv4.ICMPTypeEchoReply
}

func createConnection(network, listenAddr string) (*icmp.PacketConn, error) {
	conn, err := icmp.ListenPacket(network, listenAddr)
	if err != nil {
		return nil, err
	}
	return conn, nil
}

func setReadDeadline(conn *icmp.PacketConn) error {
	return conn.SetReadDeadline(time.Now().Add(ReadTimeout))
}

func readPacket(conn *icmp.PacketConn, buffer []byte) (int, net.Addr, error) {
	return conn.ReadFrom(buffer)
}

func isTimeout(err error) bool {
	opErr, ok := err.(*net.OpError)
	return ok && opErr.Timeout()
}

func parseICMPMessage(data []byte, proto int) (*icmp.Message, error) {
	return icmp.ParseMessage(proto, data)
}

func createEchoReply(echoBody *icmp.Echo, replyType icmp.Type) *icmp.Message {
	return &icmp.Message{
		Type: replyType,
		Code: 0,
		Body: &icmp.Echo{
			ID:   echoBody.ID,
			Seq:  echoBody.Seq,
			Data: echoBody.Data,
		},
	}
}

func sendReply(conn *icmp.PacketConn, msg *icmp.Message, peer net.Addr) error {
	data, err := msg.Marshal(nil)
	if err != nil {
		return err
	}
	_, err = conn.WriteTo(data, peer)
	return err
}

func handleEchoRequest(conn *icmp.PacketConn, msg *icmp.Message, peer net.Addr, replyType icmp.Type) {
	echoBody := msg.Body.(*icmp.Echo)

	if len(echoBody.Data) > 0 {
		log.Printf("[%v] %s", peer, string(echoBody.Data))
	}

	reply := createEchoReply(echoBody, replyType)
	if err := sendReply(conn, reply, peer); err != nil {
		log.Printf("Failed to send reply: %v", err)
		return
	}
}

func handleEchoReply(peer net.Addr) {
	log.Printf("Received Echo Reply from %v", peer)
}

func handleUnknownType(msgType icmp.Type, peer net.Addr) {
	// Silently ignore destination unreachable and other noise
	_ = msgType
	_ = peer
}

func processMessage(conn *icmp.PacketConn, msg *icmp.Message, peer net.Addr, echoRequest, echoReply icmp.Type) {
	switch msg.Type {
	case echoRequest:
		handleEchoRequest(conn, msg, peer, echoReply)
	case echoReply:
		handleEchoReply(peer)
	default:
		handleUnknownType(msg.Type, peer)
	}
}

func ListenAndPrint(addr string) {
	proto := DetectProtocol(addr)
	network, listenAddr, icmpProto := getNetworkConfig(proto)
	echoRequest, echoReply := getEchoTypes(proto)

	conn, err := createConnection(network, listenAddr)
	if err != nil {
		log.Fatalf("ListenPacket failed: %v\nRun with sudo or elevated permissions.", err)
	}
	defer conn.Close()

	log.Printf("Listening for ICMP packets on %s...", conn.LocalAddr().String())
	buffer := make([]byte, MaxPacketSize)

	for {
		if err := setReadDeadline(conn); err != nil {
			log.Printf("Error setting read deadline: %v", err)
			continue
		}

		n, peer, err := readPacket(conn, buffer)
		if err != nil {
			if isTimeout(err) {
				continue
			}
			log.Printf("ReadFrom failed: %v", err)
			continue
		}

		msg, err := parseICMPMessage(buffer[:n], icmpProto)
		if err != nil {
			log.Printf("ParseMessage failed: %v", err)
			continue
		}

		processMessage(conn, msg, peer, echoRequest, echoReply)
	}
}
