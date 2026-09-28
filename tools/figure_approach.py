#!/usr/bin/env python3
"""Walk toward and away from harmless figures using the actual FPS controller."""
import argparse
import json
import math
import os
import pathlib
import re
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', default='build/cna_backrooms')
    parser.add_argument('--world-quality', default='build/world_quality')
    parser.add_argument('--output', default='build/figure-approach')
    parser.add_argument('--level', type=int, choices=(0, 1, 2))
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[1]
    output = (root / args.output).resolve()
    output.mkdir(parents=True, exist_ok=True)
    samples = subprocess.check_output(
        [str((root / args.world_quality).resolve()), '--entity-approaches'],
        cwd=root, text=True, stderr=subprocess.STDOUT)
    source = re.search(r'\[levels\] format \d+, world \d+, source ([0-9a-f]+)', samples)
    if source is None:
        raise RuntimeError('the approach audit did not report its profile source')
    pattern = (r'approach seed (\d+) level ([012]) center ([\d.-]+),([\d.-]+) '
               r'phase ([\d.-]+) position ([\d.-]+),([\d.-]+) heading ([\d.-]+)')
    approaches = list(re.finditer(pattern, samples))
    if len(approaches) != 3:
        raise RuntimeError('the audit did not find all three clear approaches')
    env = os.environ.copy()
    env.update(SDL_VIDEODRIVER='x11', SDL_AUDIODRIVER='dummy')
    manifest = []

    def xdo(*arguments):
        return subprocess.check_output(['xdotool', *map(str, arguments)],
                                       text=True, timeout=5).strip()

    for sample in approaches:
        seed, level, cx, cz, phase, sx, sz, heading = sample.groups()
        level = int(level)
        if args.level is not None and level != args.level:
            continue
        cx, cz, sx, sz, heading = map(float, (cx, cz, sx, sz, heading))
        with (output / f'l{level}.log').open('w') as log:
            game = subprocess.Popen(
                [str((root / args.game).resolve()), '--seed', seed,
                 '--level', str(level), '--position', str(sx), str(sz),
                 '--walk-speed', '2.4'], cwd=root, env=env, stdout=log, stderr=log)
            try:
                time.sleep(3)
                startup = (output / f'l{level}.log').read_text()
                actual_source = re.search(
                    r'\[levels\] format \d+, world \d+, source ([0-9a-f]+)', startup)
                if actual_source is None or actual_source[1] != source[1]:
                    raise RuntimeError('game and approach audit profiles differ')
                window = xdo('search', '--pid', game.pid, '--name', 'cna-backrooms').splitlines()[0]

                def state():
                    title = xdo('getwindowname', window)
                    position = re.search(r'pos ([\d.-]+),([\d.-]+)', title)
                    view = re.search(r'\| view ([\d.-]+),([\d.-]+)', title)
                    if position is None or view is None or f'Level {level} ' not in title:
                        raise RuntimeError('invalid figure controller state: ' + title)
                    return float(position[1]), float(position[2]), math.radians(float(view[1])), title

                deadline = time.monotonic() + 15
                while 'loaded 25/25' not in xdo('getwindowname', window):
                    if game.poll() is not None or time.monotonic() > deadline:
                        raise RuntimeError('figure approach did not finish initial streaming')
                    time.sleep(.1)
                px, pz, yaw, title = state()
                turn = (yaw - heading + math.pi) % (2 * math.pi) - math.pi
                xdo('mousemove_relative', '--', round(turn / .0022), 0)
                time.sleep(1.1)
                for index, distance in enumerate((6, 5, 4.2, 3.5, 2.8, 2.1, 1.3, .4, 2.8, 4.2, 6)):
                    if index:
                        px, pz, yaw, title = state()
                        current_distance = ((px - cx) * (sx - cx) + (pz - cz) * (sz - cz)) / 6
                        key = 'w' if distance < current_distance else 's'
                        xdo('keydown', key)
                        time.sleep(abs(distance - current_distance) / 2.4)
                        xdo('keyup', key)
                        time.sleep(1.1)
                    px, pz, yaw, title = state()
                    tx = cx + (sx - cx) * distance / 6
                    tz = cz + (sz - cz) * distance / 6
                    if math.hypot(px - tx, pz - tz) > .25:
                        raise RuntimeError(f'figure approach blocked or drifted: {title}, target {tx},{tz}')
                    error = (yaw - heading + math.pi) % (2 * math.pi) - math.pi
                    if abs(error) > math.radians(.3):
                        raise RuntimeError('figure approach mouse convention mismatch: ' + title)
                    name = f'l{level}-{index:02d}-d{distance:.1f}'
                    subprocess.run(['import', '-window', window, str(output / (name + '.png'))],
                                   check=True, timeout=10)
                    manifest.append(dict(name=name, level=level, seed=seed, center=[cx, cz],
                                         phase=float(phase), target_distance=distance, title=title))
                    (output / 'manifest.json').write_text(json.dumps(manifest, indent=2) + '\n')
                    print(name + ' | ' + title, flush=True)
            finally:
                subprocess.run(['xdotool', 'keyup', 'w', 's'],
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
