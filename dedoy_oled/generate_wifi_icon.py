
from PIL import Image, ImageDraw
import math

width = 32
height = 32

# Create black image (0)
img = Image.new('1', (width, height), 0)
draw = ImageDraw.Draw(img)

# Draw WiFi Arcs (White=1)
# Center bottom ish
cx, cy = 16, 28

# Dot
draw.ellipse((cx-2, cy-2, cx+2, cy+2), fill=1)

# Arcs
for r in [8, 16, 24]:
    bbox = (cx-r, cy-r, cx+r, cy+r)
    draw.arc(bbox, start=225, end=315, fill=1, width=3)

# Convert to XBM hex format (LSB first)
# u8g2 drawXBMP expects standard XBM
# XBM stores rows padded to bytes. 32 width = 4 bytes.
pixels = img.load()
hex_output = []

for y in range(height):
    row_bytes = []
    current_byte = 0
    for x in range(width):
        bit = pixels[x, y]
        if bit:
            current_byte |= (1 << (x % 8))
        
        if (x + 1) % 8 == 0:
            row_bytes.append(current_byte)
            current_byte = 0
    hex_output.extend(row_bytes)

# Print as C array
print("const unsigned char icon_wifi_32[] PROGMEM = {")
for i in range(0, len(hex_output), 12):
    chunk = hex_output[i:i+12]
    print("  " + ", ".join(f"0x{b:02X}" for b in chunk) + ",")
print("};")
