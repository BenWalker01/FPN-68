package main

import (
	"log"
	"strings"
	"time"

	"github.com/google/gopacket"
	"github.com/google/gopacket/layers"
	"github.com/google/gopacket/pcap"
)

func main() {

	// Your Ethernet interface
	iface := `\Device\NPF_{A5BBE2A3-8FD1-4551-A386-3E6B01AF0898}`

	// Open packet capture
	handle, err := pcap.OpenLive(iface, 1600, true, pcap.BlockForever)
	if err != nil {
		log.Fatal("Pcap error:", err)
	}
	defer handle.Close()

	// Filter EuroScope traffic
	if err := handle.SetBPFFilter("tcp port 6809"); err != nil {
		log.Fatal("BPF error:", err)
	}

	packetSource := gopacket.NewPacketSource(handle, handle.LinkType())

	// Track last update time per aircraft
	lastSeen := make(map[string]time.Time)

	log.Println("Capturing position updates...")

	for packet := range packetSource.Packets() {

		if packet.NetworkLayer() == nil {
			continue
		}

		tcpLayer := packet.Layer(layers.LayerTypeTCP)
		if tcpLayer == nil {
			continue
		}

		tcp := tcpLayer.(*layers.TCP)

		// Only inbound data (server → you)
		if tcp.SrcPort != 6809 {
			continue
		}

		payload := tcp.Payload
		if len(payload) == 0 {
			continue
		}

		data := string(payload)

		// Split into lines (important: packets can contain multiple messages)
		lines := strings.Split(data, "\n")

		for _, line := range lines {

			// Only care about position updates
			if !strings.HasPrefix(line, "@N:") {
				continue
			}

			parts := strings.Split(line, ":")
			if len(parts) < 2 {
				continue
			}

			callsign := parts[1]
			now := time.Now()

			if last, ok := lastSeen[callsign]; ok {
				delta := now.Sub(last)

				log.Printf("%s update interval: %.2fs\n",
					callsign,
					delta.Seconds(),
				)
			}

			lastSeen[callsign] = now
		}
	}
}