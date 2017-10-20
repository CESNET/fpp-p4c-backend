/* Header definitions and constants. */

#define ETHERTYPE_8021Q          0x8100
#define ETHERTYPE_MPLS_UNICAST   0x8847
#define ETHERTYPE_MPLS_MULTICAST 0x8848
#define ETHERTYPE_IPV4           0x0800
#define ETHERTYPE_IPV6           0x86DD

#define IPPROTO_ICMP       1
#define IPPROTO_IPIP       4
#define IPPROTO_TCP        6
#define IPPROTO_UDP        17
#define IPPROTO_IPV6       41
#define IPPROTO_GRE        47
#define IPPROTO_ICMPV6     58

#define PPP_IPV4     0x0021
#define PPP_IPV6     0x0057
#define PPP_COMP     0x00FD
#define PPP_CONTROL  0xC021

#define GRE_IPV4  0x0800
#define GRE_IPV6  0x86DD
#define GRE_PPP   0x880B
#define GRE_ETH   0x6558

header ethernet_h {
   bit<48> dst_addr;
   bit<48> src_addr;
   bit<16> ethertype;
}

header ieee802_1q_h {
   bit<3> pcp;
   bit<1> cfi;
   bit<12> vid;
   bit<16> ethertype;
}

header mpls_h {
   bit<20> label;
   bit<3> tc;
   bit<1> bos;
   bit<8> ttl;
}

header eompls_h {
   bit<4> zero;
   bit<12> res;
   bit<16> seq_num;
}

header pppoe_h {
   bit<4> version;
   bit<4> type;
   bit<8> code;
   bit<16> sid;
   bit<16> len;
}

header pptp_comp_h {
   bit<16> proto;
}

header pptp_uncomp_h {
   bit<8> address;
   bit<8> cntrl;
   bit<16> proto;
}

header pptp_uncomp_proto_h {
   bit<16> proto;
}

header pptp_comp_proto_h {
   bit<8> proto;
}

header ipv4_h {
   bit<4> version;
   bit<4> ihl;
   bit<8> diffserv;
   bit<16> total_len;
   bit<16> identification;
   bit<3> flags;
   bit<13> frag_offset;
   bit<8> ttl;
   bit<8> protocol;
   bit<16> hdr_checksum;
   bit<32> src_addr;
   bit<32> dst_addr;
}

header ipv6_h {
   bit<4> version;
   bit<8> traffic_class;
   bit<20> flow_label;
   bit<16> payload_len;
   bit<8> next_hdr;
   bit<8> hop_limit;
   bit<128> src_addr;
   bit<128> dst_addr;
}

header gre_h {
   bit<1> C;
   bit<1> R;
   bit<1> K;
   bit<1> S;
   bit<1> s;
   bit<3> recur;
   bit<1> A;
   bit<4> flags;
   bit<3> ver;
   bit<16> proto;
   // type specific fields
}

header gre_sre_h {
   bit<16> addr_family;
   bit<8> offset;
   bit<8> length;
}

header l2tp_h {
   bit<1> type;
   bit<1> length;
   bit<2> res1;
   bit<1> seq;
   bit<1> res2;
   bit<1> offset;
   bit<1> priority;
   bit<4> res3;
   bit<4> version;
   // optional fields
}

header tcp_h {
   bit<16> src_port;
   bit<16> dst_port;
   bit<32> seq_num;
   bit<32> ack_num;
   bit<4> data_offset;
   bit<4> res;
   bit<8> flags;
   bit<16> window;
   bit<16> checksum;
   bit<16> urgent_ptr;
   // options varlen
}

header udp_h {
   bit<16> src_port;
   bit<16> dst_port;
   bit<16> len;
   bit<16> checksum;
}

header teredo_auth_h {
   bit<8> zero;
   bit<8> type;
   bit<8> id_len;
   bit<8> auth_len;
   // variable length fields
}

header teredo_origin_h {
   bit<8> zero;
   bit<8> type;
   bit<16> port;
   bit<32> ip;
}

header payload_h {
}

