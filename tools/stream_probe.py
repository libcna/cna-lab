#!/usr/bin/env python3
"""Measure a finite native GPU streaming sweep; collision and audible audio are bypassed."""
import argparse
import json
import math
import os
import pathlib
import re
import statistics
import subprocess
import time


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', default='build-clean/cna_backrooms')
    parser.add_argument('--output', default='build/stream-probe')
    parser.add_argument('--backend', choices=('x11', 'wayland'), default='x11')
    parser.add_argument('--level', type=int, choices=(0, 1, 2), default=0)
    parser.add_argument('--seed', default='12345')
    parser.add_argument('--metres', type=float, default=9600)
    parser.add_argument('--msaa', type=int, choices=(0, 4), default=4)
    parser.add_argument('--loops', type=int, choices=range(1,9), default=1,
                        help='repeat the same out/back route in one process')
    args = parser.parse_args()
    if not math.isfinite(args.metres) or not 400 <= args.metres <= 100000:
        parser.error('distance must be finite and between 400 and 100000 metres')
    root = pathlib.Path(__file__).resolve().parents[1]
    output = (root / args.output).resolve()
    output.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    # One explicitly requested SDL backend prevents a fallback from obscuring
    # which Linux window path ran. Dummy audio is never listening evidence.
    env.update(SDL_VIDEODRIVER=args.backend, SDL_AUDIODRIVER='dummy')
    command = [str((root / args.game).resolve()), '--seed', args.seed,
               '--level', str(args.level), '--msaa', str(args.msaa),
               '--stream-test-metres', str(args.metres),
               '--stream-test-loops', str(args.loops)]
    samples = []
    started = time.monotonic()
    with (output / 'game.log').open('w') as log:
        game = subprocess.Popen(command, cwd=root, env=env, stdout=log, stderr=log)
        try:
            deadline = started + args.metres * args.loops / 45 * 3 + 60
            while game.poll() is None:
                if time.monotonic() > deadline:
                    raise RuntimeError('native sweep did not exit within its time limit')
                try:
                    status = pathlib.Path(f'/proc/{game.pid}/status').read_text()
                    rss = re.search(r'VmRSS:\s+(\d+)', status)
                    if rss:
                        # The game flushes each diagnostic title. Label RSS by
                        # the latest native lap; boundary precision is one second.
                        with (output/'game.log').open('rb') as telemetry:
                            telemetry.seek(0,2)
                            size=telemetry.tell()
                            telemetry.seek(max(0,size-4096))
                            laps=re.findall(rb'\| sweep lap (\d+)/(\d+)',telemetry.read())
                        lap=int(laps[-1][0]) if laps else 0
                        samples.append({'seconds': round(time.monotonic() - started, 2),
                                        'rss_mib': int(rss[1]) / 1024,'lap':lap})
                except FileNotFoundError:
                    pass  # The process can exit between poll and the /proc read.
                time.sleep(1)
            if game.returncode != 0:
                raise RuntimeError(f'native sweep exited with {game.returncode}; see {output}/game.log')
        finally:
            if game.poll() is None:
                game.terminate()
                try:
                    game.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    game.kill()
                    game.wait()
            (output / 'rss.json').write_text(json.dumps(samples, indent=2) + '\n')

    text = (output / 'game.log').read_text()
    if 'Renderer ready: OPENGLES3' not in text:
        raise RuntimeError('the required OPENGLES3 renderer was not initialized')
    profile = re.search(r'\[levels\] format (\d+), world (\d+), source ([0-9a-f]+)', text)
    if not profile:
        raise RuntimeError('missing packaged level/world reproduction record')
    frames = []
    for line in text.splitlines():
        if not line.startswith('cna-backrooms | '):
            continue
        pattern = (r'pos ([\d.-]+),([\d.-]+).*chunk (-?\d+),(-?\d+)'
                   r'.*loaded (\d+)/25.*VBO (\d+)/(\d+) pool (\d+) gpu ([\d.]+) MiB'
                   r'.*build ([\d.]+) ms \| peak ([\d.]+) ms.*\| (\d+) FPS'
                   r'.*draw p95/max ([\d.]+)/([\d.]+) ms \| submit ([\d.]+) ms \| (\d+) UPS')
        match = re.search(pattern, line)
        if not match:
            raise RuntimeError('unrecognized native streaming telemetry: ' + line)
        v = list(map(float, match.groups()))
        lap=re.search(r'\| sweep lap (\d+)/(\d+)',line)
        if lap is None or int(lap[2])!=args.loops:
            raise RuntimeError('missing or mismatched native sweep lap telemetry')
        frames.append(dict(x=v[0], z=v[1], chunk=[int(v[2]), int(v[3])],
                           loaded=int(v[4]), created=int(v[5]), reused=int(v[6]),
                           spares=int(v[7]), packed_mib=v[8], build_ms=v[9],
                           peak_build_ms=v[10], fps=int(v[11]), p95_ms=v[12],
                           max_frame_ms=v[13], submit_ms=v[14], ups=int(v[15]),lap=int(lap[1])))
    if len(frames) < 5 or not samples:
        raise RuntimeError('insufficient native/RSS samples')
    if max(f['loaded'] for f in frames) != 25 or max(f['spares'] for f in frames) > 48:
        raise RuntimeError('streaming active/pool limits are incorrect')
    last = frames[-1]
    # Title telemetry is emitted once per second, so the final sample may
    # precede the exact return by at most one 45-metre diagnostic step.
    if math.hypot(last['x'] - 2.5, last['z'] - 2.5) > 50:
        raise RuntimeError('finite diagnostic did not return near its starting point')
    chunks = {tuple(f['chunk']) for f in frames}
    if not any(x < 0 or z < 0 for x, z in chunks) or not any(x > 0 or z > 0 for x, z in chunks):
        raise RuntimeError('sweep did not exercise both coordinate signs')
    late = [s['rss_mib'] for s in samples[len(samples) * 3 // 4:]]
    lap_summaries=[]
    for lap in range(1,args.loops+1):
        native=[f for f in frames if f['lap']==lap]
        rss=[s['rss_mib'] for s in samples if s['lap']==lap]
        if not native or not rss or math.hypot(native[-1]['x']-2.5,native[-1]['z']-2.5)>50:
            raise RuntimeError('a repeated route did not report a complete native return')
        tail=rss[len(rss)//2:]
        lap_summaries.append(dict(lap=lap,rss_final_mib=rss[-1],
            latter_half_rss_min_mib=min(tail),latter_half_rss_max_mib=max(tail),
            packed_peak_mib=max(f['packed_mib'] for f in native),
            buffers_created=native[-1]['created'],buffers_reused=native[-1]['reused']))
    summary = dict(backend=args.backend, renderer='OPENGLES3', level=args.level,
                   seed=args.seed, metres=args.metres*args.loops,
                   per_loop_metres=args.metres,loops=args.loops,lap_summaries=lap_summaries,
                   msaa=args.msaa,
                   profile_format=int(profile[1]), world_format=int(profile[2]),
                   profile_source=profile[3], seconds=round(time.monotonic() - started, 1),
                   title_samples=len(frames), unique_player_chunks=len(chunks),
                   max_active_chunks=max(f['loaded'] for f in frames),
                   max_spares=max(f['spares'] for f in frames),
                   buffers_created=last['created'], buffers_reused=last['reused'],
                   packed_peak_mib=max(f['packed_mib'] for f in frames),
                   packed_final_mib=last['packed_mib'],
                   rss_peak_mib=max(s['rss_mib'] for s in samples),
                   rss_final_mib=samples[-1]['rss_mib'],
                   final_quarter_rss_min_mib=min(late), final_quarter_rss_max_mib=max(late),
                   fps_median=statistics.median(f['fps'] for f in frames),
                   ups_median=statistics.median(f['ups'] for f in frames),
                   p95_median_ms=statistics.median(f['p95_ms'] for f in frames),
                   max_sampled_frame_ms=max(f['max_frame_ms'] for f in frames),
                   build_median_ms=statistics.median(f['build_ms'] for f in frames),
                   build_peak_ms=max(f['peak_build_ms'] for f in frames),
                   builds_over_16_67_ms=text.count('Chunk build spike '),
                   submit_median_ms=statistics.median(f['submit_ms'] for f in frames),
                   collision_bypassed=True, audio_device='dummy')
    (output / 'frames.json').write_text(json.dumps(frames, indent=2) + '\n')
    (output / 'summary.json').write_text(json.dumps(summary, indent=2) + '\n')
    print(json.dumps(summary, indent=2), flush=True)
    print('Finite observations only. Dummy audio does not validate audibility.', flush=True)


if __name__ == '__main__':
    main()
