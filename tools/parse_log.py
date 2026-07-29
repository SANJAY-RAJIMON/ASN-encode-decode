#!/usr/bin/env python3
import sys
import re
import subprocess
import argparse

def parse_log(filepath):
    # Regex to match the hex dump lines:
    # Pattern: 00000000: 000f4042 00000500 55000200 03002600
    # Also we look for protocol hints like 'NGAP Msg UL' or 'DL_CCCH'
    
    with open(filepath, 'r') as f:
        lines = f.readlines()

    current_protocol = None
    current_hex = []
    
    print(f"Parsing {filepath} for ASN.1 Hex Dumps...\n")
    
    for i, line in enumerate(lines):
        # Detect Protocol Start
        if "NGAP Msg" in line or "INITIAL_UE_MESSAGE" in line:
            current_protocol = "NGAP"
        elif "DL_DCCH" in line:
            current_protocol = "DL_DCCH"
        elif "UL_DCCH" in line:
            current_protocol = "UL_DCCH"
        elif "DL_CCCH" in line:
            current_protocol = "DL_CCCH"
        elif "UL_CCCH" in line:
            current_protocol = "UL_CCCH"
        elif "RRC" in line:
            current_protocol = "UL_CCCH" # default fallback
        elif "X2AP" in line:
            current_protocol = "X2AP"
        elif "XNAP" in line:
            current_protocol = "XNAP"
            
        # Match hex offsets like "00000010: "
        match = re.search(r'[0-9a-fA-F]{8}:\s+(.*)', line)
        if match:
            hex_payload = match.group(1).replace(" ", "").strip()
            # It might have wrapped to the next line
            if i + 1 < len(lines):
                next_line = lines[i+1].strip()
                if next_line and not re.search(r'[0-9a-fA-F]{8}:', next_line) and all(c in "0123456789abcdefABCDEF " for c in next_line):
                    hex_payload += next_line.replace(" ", "")
            
            current_hex.append((current_protocol or "NGAP", hex_payload))

    # Reassemble blocks that might belong to the same packet
    # (Simplified: we'll just print them out)
    if not current_hex:
        print("No hex dumps found!")
        return

    # To group them, we'd need to look at contiguous sequences, but for now let's just dump the raw strings we found
    # Actually, let's group contiguous lines that share the same protocol
    
    packets = []
    current_packet = ""
    last_proto = None
    
    for proto, hex_str in current_hex:
        if proto != last_proto and current_packet:
            packets.append((last_proto, current_packet))
            current_packet = ""
        current_packet += hex_str
        last_proto = proto
        
    if current_packet:
        packets.append((last_proto, current_packet))

    for idx, (proto, payload) in enumerate(packets):
        print("="*60)
        print(f"Packet #{idx+1} [{proto}]")
        print(f"Payload: {payload[:len(payload)//2 + 1]}... (len: {len(payload)//2} bytes)")
        
        # Call asncodec
        decoder_path = "./build/asncodec"
        try:
            result = subprocess.run([decoder_path, "decode", "--protocol", proto, "--hex", payload], capture_output=True, text=True)
            if "Decode successful" in result.stdout:
                print(result.stdout)
                print("[SUCCESS] Decoded successfully!")
                # Print any validation errors if present
                for rline in result.stdout.split('\n'):
                    if "Validation failed" in rline:
                        print("  ->", rline)
            else:
                print(f"[FAILED] {result.stdout.strip()}")
        except Exception as e:
            print(f"Failed to execute decoder: {e}")
            
if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Parse Hex Dumps from logs")
    parser.add_argument("log_file", help="Path to the log file")
    args = parser.parse_args()
    parse_log(args.log_file)
