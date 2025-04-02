package icmp

import (
	"fmt"
	"net"
)

/*
   https://darkcoding.net/uncategorized/raw-sockets-in-go-ip-layer
*/

func ListenAndPrint(addr string) {

	netaddr, _ := net.ResolveIPAddr("ip4", addr)
	conn, _ := net.ListenIP("ip4:icmp", netaddr)

	buf := make([]byte, 1024)
	numRead, _, _ := conn.ReadFrom(buf)

	fmt.Printf("% X\n", buf[:numRead])

}
