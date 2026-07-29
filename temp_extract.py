import re

def extract():
    with open("Trial-UE-Attach.log", "r") as f:
        log = f.read()
    
    matches = re.finditer(r"NGAP Msg (UL|DL).*?00000000:\s+(.*?)\s+88 00000001", log, re.DOTALL)
    for i, match in enumerate(matches):
        hex_block = match.group(2)
        # Strip everything that isn't a hex char
        hex_str = re.sub(r'[^0-9a-fA-F]', '', hex_block)
        print(f"Packet {i+1} [NGAP]: {hex_str}")
        if i == 2: break

if __name__ == "__main__":
    extract()
