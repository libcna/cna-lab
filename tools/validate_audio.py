#!/usr/bin/env python3
"""Capture this game's normal PipeWire/PulseAudio stream without changing device settings."""
import argparse
import array
import json
import math
import os
import pathlib
import re
import subprocess
import time
import wave


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game', default='build-clean/cna_backrooms')
    parser.add_argument('--output', default='build/audio-qa')
    parser.add_argument('--pulse-server', help='real user audio socket when the private display changes XDG_RUNTIME_DIR')
    parser.add_argument('--isolated-transition', action='store_true',
                        help='start just outside the entrance and trigger the cue without a footstep')
    parser.add_argument('--driver', help='optional SDL audio driver; default uses normal selection')
    parser.add_argument('--creature-kind', type=int, choices=(0, 1, 2),
                        help='capture one harmless creature voice without walking')
    parser.add_argument('--world-quality', default='build-clean/world_quality')
    args = parser.parse_args()
    root = pathlib.Path(__file__).resolve().parents[1]
    if args.creature_kind is not None and args.isolated_transition:
        parser.error('choose a creature or transition event')
    output = (root / args.output).resolve()
    output.mkdir(parents=True, exist_ok=True)
    env = os.environ.copy()
    env['SDL_VIDEODRIVER'] = 'x11'
    env.pop('SDL_AUDIODRIVER', None)
    if args.pulse_server:
        env['PULSE_SERVER'] = args.pulse_server
    if args.driver:
        env['SDL_AUDIODRIVER'] = args.driver

    def command(*arguments):
        return subprocess.check_output(arguments, text=True, env=env, timeout=5).strip()

    capture = None
    with (output / 'game.log').open('w') as log:
        arguments = [str((root / args.game).resolve()), '--seed', '12345']
        creature=None
        if args.creature_kind is not None:
            quality=subprocess.check_output([str((root/args.world_quality).resolve()),
                                             '--creature-approaches'],cwd=root,text=True)
            pattern=(r'approach seed 12345 level 0 center [\d.-]+,[\d.-]+ '
                     r'phase [\d.-]+ position ([\d.-]+),([\d.-]+) heading ([\d.-]+) kind '+
                     str(args.creature_kind))
            creature=re.search(pattern,quality)
            if creature is None:
                raise RuntimeError('no clear office sample for the creature kind')
            arguments+=['--position',creature[1],creature[2]]
        if args.isolated_transition:
            arguments += ['--position', '18.36', '2.5']
        game = subprocess.Popen(arguments,
                                cwd=root, env=env, stdout=log, stderr=log)
        try:
            deadline = time.monotonic() + 8
            stream = None
            while time.monotonic() < deadline:
                if game.poll() is not None:
                    raise RuntimeError('game exited; inspect game.log')
                streams = json.loads(command('pactl', '-f', 'json', 'list', 'sink-inputs'))
                stream = next((s for s in streams if s.get('properties', {}).get(
                    'application.process.id') == str(game.pid)), None)
                if stream:
                    break
                time.sleep(.25)
            if stream is None:
                raise RuntimeError('game has no normal PulseAudio/PipeWire stream')
            sinks = json.loads(command('pactl', '-f', 'json', 'list', 'sinks'))
            sink = next(s for s in sinks if s['index'] == stream['sink'])
            routing = {'stream_index': stream['index'], 'stream_muted': stream['mute'],
                       'stream_corked': stream['corked'], 'stream_volume': stream['volume'],
                       'sink': sink['name'], 'sink_muted': sink['mute'], 'sink_volume': sink['volume']}
            (output / 'routing.json').write_text(json.dumps(routing, indent=2) + '\n')
            window = command('xdotool', 'search', '--pid', str(game.pid), '--name',
                             'cna-backrooms').splitlines()[0]
            time.sleep(2)
            if creature:
                turn=(math.pi/2-float(creature[3])+math.pi)%(2*math.pi)-math.pi
                command('xdotool','mousemove_relative','--',str(round(turn/.0022)),'0')
            pcm = output / 'game.wav'
            with (output / 'capture.log').open('w') as capture_log:
                capture = subprocess.Popen(['parec', '--device=' + str(sink.get('monitor_source_name') or sink['monitor_source']),
                    '--monitor-stream=' + str(stream['index']), '--rate=44100', '--channels=2',
                    '--format=s16le', '--file-format=wav', str(pcm)],
                    env=env, stdout=capture_log, stderr=capture_log)
                time.sleep(2)
                if creature:
                    time.sleep(6)
                else:
                    command('xdotool', 'keydown', 'w')
                    time.sleep(.15 if args.isolated_transition else 7.2)
                    command('xdotool', 'keyup', 'w')
                    time.sleep(3 if args.isolated_transition else 2)
                title = command('xdotool', 'getwindowname', window)
                capture.terminate()
                capture.wait(timeout=3)
                capture = None
            with wave.open(str(pcm), 'rb') as wav:
                if wav.getsampwidth() != 2 or wav.getnchannels() != 2:
                    raise RuntimeError('unexpected captured PCM format')
                rate = wav.getframerate()
                samples = array.array('h', wav.readframes(wav.getnframes()))
            def levels(start, end):
                values = samples[int(start * rate * 2):int(end * rate * 2)]
                if not values:
                    raise RuntimeError('audio capture is empty or too short')
                rms = math.sqrt(sum(v * v for v in values) / len(values)) / 32768
                peak = max(abs(v) for v in values) / 32768
                return {'rms_dbfs': 20 * math.log10(max(rms, 1e-10)),
                        'peak_dbfs': 20 * math.log10(max(peak, 1e-10))}
            event_range = (1.9, 7.5) if creature else (1.9, 4.5) if args.isolated_transition else (8, 10.5)
            result = {'routing': routing, 'hum': levels(.5, 1.5),
                      'walking': None if args.isolated_transition or creature else levels(2.5, 7),
                      'transition_window': None if creature else levels(*event_range),
                      'creature_window': levels(*event_range) if creature else None,
                      'creature_kind': args.creature_kind, 'final_title': title,
                      'subjective_listening': 'Not performed by the agent.'}
            (output / 'summary.json').write_text(json.dumps(result, indent=2) + '\n')
            print(json.dumps(result, indent=2), flush=True)
            if stream['mute'] or stream['corked'] or sink['mute']:
                raise RuntimeError('game stream or its physical output is muted/paused')
            if result['hum']['rms_dbfs'] < -65:
                raise RuntimeError('hum is technically silent')
            event = result['creature_window'] if creature else result['transition_window'] if args.isolated_transition else result['walking']
            if event['peak_dbfs'] < result['hum']['peak_dbfs'] + 6:
                raise RuntimeError('event sound did not rise above the hum')
            if max(abs(v) for v in samples) >= 32767:
                raise RuntimeError('captured audio clips')
            if 'Audio: transition cue could not acquire' in (output / 'game.log').read_text():
                raise RuntimeError('transition audio failed to acquire a voice')
            if creature:
                name=('wanderer','watcher','crawler')[args.creature_kind]
                if '[entity-audio] '+name not in (output/'game.log').read_text():
                    raise RuntimeError('the requested creature did not emit a voice')
                if 'Level 0 ' not in title:
                    raise RuntimeError('stationary creature capture changed level')
            elif 'Level 1 ' not in title:
                raise RuntimeError('walk did not reach the transition')
            print('Own-stream event and routing checked; listening remains subjective.')
        finally:
            subprocess.run(['xdotool', 'keyup', 'w'], check=False, capture_output=True)
            if capture is not None:
                capture.terminate()
                capture.wait(timeout=3)
            if game.poll() is None:
                game.terminate()
            game.wait(timeout=3)


if __name__ == '__main__':
    main()
