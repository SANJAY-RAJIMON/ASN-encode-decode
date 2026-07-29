#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codec.h"
#include "protocol_map.h"

// Turn on ASN.1 debug by overriding asn_debug macro in a local way, or just compile with -DEMIT_ASN_DEBUG=1
int main() {
    codec_protocol_t protocol = codec_protocol_from_name("NGAP");
    
    // Hex from test_packet_18_NGAP.xml? Wait, the user didn't provide the hex!
    // But I can parse test_packet_18_NGAP.xml from parse_log.py! No, the XML is truncated, the hex was in the log!
    // I can get the hex by looking at test_packet_18_NGAP.xml? No.
    // I will write a script to extract the hex for packet 18 from the original log. But I don't have the log name!
    return 0;
}
