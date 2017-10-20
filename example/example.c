#include <stdio.h>
#include <time.h>
#include <netinet/ether.h>
#include <pcap.h>

#include "parser.h"

#define PRINT_OFFSET(proto, offset) printf("  %s offset=%u:\n", proto, offset);

void process_packet(const struct pcap_pkthdr *pcap_hdr, packet_hdr_t *headers, int status)
{
   packet_hdr_t *tmp = headers;
   time_t time;
   char ts[32];
   char *stat;

   char src_ip[INET6_ADDRSTRLEN];
   char dst_ip[INET6_ADDRSTRLEN];
   struct ethernet_h *eth;
   struct ipv4_h *ipv4;
   struct ipv6_h *ipv6;
   struct tcp_h *tcp;
   struct udp_h *udp;
   struct payload_h *payload;

   if (status == NoError) {
      stat = "accept";
   } else {
      stat = "reject";
   }

   time = pcap_hdr->ts.tv_sec;
   strftime(ts, sizeof(ts), "%FT%T", localtime(&time));
   printf("Packet ts=%s.%06lu, length=%u, status=%s:\n", ts, pcap_hdr->ts.tv_usec, pcap_hdr->caplen, stat);

   while (headers != NULL) {
      if (headers->type == fpp_ethernet_h) {
         eth = headers->hdr;

         PRINT_OFFSET("ETHERNET", eth->header_offset);
         printf("      src-mac=\t%s\n", ether_ntoa((const struct ether_addr *) eth->src_addr));
         printf("      dst-mac=\t%s\n", ether_ntoa((const struct ether_addr *) eth->dst_addr));
         printf("      ethtype=\t%#02x\n", eth->ethertype);
      } else if (headers->type == fpp_ipv4_h) {
         ipv4 = headers->hdr;

         inet_ntop(AF_INET, (const void *) &ipv4->src_addr, src_ip, INET6_ADDRSTRLEN);
         inet_ntop(AF_INET, (const void *) &ipv4->dst_addr, dst_ip, INET6_ADDRSTRLEN);

         PRINT_OFFSET("IPv4", ipv4->header_offset);
         printf("      src-ip=\t%s\n", src_ip);
         printf("      dst-ip=\t%s\n", dst_ip);
         printf("      proto=\t%#02x\n", ipv4->protocol);
      } else if (headers->type == fpp_ipv6_h) {
         ipv6 = headers->hdr;

         inet_ntop(AF_INET6, (const void *) &ipv6->src_addr, src_ip, INET6_ADDRSTRLEN);
         inet_ntop(AF_INET6, (const void *) &ipv6->dst_addr, dst_ip, INET6_ADDRSTRLEN);

         PRINT_OFFSET("IPv6", ipv6->header_offset);
         printf("      src-ip=\t%s\n", src_ip);
         printf("      dst-ip=\t%s\n", dst_ip);
         printf("      proto=\t%#02x\n", ipv6->next_hdr);
      } else if (headers->type == fpp_tcp_h) {
         tcp = headers->hdr;

         PRINT_OFFSET("TCP", tcp->header_offset);
         printf("      src-port=\t%u\n", tcp->src_port);
         printf("      dst-port=\t%u\n", tcp->dst_port);
      } else if (headers->type == fpp_udp_h) {
         udp = headers->hdr;

         PRINT_OFFSET("UDP", udp->header_offset);
         printf("      src-port=\t%u\n", udp->src_port);
         printf("      dst-port=\t%u\n", udp->dst_port);
      } else if (headers->type == fpp_payload_h) {
         payload = headers->hdr;
         PRINT_OFFSET("PAYLOAD", payload->header_offset);
      }

      headers = headers->next;
      free(tmp->hdr);
      free(tmp);
      tmp = headers;
   }

   printf("\n");
}

int main(int argc, char *argv[])
{
   int ret;
   int status = 0;
   const char pcap_path[] = "pcaps/tunnel.pcap";
   packet_hdr_t *parsed_headers = NULL;

   char errbuf[PCAP_ERRBUF_SIZE];
   const u_char *packet_raw = NULL;
   struct pcap_pkthdr *hdr = NULL;
   pcap_t *handle = NULL;

   /* Open pcap file. */
   handle = pcap_open_offline(pcap_path, errbuf);
   if (handle == NULL) {
      fprintf(stderr, "Error: unable to open PCAP file: %s\n", errbuf);
      return 1;
   }

   /* Main packet reading loop. */
   while ((ret = pcap_next_ex(handle, &hdr, &packet_raw))) {
      if (ret == -1) {
         fprintf(stderr, "Error: %s\n", pcap_geterr(handle));
         status = 1;
         break;
      } else if (ret == -2) {
         break;
      }

      /* Parse packet. */
      ret = fpp_parse_packet(packet_raw, hdr->caplen, &parsed_headers);
      switch (ret) {
         case NoError:
         case PacketTooShort:
         case NoMatch:
         case StackOutOfBounds:
         case HeaderTooShort:
         case ParserTimeout:
         case ParserDefaultReject:
            break;
         case OutOfMemory:
            fprintf(stderr, "Error: not enough memory for packet headers allocation\n");
            status = 1;
            break;
         default:
            break;
      }

      /* Print parsed headers. */
      process_packet(hdr, parsed_headers, ret);
   }

   /* Cleanup. */
   if (handle != NULL) {
      pcap_close(handle);
   }

   return status;
}
