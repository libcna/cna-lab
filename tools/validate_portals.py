#!/usr/bin/env python3
"""Exercise maintenance entrances through real X11 mouse/keyboard input."""
import argparse
import os
import pathlib
import re
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', default='build/cna_backrooms')
    parser.add_argument('--output', default='build/portal-controller-qa')
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[1]
    output = (root / args.output).resolve()
    output.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env.update(SDL_VIDEODRIVER='x11', SDL_AUDIODRIVER='dummy')
    # Default walking speed; the held keys span several input frames.
    cases = [
        ('zero_to_one', 0, 14.5, 2.5, 0, 1),
        ('one_to_zero', 1, 2.5, 14.5, 714, 0),
        ('one_to_two', 1, 14.5, 2.5, 0, 2),
        ('two_to_one', 2, 2.5, 14.5, 0, 1),
        ('outside_gate', 0, 14.5, 3.69, 0, 0),
        ('frame_collision', 0, 14.5, 3.23, 0, 0),
    ]
    for name, level, x, z, turn, expected in cases:
        with (output / (name + '.log')).open('w') as log:
            game = subprocess.Popen([
                str((root / args.game).resolve()), '--seed', '12345',
                '--level', str(level), '--position', str(x), str(z)],
                cwd=root, env=env, stdout=log, stderr=log)
            def xdo(*arguments):
                return subprocess.check_output(['xdotool', *map(str, arguments)],
                                               text=True, timeout=5).strip()
            try:
                time.sleep(3)
                if game.poll() is not None:
                    raise RuntimeError(name + ': game failed to launch')
                window = xdo('search', '--pid', game.pid, '--name',
                             'cna-backrooms').splitlines()[0]
                if turn:
                    xdo('mousemove_relative', '--', turn, 0)
                    time.sleep(.2)
                xdo('keydown', 'w')
                time.sleep(2.1)
                xdo('keyup', 'w')
                time.sleep(1.1)
                title = xdo('getwindowname', window)
                if 'Level ' + str(expected) not in title:
                    raise RuntimeError(name + ': unexpected level: ' + title)
                position = re.search(r'pos ([\d.-]+),([\d.-]+)', title)
                if position is None:
                    raise RuntimeError(name + ': missing position: ' + title)
                px = float(position[1])
                if name == 'outside_gate' and px < 19.2:
                    raise RuntimeError('outside entrance unexpectedly blocked: ' + title)
                if name == 'frame_collision' and not 17.3 < px < 17.8:
                    raise RuntimeError('entrance frame collision failed: ' + title)
                subprocess.run(['import', '-window', window,
                                str(output / (name + '.png'))], check=True, timeout=10)
                print(name + ' passed | ' + title, flush=True)
            finally:
                subprocess.run(['xdotool', 'keyup', 'w'], stdout=subprocess.DEVNULL,
                               stderr=subprocess.DEVNULL)
                if game.poll() is None:
                    game.terminate()
                try:
                    game.wait(timeout=3)
                except subprocess.TimeoutExpired:
                    game.kill()
                    game.wait()


if __name__ == '__main__':
    main()
