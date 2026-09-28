#!/usr/bin/env python3
"""Make labeled QA sheets from one capture manifest or a matched pair (Pillow)."""
import argparse
import json
import pathlib

from PIL import Image, ImageDraw


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('first', type=pathlib.Path, help='capture directory with manifest.json')
    parser.add_argument('second', nargs='?', type=pathlib.Path, help='matched capture directory')
    parser.add_argument('--output', type=pathlib.Path, required=True, help='output filename prefix')
    parser.add_argument('--width', type=int, default=960, help='width of each view')
    parser.add_argument('--rows', type=int, default=3, help='views per sheet')
    args = parser.parse_args()
    if not 240 <= args.width <= 1920 or not 1 <= args.rows <= 6:
        parser.error('width must be 240..1920 and rows 1..6')
    directories = [args.first] + ([args.second] if args.second else [])
    manifests = [json.loads((directory / 'manifest.json').read_text())
                 for directory in directories]
    names = [entry['name'] for entry in manifests[0]]
    if not names or len(set(names)) != len(names):
        parser.error('the first manifest must contain unique views')
    if args.second:
        second = {entry['name']: entry for entry in manifests[1]}
        if set(names) != set(second):
            parser.error('paired manifests must contain the same view names')
        for first in manifests[0]:
            other = second[first['name']]
            keys = ('seed', 'level', 'x', 'z', 'relative_mouse_x', 'relative_mouse_y')
            if any(first.get(key) != other.get(key) for key in keys):
                parser.error('paired views have different camera definitions: ' + first['name'])
    height = args.width * 9 // 16
    row_height = height + 28
    args.output.parent.mkdir(parents=True, exist_ok=True)
    for start in range(0, len(names), args.rows):
        page = names[start:start + args.rows]
        sheet = Image.new('RGB', (args.width * len(directories), row_height * len(page)), '#151515')
        draw = ImageDraw.Draw(sheet)
        for row, name in enumerate(page):
            for column, directory in enumerate(directories):
                x, y = column * args.width, row * row_height
                draw.text((x + 8, y + 8), directory.name + ' / ' + name, fill='#eeeeee')
                with Image.open(directory / (name + '.png')) as source:
                    if source.width * 9 != source.height * 16:
                        parser.error('capture aspect must be 16:9: ' + name)
                    sheet.paste(source.convert('RGB').resize((args.width, height),
                                Image.Resampling.LANCZOS), (x, y + 28))
        destination = args.output.parent / (args.output.name + f'-{start // args.rows}.png')
        sheet.save(destination)
        print(destination)


if __name__ == '__main__':
    main()
