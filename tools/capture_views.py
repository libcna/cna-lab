#!/usr/bin/env python3
"""Capture reproducible game views on an X11/Xwayland QA display."""
import argparse
import json
import os
import pathlib
import re
import subprocess
import time

BASE_VIEWS = [
    ('l0_open',12345,0,-22.5,-2.5,0),
    ('l0_columns',12345,0,2.5,27.5,0),
    ('l0_rooms',12345,0,2.5,-22.5,0),
    ('l0_halls',12345,0,32.5,2.5,0),
    ('l0_irregular',12345,0,-22.5,2.5,0),
    ('l0_spawn',12345,0,2.5,2.5,0),
    ('l1_open',12345,1,32.5,-2.5,0),
    ('l1_rooms',12345,1,2.5,-22.5,0),
    ('l1_storage',12345,1,2.5,27.5,0),
    ('l2_halls',12345,2,2.5,-22.5,0),
    ('l2_irregular',12345,2,-22.5,2.5,0),
    ('l2_tunnels',12345,2,2.5,27.5,0),
]
DISTANT_VIEWS = [
    ('l0_open_0',0,0,477.5,227.5,0),
    ('l0_rooms_0',0,0,-242.5,-417.5,-280),
    ('l0_irregular_0',0,0,372.5,-367.5,210),
    ('l0_open_1',1,0,297.5,392.5,0),
    ('l0_rooms_1',1,0,-277.5,-417.5,240),
    ('l0_irregular_1',1,0,222.5,-457.5,-190),
    ('l0_open_31337',31337,0,362.5,457.5,0),
    ('l0_irregular_31337',31337,0,232.5,-277.5,-270),
    ('l1_storage_0',0,1,-452.5,407.5,0),
    ('l1_open_0',0,1,-482.5,-417.5,-220),
    ('l1_halls_0',0,1,227.5,462.5,240),
    ('l1_storage_1',1,1,387.5,-412.5,0),
    ('l2_tunnel_0',0,2,-277.5,362.5,0),
    ('l2_irregular_0',0,2,272.5,-272.5,-230),
    ('l2_chamber_0',0,2,-347.5,-497.5,170),
    ('l2_tunnel_1',1,2,-482.5,502.5,0),
    ('l2_irregular_1',1,2,472.5,-437.5,-220),
    ('l2_chamber_1',1,2,-237.5,-392.5,130),
]

def main():
    parser=argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--game',default='build/cna_backrooms')
    parser.add_argument('--output',default='build/visual-qa')
    parser.add_argument('--msaa',type=int,choices=(0,4),
                        help='override native multisampling for a matched comparison')
    parser.add_argument('--distant',action='store_true')
    parser.add_argument('--partitions',action='store_true',
                        help='sample all six sparse office partition plans')
    parser.add_argument('--world-quality',default='build/world_quality')
    parser.add_argument('--all-directions',action='store_true',
                        help='also capture right, back and left views at each location')
    args=parser.parse_args()
    root=pathlib.Path(__file__).resolve().parents[1]
    if args.partitions and args.distant:
        parser.error('choose either distant or partition views')
    views=DISTANT_VIEWS if args.distant else BASE_VIEWS
    profile_source=None
    if args.partitions:
        quality=subprocess.check_output([str((root/args.world_quality).resolve()),
                                         '--partitions'],cwd=root,text=True,
                                         stderr=subprocess.STDOUT)
        source=re.search(r'\[levels\] format \d+, world \d+, source ([0-9a-f]+)',quality)
        profile_source=source[1] if source else '0' if 'using built-in defaults' in quality else None
        if profile_source is None:
            raise RuntimeError('world quality did not report its level-profile source')
        selected={}
        pattern=r'partition seed 12345 region (-?\d+),(-?\d+) style (\d+) along_x [01] position ([\d.-]+),([\d.-]+)'
        for match in re.finditer(pattern,quality):
            rx,rz,style,x,z=match.groups()
            selected.setdefault(int(style),(f'l0_partition_{style}',12345,0,
                                             float(x),float(z),357))
        if len(selected)!=6:
            raise RuntimeError('world quality did not find all six partition plans')
        views=[selected[style] for style in sorted(selected)]
    output=(root/args.output).resolve()
    output.mkdir(parents=True,exist_ok=True)
    env=os.environ.copy()
    env.update(SDL_VIDEODRIVER='x11',SDL_AUDIODRIVER='dummy')
    manifest=[]
    for name,seed,level,x,z,turn in views:
        with (output/(name+'.log')).open('w') as log:
            game_arguments=[str((root/args.game).resolve()),'--seed',str(seed),
                '--level',str(level),'--position',str(x),str(z)]
            if args.msaa is not None:
                game_arguments+=['--msaa',str(args.msaa)]
            game=subprocess.Popen(game_arguments,
                cwd=root,env=env,stdout=log,stderr=log)
            try:
                time.sleep(3)
                if game.poll() is not None:
                    raise RuntimeError(f'{name}: game failed to launch; inspect its log')
                if profile_source is not None:
                    startup=(output/(name+'.log')).read_text()
                    source=re.search(r'\[levels\] format \d+, world \d+, source ([0-9a-f]+)',startup)
                    actual=source[1] if source else '0' if 'using built-in defaults' in startup else None
                    if actual!=profile_source:
                        raise RuntimeError('world quality and game use different level definitions')
                def xdo(*arguments):
                    return subprocess.check_output(['xdotool',*map(str,arguments)],
                        text=True,timeout=5).strip()
                window=xdo('search','--pid',game.pid,'--name','cna-backrooms').splitlines()[0]
                if turn:
                    xdo('mousemove_relative','--',turn,0)
                    time.sleep(.2)
                suffixes=['','_right','_back','_left'] if args.all_directions else ['']
                for direction,suffix in enumerate(suffixes):
                    if direction:
                        xdo('mousemove_relative','--',714,0)
                        time.sleep(.2)
                    title=xdo('getwindowname',window)
                    if args.partitions:
                        actual=re.search(r'pos ([\d.-]+),([\d.-]+)',title)
                        if actual is None or abs(float(actual[1])-x)>0.15 or abs(float(actual[2])-z)>0.15:
                            raise RuntimeError('partition view reset from the requested position: '+title)
                    view=name+suffix
                    subprocess.run(['import','-window',window,str(output/(view+'.png'))],
                        check=True,timeout=10)
                    manifest.append(dict(name=view,seed=seed,level=level,x=x,z=z,
                                         relative_mouse_x=turn+direction*714,title=title))
                    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
                    print(view+' | '+title,flush=True)
            finally:
                if game.poll() is None:
                    game.terminate()
                try:game.wait(timeout=3)
                except subprocess.TimeoutExpired:game.kill();game.wait()

if __name__=='__main__':
    main()
