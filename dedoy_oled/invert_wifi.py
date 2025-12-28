
import re

file_path = "Bitmaps.h"

with open(file_path, "r") as f:
    content = f.read()

# Regex to find the array content
# static const unsigned char epd_bitmap_wifi_symbol[] PROGMEM = { ... };
pattern = r"(static const unsigned char epd_bitmap_wifi_symbol\[\] PROGMEM = \{)(.*?)(\};)"

match = re.search(pattern, content, re.DOTALL)
if match:
    header = match.group(1)
    body = match.group(2)
    footer = match.group(3)
    
    # Process body: find hex numbers and invert
    def invert_hex(m):
        val = int(m.group(0), 16)
        inv = (~val) & 0xFF
        return f"0x{inv:02x}"
    
    new_body = re.sub(r"0x[0-9a-fA-F]{2}", invert_hex, body)
    
    new_content = content[:match.start()] + header + new_body + footer + content[match.end():]
    
    with open(file_path, "w") as f:
        f.write(new_content)
    print("Inverted WiFi symbol bits.")
else:
    print("Could not find WiFi symbol array.")
