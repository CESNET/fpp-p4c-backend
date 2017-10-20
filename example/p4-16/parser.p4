#include <core.p4>

#include "headers.p4"

struct headers_s
{
   ethernet_h eth;
   ipv4_h ipv4;
   ipv6_h ipv6;
   tcp_h tcp;
   udp_h udp;
   payload_h payload;
}

parser prs(packet_in packet, out headers_s headers)
{
   ieee802_1q_h vlan_q;
   mpls_h mpls;
   eompls_h eompls;
   pppoe_h pppoe;
   gre_h gre;
   gre_sre_h gre_sre;
   l2tp_h l2tp;
   teredo_auth_h teredo_auth;
   teredo_origin_h teredo_origin;
   pptp_uncomp_proto_h pptp_uncomp_proto;
   pptp_comp_proto_h pptp_comp_proto;

   bit<16> udp_src_port;

   state start {
      transition parse_ethernet;
   }

   state parse_ethernet {
      packet.extract(headers.eth);

      transition select(headers.eth.ethertype) {
         ETHERTYPE_IPV4: parse_ipv4;
         ETHERTYPE_IPV6: parse_ipv6;
         ETHERTYPE_MPLS_UNICAST: parse_mpls;
         ETHERTYPE_MPLS_MULTICAST: parse_mpls;
         ETHERTYPE_8021Q: parse_vlan_q;
         default: reject;
      }
   }

   state parse_vlan_q {
      packet.extract(vlan_q);

      transition select(vlan_q.ethertype) {
         ETHERTYPE_IPV4: parse_ipv4;
         ETHERTYPE_IPV6: parse_ipv6;
         ETHERTYPE_MPLS_UNICAST: parse_mpls;
         ETHERTYPE_MPLS_MULTICAST: parse_mpls;
         ETHERTYPE_8021Q: parse_vlan_q;
         default: reject;
      }
   }

   state parse_mpls {
      packet.extract(mpls);
      transition select(mpls.bos) {
         0: parse_mpls;
         1: parse_mpls_end;
         default: reject;
      }
   }

   state parse_mpls_end {
      transition select(packet.lookahead<bit<4>>()) {
         4: parse_ipv4;
         6: parse_ipv6;
         0: parse_eompls;
         default: reject;
      }
   }

   state parse_eompls {
      packet.extract(eompls);

      transition parse_ethernet;
   }

   state parse_pptp {
      transition select(packet.lookahead<bit<16>>()) {
         0xFF03: parse_pptp_uncomp_addr_cntrl;
         default: parse_pptp_comp_addr_cntrl;
      }
   }

   state parse_pptp_uncomp_addr_cntrl {
      /* Skip address and control fields. */
      packet.advance(16);

      transition select(packet.lookahead<bit<8>>() & 0x01) {
         0: parse_pptp_uncomp_proto;
         1: parse_pptp_comp_proto;
         default: reject;
      }
   }

   state parse_pptp_comp_addr_cntrl {
      transition select(packet.lookahead<bit<8>>() & 0x01) {
         0: parse_pptp_uncomp_proto;
         1: parse_pptp_comp_proto;
         default: reject;
      }
   }

   state parse_pptp_uncomp_proto {
      packet.extract(pptp_uncomp_proto);

      transition select((bit<16>)pptp_uncomp_proto.proto) {
         PPP_IPV4: parse_ipv4;
         PPP_IPV6: parse_ipv6;
         PPP_COMP: accept;
         PPP_CONTROL: accept;
         default: reject;
      }
   }

   state parse_pptp_comp_proto {
      packet.extract(pptp_comp_proto);

      transition select((bit<16>)pptp_comp_proto.proto) {
         PPP_IPV4: parse_ipv4;
         PPP_IPV6: parse_ipv6;
         PPP_COMP: accept;
         PPP_CONTROL: accept;
         default: reject;
      }
   }

   state parse_ipv4 {
      packet.extract(headers.ipv4);

      /* Skip IP options. */
      packet.advance((bit<32>)(((int<32>)(bit<32>)headers.ipv4.ihl - (int<32>)5) * 32));

      transition select(headers.ipv4.protocol) {
         IPPROTO_TCP: parse_tcp;
         IPPROTO_UDP: parse_udp;
         IPPROTO_ICMP: accept;
         IPPROTO_GRE: parse_gre;
         IPPROTO_IPIP: parse_ipv4;
         IPPROTO_IPV6: parse_ipv6;
         default: reject;
      }
   }

   state parse_ipv6 {
      packet.extract(headers.ipv6);

      transition select(headers.ipv6.next_hdr) {
         IPPROTO_TCP: parse_tcp;
         IPPROTO_UDP: parse_udp;
         IPPROTO_ICMPV6: accept;
         IPPROTO_GRE: parse_gre;
         IPPROTO_IPIP: parse_ipv4;
         IPPROTO_IPV6: parse_ipv6;
         default: reject;
      }
   }

   state parse_gre {
      packet.extract(gre);

      transition select(gre.ver) {
         0: parse_gre_v0;
         1: parse_gre_v1;
         default: reject;
      }
   }

   state parse_gre_v0 {
      /* Skip optional fields. */
      packet.advance((((bit<32>)gre.C | (bit<32>)gre.R) * 32));
      packet.advance(((bit<32>)gre.K * 32));
      packet.advance(((bit<32>)gre.S * 32));

      transition select(gre.R) {
         1: parse_gre_sre;
         0: parse_gre_v0_fin;
         default: reject;
      }
   }

   state parse_gre_v0_fin {
      transition select(gre.proto) {
         GRE_IPV4: parse_ipv4;
         GRE_IPV6: parse_ipv6;
         GRE_PPP: parse_pptp;
         GRE_ETH: parse_ethernet;
         ETHERTYPE_MPLS_UNICAST: parse_mpls;
         ETHERTYPE_MPLS_MULTICAST: parse_mpls;
         default: reject;
      }
   }

   state parse_gre_v1 {
      /* Skip fields. */
      packet.advance(32);
      packet.advance((bit<32>)gre.S * 32);
      packet.advance((bit<32>)gre.A * 32);

      transition select(gre.proto) {
         GRE_IPV4: parse_ipv4;
         GRE_IPV6: parse_ipv6;
         GRE_PPP: parse_pptp;
         GRE_ETH: parse_ethernet;
         ETHERTYPE_MPLS_UNICAST: parse_mpls;
         ETHERTYPE_MPLS_MULTICAST: parse_mpls;
         default: reject;
      }
   }

   state parse_gre_sre {
      packet.extract(gre_sre);
      packet.advance((bit<32>)gre_sre.length);

      transition select(gre_sre.length) {
         0: parse_gre_v0_fin;
         default: parse_gre_sre;
      }
   }

   state parse_l2tp {
      packet.extract(l2tp);

      transition select(l2tp.version) {
         2: parse_l2tp_v2;
         default: reject;
      }
   }

   state parse_l2tp_v2 {
      packet.advance((bit<32>)l2tp.length * 16);
      packet.advance((bit<32>)32);
      packet.advance((bit<32>)l2tp.seq * 32);
      packet.advance((bit<32>)l2tp.offset * (bit<32>)(packet.lookahead<bit<32>>()) * 8);
      packet.advance((bit<32>)l2tp.offset * 16);

      transition select(l2tp.type) {
         0: parse_pptp;
         default: reject;
      }
   }

   state parse_teredo {
      transition select(packet.lookahead<bit<4>>()) {
         6: parse_ipv6;
         0: parse_teredo_hdr;
         default: reject;
      }
   }

   state parse_teredo_hdr {
      transition select(packet.lookahead<bit<16>>()) {
         0x0001: parse_teredo_auth_hdr;
         0x0000: parse_teredo_origin_hdr;
         default: reject;
      }
   }

   state parse_teredo_auth_hdr {
      packet.extract(teredo_auth);

      /* Skip auth, id, confirmation byte and nonce. */
      packet.advance((bit<32>)teredo_auth.id_len * 8 + (bit<32>)teredo_auth.auth_len * 8 + 72);

      transition select(packet.lookahead<bit<4>>()) {
         6: parse_ipv6;
         0: parse_teredo_hdr;
         default: reject;
      }
   }

   state parse_teredo_origin_hdr {
      packet.extract(teredo_origin);

      transition select(packet.lookahead<bit<4>>()) {
         6: parse_ipv6;
         0: parse_teredo_hdr;
         default: reject;
      }
   }

   state parse_tcp {
      packet.extract(headers.tcp);

      /* Skip TCP options. */
      packet.advance((bit<32>)(((int<32>)(bit<32>)headers.tcp.data_offset - (int<32>)5) * 32));

      transition parse_payload;
   }

   state parse_udp {
      packet.extract(headers.udp);

      udp_src_port = headers.udp.src_port;

      transition select(headers.udp.dst_port) {
         1701: parse_l2tp;
         3544: parse_teredo;
         default: parse_udp_2;
      }
   }

   state parse_udp_2 {
      transition select(udp_src_port) {
         1701: parse_l2tp;
         3544: parse_teredo;
         default: parse_payload;
      }
   }

   state parse_payload {
      /* Get position of the payload. */
      packet.extract(headers.payload);
      transition accept;
   }
}

parser parse<H>(packet_in packet, out H headers);
package top<H>(parse<H> prs);

top(prs()) main;

