package main

import (
	"fmt"
	"github.com/google/gopacket/pcap"
)

func main() {
	devices, _ := pcap.FindAllDevs()
	for _, d := range devices {
		fmt.Println(d.Name, "-", d.Description)
	}
}