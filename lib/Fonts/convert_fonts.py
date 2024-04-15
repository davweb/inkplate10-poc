"""Convert TTF fonts to C++ header file."""

from pathlib import Path
import subprocess
from urllib import request

FONTS = [
    ['Roboto', 'Regular', [16], 'https://github.com/googlefonts/roboto/raw/main/src/hinted/Roboto-Regular.ttf'],
    ['Roboto', 'Bold', [16, 24], 'https://github.com/googlefonts/roboto/raw/main/src/hinted/Roboto-Bold.ttf'],
    ['RobotoMono', 'Regular', [20], 'https://github.com/googlefonts/RobotoMono/raw/main/fonts/ttf/RobotoMono-Regular.ttf']
]

OUTPUT_FILE = Path('src/Fonts.h')
RESOURCES_DIR = Path('resources')

OUTPUT_FILE.unlink(missing_ok=True)
RESOURCES_DIR.mkdir(exist_ok=True)

with open(OUTPUT_FILE, 'w', encoding='utf-8') as font_header:
    for typeface, weight, sizes, font_url in FONTS:
        ttf_file = RESOURCES_DIR / f'{typeface}-{weight}.ttf'

        if not ttf_file.exists():
            request.urlretrieve(font_url, ttf_file)

        for size in sizes:
            print(f'Converting {typeface} {weight} {size}...')
            subprocess.run(['fontconvert', ttf_file, str(size)], stdout=font_header, check=True)

print ('Fonts converted.')
