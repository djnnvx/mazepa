package icmp

import (
	"net"
	"testing"

	"golang.org/x/net/icmp"
	"golang.org/x/net/ipv4"
	"golang.org/x/net/ipv6"
)

func TestDetectProtocol(t *testing.T) {
	tests := []struct {
		name     string
		addr     string
		expected Protocol
	}{
		{"IPv4 localhost", "127.0.0.1", ProtocolIPv4},
		{"IPv4 any", "0.0.0.0", ProtocolIPv4},
		{"IPv4 public", "8.8.8.8", ProtocolIPv4},
		{"IPv6 localhost", "::1", ProtocolIPv6},
		{"IPv6 any", "::", ProtocolIPv6},
		{"IPv6 full", "2001:4860:4860::8888", ProtocolIPv6},
		{"IPv6 compressed", "fe80::1", ProtocolIPv6},
		{"invalid address", "invalid", ProtocolIPv4},
		{"empty string", "", ProtocolIPv4},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := DetectProtocol(tt.addr)
			if result != tt.expected {
				t.Errorf("DetectProtocol(%q) = %v, want %v", tt.addr, result, tt.expected)
			}
		})
	}
}

func TestGetNetworkConfig(t *testing.T) {
	tests := []struct {
		name           string
		proto          Protocol
		wantNetwork    string
		wantListenAddr string
		wantICMPProto  int
	}{
		{
			"IPv4",
			ProtocolIPv4,
			NetworkIPv4,
			ListenAddrV4,
			ipv4.ICMPTypeEcho.Protocol(),
		},
		{
			"IPv6",
			ProtocolIPv6,
			NetworkIPv6,
			ListenAddrV6,
			ipv6.ICMPTypeEchoRequest.Protocol(),
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			network, listenAddr, icmpProto := getNetworkConfig(tt.proto)
			if network != tt.wantNetwork {
				t.Errorf("network = %q, want %q", network, tt.wantNetwork)
			}
			if listenAddr != tt.wantListenAddr {
				t.Errorf("listenAddr = %q, want %q", listenAddr, tt.wantListenAddr)
			}
			if icmpProto != tt.wantICMPProto {
				t.Errorf("icmpProto = %d, want %d", icmpProto, tt.wantICMPProto)
			}
		})
	}
}

func TestGetEchoTypes(t *testing.T) {
	tests := []struct {
		name            string
		proto           Protocol
		wantEchoRequest icmp.Type
		wantEchoReply   icmp.Type
	}{
		{
			"IPv4",
			ProtocolIPv4,
			ipv4.ICMPTypeEcho,
			ipv4.ICMPTypeEchoReply,
		},
		{
			"IPv6",
			ProtocolIPv6,
			ipv6.ICMPTypeEchoRequest,
			ipv6.ICMPTypeEchoReply,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			echoRequest, echoReply := getEchoTypes(tt.proto)
			if echoRequest != tt.wantEchoRequest {
				t.Errorf("echoRequest = %v, want %v", echoRequest, tt.wantEchoRequest)
			}
			if echoReply != tt.wantEchoReply {
				t.Errorf("echoReply = %v, want %v", echoReply, tt.wantEchoReply)
			}
		})
	}
}

func TestIsTimeout(t *testing.T) {
	tests := []struct {
		name     string
		err      error
		expected bool
	}{
		{
			"timeout error",
			&net.OpError{Op: "read", Err: &timeoutError{}},
			true,
		},
		{
			"non-timeout OpError",
			&net.OpError{Op: "read", Err: &nonTimeoutError{}},
			false,
		},
		{
			"regular error",
			net.ErrClosed,
			false,
		},
		{
			"nil error",
			nil,
			false,
		},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			result := isTimeout(tt.err)
			if result != tt.expected {
				t.Errorf("isTimeout() = %v, want %v", result, tt.expected)
			}
		})
	}
}

type timeoutError struct{}

func (e *timeoutError) Error() string   { return "timeout" }
func (e *timeoutError) Timeout() bool   { return true }
func (e *timeoutError) Temporary() bool { return true }

type nonTimeoutError struct{}

func (e *nonTimeoutError) Error() string   { return "not timeout" }
func (e *nonTimeoutError) Timeout() bool   { return false }
func (e *nonTimeoutError) Temporary() bool { return false }

func TestCreateEchoReply(t *testing.T) {
	echoBody := &icmp.Echo{
		ID:   1234,
		Seq:  5678,
		Data: []byte("test data"),
	}

	tests := []struct {
		name      string
		replyType icmp.Type
	}{
		{"IPv4 reply", ipv4.ICMPTypeEchoReply},
		{"IPv6 reply", ipv6.ICMPTypeEchoReply},
	}

	for _, tt := range tests {
		t.Run(tt.name, func(t *testing.T) {
			reply := createEchoReply(echoBody, tt.replyType)

			if reply.Type != tt.replyType {
				t.Errorf("Type = %v, want %v", reply.Type, tt.replyType)
			}
			if reply.Code != 0 {
				t.Errorf("Code = %d, want 0", reply.Code)
			}

			replyBody := reply.Body.(*icmp.Echo)
			if replyBody.ID != echoBody.ID {
				t.Errorf("ID = %d, want %d", replyBody.ID, echoBody.ID)
			}
			if replyBody.Seq != echoBody.Seq {
				t.Errorf("Seq = %d, want %d", replyBody.Seq, echoBody.Seq)
			}
			if string(replyBody.Data) != string(echoBody.Data) {
				t.Errorf("Data = %q, want %q", replyBody.Data, echoBody.Data)
			}
		})
	}
}

func TestParseICMPMessage(t *testing.T) {
	echoMsg := icmp.Message{
		Type: ipv4.ICMPTypeEcho,
		Code: 0,
		Body: &icmp.Echo{
			ID:   100,
			Seq:  1,
			Data: []byte("hello"),
		},
	}

	data, err := echoMsg.Marshal(nil)
	if err != nil {
		t.Fatalf("Failed to marshal test message: %v", err)
	}

	parsed, err := parseICMPMessage(data, ipv4.ICMPTypeEcho.Protocol())
	if err != nil {
		t.Fatalf("parseICMPMessage failed: %v", err)
	}

	if parsed.Type != ipv4.ICMPTypeEcho {
		t.Errorf("Type = %v, want %v", parsed.Type, ipv4.ICMPTypeEcho)
	}

	parsedBody := parsed.Body.(*icmp.Echo)
	if parsedBody.ID != 100 {
		t.Errorf("ID = %d, want 100", parsedBody.ID)
	}
	if parsedBody.Seq != 1 {
		t.Errorf("Seq = %d, want 1", parsedBody.Seq)
	}
}

func TestConstants(t *testing.T) {
	if NetworkIPv4 != "ip4:icmp" {
		t.Errorf("NetworkIPv4 = %q, want %q", NetworkIPv4, "ip4:icmp")
	}
	if NetworkIPv6 != "ip6:ipv6-icmp" {
		t.Errorf("NetworkIPv6 = %q, want %q", NetworkIPv6, "ip6:ipv6-icmp")
	}
	if ListenAddrV4 != "0.0.0.0" {
		t.Errorf("ListenAddrV4 = %q, want %q", ListenAddrV4, "0.0.0.0")
	}
	if ListenAddrV6 != "::" {
		t.Errorf("ListenAddrV6 = %q, want %q", ListenAddrV6, "::")
	}
	if MaxPacketSize != 1500 {
		t.Errorf("MaxPacketSize = %d, want 1500", MaxPacketSize)
	}
}
