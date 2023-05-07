import json
from PIL import Image

def hextriplet(colortuple):
    return ''.join(f'{i:02x}' for i in colortuple)

user_indecies = [5, 7]

image = Image.open("../icon.png")
rgb = image.convert("RGB")
colors = [hextriplet(rgb.getpixel((35, 0)))] * 8

colors[0] = hextriplet(rgb.getpixel((35, 0)))
colors[1] = hextriplet(rgb.getpixel((0, 0)))
colors[2] = hextriplet(rgb.getpixel((0, 1)))
colors[3] = hextriplet(rgb.getpixel((3, 32)))
colors[4] = hextriplet(rgb.getpixel((0, 12)))
colors[6] = hextriplet(rgb.getpixel((7, 34)))

def find_colors():
    filled = 0
    for x in range(35):
        for y in range(47):
            color = hextriplet(rgb.getpixel((x, y)))
            new_color = True
            for i in range(8):
                if color == colors[i]:
                    new_color = False
                    break
            if new_color:
                colors[user_indecies[filled]] = color
                if filled == 1:
                    return
                filled += 1

find_colors()

json_data = {}
for i in range(len(colors)):
    json_data[colors[i]] = i

print(json_data)

with open("../mesproj.json", "r") as mesproj:
    data = json.load(mesproj)

data["sd_information"]["colormap"] = json_data

with open("../mesproj.json", "w") as mesproj:
    json.dump(data, mesproj, indent=4)
