#!/usr/bin/env python3
"""Capture reproducible game views on an X11/Xwayland QA display."""
import argparse
import json
import math
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
    parser.add_argument('--level',type=int,choices=(0,1,2),
                        help='limit a visual pass to one environment family')
    parser.add_argument('--pitch',type=float,default=0,
                        help='camera pitch in degrees, positive looks up (within +/-75)')
    parser.add_argument('--distant',action='store_true')
    parser.add_argument('--partitions',action='store_true',
                        help='sample all six sparse office partition plans')
    parser.add_argument('--entities',action='store_true',
                        help='sample harmless figures at near, mid and distant ranges')
    parser.add_argument('--sampled',action='store_true',
                        help='sample uncurated off-grid positions across new seeds')
    parser.add_argument('--office-lighting',action='store_true',
                        help='sample empty halls, broad rooms, weak circuits and enclosed offices')
    parser.add_argument('--wall-ends',action='store_true',
                        help='inspect exposed ends and outer corners in all three families')
    parser.add_argument('--world-quality',default='build/world_quality')
    parser.add_argument('--all-directions',action='store_true',
                        help='also capture right, back and left views at each location')
    args=parser.parse_args()
    if not math.isfinite(args.pitch) or abs(args.pitch)>75:
        parser.error('pitch must be finite and within +/-75 degrees')
    pitch_pixels=round(-math.radians(args.pitch)/0.0022)
    root=pathlib.Path(__file__).resolve().parents[1]
    if sum((args.partitions,args.distant,args.entities,args.sampled,args.office_lighting,args.wall_ends))>1:
        parser.error('choose one visual sample set')
    views=DISTANT_VIEWS if args.distant else BASE_VIEWS
    profile_source=None
    if args.partitions or args.entities or args.sampled or args.office_lighting or args.wall_ends:
        quality=subprocess.check_output([str((root/args.world_quality).resolve()),
                                         '--wall-ends' if args.wall_ends else
                                         '--office-lighting' if args.office_lighting else
                                         '--views' if args.sampled else
                                         '--entities' if args.entities else '--partitions'],cwd=root,text=True,
                                         stderr=subprocess.STDOUT)
        source=re.search(r'\[levels\] format \d+, world \d+, source ([0-9a-f]+)',quality)
        profile_source=source[1] if source else '0' if 'using built-in defaults' in quality else None
        if profile_source is None:
            raise RuntimeError('world quality did not report its level-profile source')
        if args.sampled or args.office_lighting or args.wall_ends:
            pattern=r'view seed (\d+) level ([012]) sample (\d+) region \d+ position ([\d.-]+),([\d.-]+) heading ([\d.-]+)'
            views=[]
            for match in re.finditer(pattern,quality):
                seed,level,sample,x,z,heading=match.groups()
                initial=0 if int(level)==2 else math.pi/2
                turn=(initial-float(heading)+math.pi)%(2*math.pi)-math.pi
                views.append((f'l{level}_sample_{sample}',int(seed),int(level),
                              float(x),float(z),round(turn/0.0022)))
            if len(views)!=(18 if args.wall_ends else 12):
                raise RuntimeError('world quality did not find all requested view samples')
        elif args.entities:
            pattern=r'entity seed (0|12345) level ([012]) band ([012]) cell -?\d+,-?\d+ position ([\d.-]+),([\d.-]+) heading ([\d.-]+)'
            views=[]
            for match in re.finditer(pattern,quality):
                seed,level,band,x,z,heading=match.groups()
                initial=0 if int(level)==2 else math.pi/2
                turn=(initial-float(heading)+math.pi)%(2*math.pi)-math.pi
                views.append((f'l{level}_entity_s{seed}_b{band}',int(seed),int(level),
                              float(x),float(z),round(turn/0.0022)))
            if len(views)!=18:
                raise RuntimeError('world quality did not find all entity distance samples')
        else:
            selected={}
            pattern=r'partition seed 12345 region (-?\d+),(-?\d+) style (\d+) along_x [01] position ([\d.-]+),([\d.-]+)'
            for match in re.finditer(pattern,quality):
                rx,rz,style,x,z=match.groups()
                selected.setdefault(int(style),(f'l0_partition_{style}',12345,0,
                                                 float(x),float(z),357))
            if len(selected)!=6:
                raise RuntimeError('world quality did not find all six partition plans')
            views=[selected[style] for style in sorted(selected)]
    if args.level is not None:
        views=[view for view in views if view[2]==args.level]
        if not views:
            parser.error('the selected view set has no locations in this level')
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
                deadline=time.monotonic()+15
                while True:
                    title=xdo('getwindowname',window)
                    actual=re.search(r'pos ([\d.-]+),([\d.-]+)',title)
                    if actual is not None and 'loaded 25/25' in title:
                        if abs(float(actual[1])-x)>0.15 or abs(float(actual[2])-z)>0.15:
                            raise RuntimeError('view reset from the requested position: '+title)
                        break
                    if game.poll() is not None or time.monotonic()>deadline:
                        raise RuntimeError('view did not finish streaming: '+title)
                    time.sleep(.1)
                if turn or pitch_pixels:
                    xdo('mousemove_relative','--',turn,pitch_pixels)
                    time.sleep(.2)
                suffixes=['','_right','_back','_left'] if args.all_directions else ['']
                for direction,suffix in enumerate(suffixes):
                    if direction:
                        xdo('mousemove_relative','--',714,0)
                        time.sleep(.2)
                    title=xdo('getwindowname',window)
                    expected_yaw=(0 if level==2 else math.pi/2)-(turn+direction*714)*0.0022
                    deadline=time.monotonic()+2.5
                    while True:
                        view_angle=re.search(r'\| view ([\d.-]+),([\d.-]+)',title)
                        if view_angle is None: break # older comparison binaries lack angle telemetry
                        error=(float(view_angle[1])-math.degrees(expected_yaw)+180)%360-180
                        pitch_error=float(view_angle[2])-math.degrees(-pitch_pixels*0.0022)
                        if abs(error)<0.3 and abs(pitch_error)<0.3: break
                        if time.monotonic()>deadline:
                            raise RuntimeError('relative mouse did not reach the requested view: '+title)
                        time.sleep(.1)
                        title=xdo('getwindowname',window)
                    view=name+suffix
                    subprocess.run(['import','-window',window,str(output/(view+'.png'))],
                        check=True,timeout=10)
                    manifest.append(dict(name=view,seed=seed,level=level,x=x,z=z,
                                         relative_mouse_x=turn+direction*714,
                                         relative_mouse_y=pitch_pixels,title=title))
                    (output/'manifest.json').write_text(json.dumps(manifest,indent=2)+'\n')
                    print(view+' | '+title,flush=True)
            finally:
                if game.poll() is None:
                    game.terminate()
                try:game.wait(timeout=3)
                except subprocess.TimeoutExpired:game.kill();game.wait()

if __name__=='__main__':
    main()
