#include <core.p4>

header Ethernet_h {
    bit<48> dstAddr;
    bit<48> srcAddr;
    bit<16> etherType;
}

struct Parsed_packet {
    Ethernet_h ethernet;
}

extern ValueSet {
    ValueSet(bit<32> size);
    bit<8> index(in bit<16> proto);
}

#define ETH_KIND_TRILL 1
#define ETH_KIND_TPID  2

parser TopParser(packet_in b, out Parsed_packet p) {
    ValueSet(5) ethtype_kinds;
    state start {
        b.extract(p.ethernet);
        transition select(p.ethernet.etherType) {
            16w0x0800: parse_ipv4;
            16w0x0806: parse_arp;
            16w0x86DD: parse_ipv6;
            default: dispatch_value_sets;
        }
    }

    state dispatch_value_sets {
        bit<8> setIndex = ethtype_kinds.index(p.ethernet.etherType);
        transition select(setIndex) {
            ETH_KIND_TRILL: parse_trill;
            ETH_KIND_TPID: parse_vlan_tag;
        }
    }

    state parse_ipv4     { transition accept; }
    state parse_arp      { transition accept; }
    state parse_ipv6     { transition accept; }
    state parse_trill    { transition accept; }
    state parse_vlan_tag { transition accept; }
}

parser proto<T>(packet_in p, out T h);
package top<T>(proto<T> _p);

top(TopParser()) main;

