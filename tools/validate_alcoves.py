#!/usr/bin/env python3
"""Walk into office alcoves, test both wall directions, then walk out."""
import argparse
import json
import os
import pathlib
import re
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', default='build/cna_backrooms')
    parser.add_argument('--world-quality', default='build/world_quality')
    parser.add_argument('--output', default='build/alcove-controller-qa')
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[1]
    output = (root / args.output).resolve()
    output.mkdir(parents=True, exist_ok=True)
    quality = subprocess.run([str((root / args.world_quality).resolve()), '--alcoves'],
                             capture_output=True, text=True, check=True)
    source = re.search(r'source ([0-9a-f]+)', quality.stderr)
    pattern = (r'alcove seed 12345 cell (-?\d+),(-?\d+) vertical (\d) inward (-?\d) '
               r'door (\d) boundary ([\d.-]+) span ([\d.-]+),([\d.-]+) depth ([\d.]+)')
    cases = {}
    for match in re.finditer(pattern, quality.stdout):
        cx, cz, vertical, inward, door = map(int, match.group(1, 2, 3, 4, 5))
        key = (vertical, inward)
        if key not in cases:
            cases[key] = (cx, cz, door, *map(float, match.group(6, 7, 8, 9)))
    if len(cases) != 4:
        raise RuntimeError('world_quality did not report all four office alcove directions')
    env = os.environ.copy()
    env.update(SDL_VIDEODRIVER='x11', SDL_AUDIODRIVER='dummy')
    results = []
    for (vertical, inward), (cx, cz, door, boundary, start, end, depth) in cases.items():
        name = f'alcove_{vertical}_{inward}'
        center = (start + end) / 2
        cross = boundary + inward * 3.3
        x, z = (cross, center) if vertical else (center, cross)
        turn = 1428 if vertical and inward == 1 else 0 if vertical else -714 if inward == 1 else 714
        with (output / (name + '.log')).open('w') as log:
            game = subprocess.Popen([str((root / args.game).resolve()), '--seed', '12345',
                                     '--position', str(x), str(z)],
                                    cwd=root, env=env, stdout=log, stderr=log)
            def xdo(*arguments):
                return subprocess.check_output(['xdotool', *map(str, arguments)],
                                               text=True, timeout=5).strip()
            try:
                time.sleep(3)
                if game.poll() is not None:
                    raise RuntimeError(name + ': game failed to launch')
                if source and source[1] not in (output / (name + '.log')).read_text():
                    raise RuntimeError('world_quality and game use different profiles')
                window = xdo('search', '--pid', game.pid, '--name', 'cna-backrooms').splitlines()[0]
                xdo('mousemove_relative', '--', turn, 0)
                time.sleep(.2)
                def move(key, seconds):
                    xdo('keydown', key)
                    time.sleep(seconds)
                    xdo('keyup', key)
                    time.sleep(1.1)
                    title = xdo('getwindowname', window)
                    position = re.search(r'pos ([\d.-]+),([\d.-]+)', title)
                    if not position or 'Level 0 ' not in title:
                        raise RuntimeError('missing office position: ' + title)
                    px, pz = map(float, position.group(1, 2))
                    return ((px if vertical else pz) - boundary) * inward, (pz if vertical else px), title
                distance, along, title = move('w', 1.8)
                if not .32 < distance < .52 or abs(along-center) > .15:
                    raise RuntimeError('alcove back-wall collision failed: ' + title)
                subprocess.run(['import', '-window', window, str(output / (name + '-inside.png'))],
                               check=True, timeout=10)
                distance, along, title = move('d', 1.2)
                if not .32 < distance < .52 or min(abs(along-(start+.41)), abs(along-(end-.41))) > .16:
                    raise RuntimeError('alcove side-wall collision failed: ' + title)
                distance, along, title = move('s', 1.4)
                if distance < depth + .6:
                    raise RuntimeError('player could not leave alcove: ' + title)
                subprocess.run(['import', '-window', window, str(output / (name + '-outside.png'))],
                               check=True, timeout=10)
                results.append(dict(cell=[cx, cz], vertical=vertical, inward=inward,
                                    false_door=bool(door), passed=True, title=title))
                (output / 'results.json').write_text(json.dumps(results, indent=2) + '\n')
                print(name + ' back/side/exit passed | ' + title, flush=True)
            finally:
                subprocess.run(['xdotool', 'keyup', 'w', 'd', 's'],
                               stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                if game.poll() is None:
                    game.terminate()
                try:
                    game.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    game.kill()
                    game.wait()


if __name__ == '__main__':
    main()
