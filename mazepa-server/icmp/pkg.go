package icmp

import (
	"log"
	"net"
	"time"

	"golang.org/x/net/icmp"
	"golang.org/x/net/ipv4"
)

// The network must be "ip4:icmp" for IPv4 ICMP.
const network = "ip4:icmp"

func ListenAndPrint(addr string) {

	conn, err := icmp.ListenPacket(network, "0.0.0.0")
	if err != nil {
		log.Fatalf("ListenPacket failed: %v\nRun with sudo or elevated permissions.", err)
	}
	defer conn.Close()

	log.Printf("Listening for ICMP packets on %s...", conn.LocalAddr().String())
	rb := make([]byte, 1500)

	for {
		/* Set a read deadline to prevent the program from blocking indefinitely */
		err := conn.SetReadDeadline(time.Now().Add(time.Second * 5))
		if err != nil {
			log.Printf("Error setting read deadline: %v", err)
			continue
		}

		n, peer, err := conn.ReadFrom(rb)
		if err != nil {
			if opErr, ok := err.(*net.OpError); ok && opErr.Timeout() {
				continue
			}
			log.Printf("ReadFrom failed: %v", err)
			continue
		}

		rm, err := icmp.ParseMessage(ipv4.ICMPTypeEcho.Protocol(), rb[:n])
		if err != nil {
			log.Printf("ParseMessage failed: %v", err)
			continue
		}

		switch rm.Type {
		case ipv4.ICMPTypeEcho:
			echoBody := rm.Body.(*icmp.Echo)
			log.Printf("Received Echo Request from %v: ID=%d, Seq=%d",
				peer, echoBody.ID, echoBody.Seq)

			wm := icmp.Message{
				Type: ipv4.ICMPTypeEchoReply, // Change type to Reply
				Code: 0,
				Body: &icmp.Echo{
					ID:   echoBody.ID,
					Seq:  echoBody.Seq,
					Data: echoBody.Data,
				},
			}

			wb, err := wm.Marshal(nil)
			if err != nil {
				log.Printf("Marshal failed: %v", err)
				continue
			}

			if _, err := conn.WriteTo(wb, peer); err != nil {
				log.Printf("WriteTo failed: %v", err)
			}
			log.Printf("Sent Echo Reply to %v", peer)

		case ipv4.ICMPTypeEchoReply:
			log.Printf("Received Echo Reply from %v", peer)

		default:
			log.Printf("Received unhandled ICMP message type %v from %v", rm.Type, peer)
		}
	}
}
