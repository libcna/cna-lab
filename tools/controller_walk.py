#!/usr/bin/env python3
import json, math, os, pathlib, re, subprocess, time
# A normal-controller regression route for world format 22, seed 12345.
# It deliberately crosses negative chunk coordinates and returns over the path.
import argparse
parser=argparse.ArgumentParser(description="Walk a 354 m collision route and capture streaming views.")
parser.add_argument('--game',default='build/cna_backrooms')
parser.add_argument('--output',default='build/controller-qa')
parser.add_argument('--route-file',help='JSON produced by world_route; followed outward and back')
parser.add_argument('--speed',type=float,default=2.0)
parser.add_argument('--capture-every',type=int,default=3)
args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
output=(root/args.output).resolve()
output.mkdir(parents=True,exist_ok=True)
# Planned with 0.55 m clearance, leaving tolerance around the 0.31 m player.
route=[
    [
        2.5,
        2.5
    ],
    [
        -27.5,
        2.5
    ],
    [
        -27.5,
        5.5
    ],
    [
        -28.5,
        5.5
    ],
    [
        -28.5,
        6.5
    ],
    [
        -44.5,
        6.5
    ],
    [
        -44.5,
        17.5
    ],
    [
        -47.5,
        17.5
    ],
    [
        -47.5,
        61.5
    ],
    [
        -58.5,
        61.5
    ],
    [
        -58.5,
        67.5
    ],
    [
        -63.5,
        67.5
    ],
    [
        -63.5,
        87.5
    ],
    [
        -77.5,
        87.5
    ],
    [
        -77.5,
        75.5
    ]
]
level,seed=0,'12345'
if args.route_file:
    data=json.loads((root/args.route_file).read_text())
    route=data['waypoints'];level=data['level'];seed=data['seed']
    if level not in (0,1,2) or len(route)<2 or any(
        len(point)!=2 or not all(math.isfinite(v) for v in point) for point in route):
        raise ValueError('invalid controller route')
if not 0<args.speed<10 or args.capture_every<1:
    raise ValueError('invalid speed or capture interval')
waypoints=route[1:]+list(reversed(route[:-1]))
env=os.environ.copy()
env.update(SDL_VIDEODRIVER='x11', SDL_AUDIODRIVER='dummy')
log=(output/'game.log').open('w')
game=subprocess.Popen([str((root/args.game).resolve()),'--seed',str(seed),
    '--level',str(level),'--position',str(route[0][0]),str(route[0][1]),
    '--walk-speed',str(args.speed),'--run-speed',str(args.speed*2)],cwd=root,env=env,stdout=log,stderr=log)
def xdo(*args):
    return subprocess.check_output(['xdotool',*map(str,args)],text=True,timeout=5).strip()
try:
    time.sleep(3)
    if args.route_file and 'profile_source' in data:
        startup=(output/'game.log').read_text()
        source=re.search(r'\[levels\] format \d+, world \d+, source ([0-9a-f]+)',startup)
        actual=source[1] if source else '0' if 'using built-in defaults' in startup else None
        if actual!=data['profile_source']:
            raise RuntimeError('route and game use different level definitions; regenerate the route')
    window=xdo('search','--pid',game.pid,'--name','cna-backrooms').splitlines()[0]
    deadline=time.monotonic()+15
    while True:
        title=xdo('getwindowname',window)
        if 'Level '+str(level) in title and 'loaded 25/25' in title:
            break
        if game.poll() is not None or time.monotonic()>deadline:
            raise RuntimeError('initial streaming did not finish: '+title)
        time.sleep(.1)
    yaw=0 if level==2 else math.pi/2
    def position():
        title=xdo('getwindowname',window)
        if 'Level '+str(level) not in title:
            raise RuntimeError('unexpected transition: '+title)
        match=re.search(r'pos ([\d.-]+),([\d.-]+)',title)
        if not match:
            raise RuntimeError('missing position: '+title)
        return float(match[1]),float(match[2]),title
    trace=(output/'trace.txt').open('w')
    start=time.monotonic()
    rss_samples=[];observed_chunks=set();max_loaded=0
    distance_walked=0;last_position=route[0]
    for index,(tx,tz) in enumerate(waypoints):
        px,pz,title=position()
        for attempt in range(4):
            distance=math.hypot(tx-px,tz-pz)
            if distance<=0.15:
                break
            heading=math.atan2(tx-px,tz-pz)
            turn=(yaw-heading+math.pi)%(2*math.pi)-math.pi
            pixels=round(turn/0.0022)
            xdo('mousemove_relative','--',pixels,0)
            yaw-=pixels*0.0022
            time.sleep(0.08)
            xdo('keydown','w')
            time.sleep(distance/args.speed)
            xdo('keyup','w')
            time.sleep(1.08)
            nx,nz,title=position()
            if math.hypot(nx-px,nz-pz)<0.05 and distance>0.2:
                raise RuntimeError(f'blocked at {(nx,nz)} targeting {(tx,tz)}')
            px,pz=nx,nz
        if math.hypot(tx-px,tz-pz)>0.2:
            raise RuntimeError(f'waypoint missed: {(px,pz)} versus {(tx,tz)}')
        status=pathlib.Path(f'/proc/{game.pid}/status').read_text()
        rss=int(re.search(r'VmRSS:\s+(\d+)',status)[1])/1024
        rss_samples.append(rss)
        observed_chunks.add(re.search(r'chunk ([\d-]+,[\d-]+)',title)[1])
        max_loaded=max(max_loaded,int(re.search(r'loaded (\d+)',title)[1]))
        distance_walked+=math.hypot(px-last_position[0],pz-last_position[1])
        last_position=(px,pz)
        line=f'{index+1}/{len(waypoints)} target {tx},{tz} | RSS {rss:.1f} MiB | {title}'
        print(line,flush=True)
        trace.write(line+'\n');trace.flush()
        if index%args.capture_every==0 or index==len(waypoints)-1:
            subprocess.run(['import','-window',window,
                str(output/f'view-{index:02d}.png')],check=True)
        if not args.route_file and index==len(route)-2:
            pixels=round(((yaw+math.pi)%(2*math.pi)-math.pi)/0.0022)
            xdo('mousemove_relative','--',pixels,0)
            yaw-=pixels*0.0022
            time.sleep(0.2)
            subprocess.run(['import','-window',window,
                str(output/'pooled-buffer-regression.png')],check=True)
    elapsed=time.monotonic()-start
    summary=dict(level=level,seed=seed,waypoints=len(waypoints),
        seconds=round(elapsed,1),metres=round(distance_walked,1),
        unique_player_chunks=len(observed_chunks),max_active_chunks=max_loaded,
        rss_peak_mib=max(rss_samples),rss_final_mib=rss_samples[-1],
        warmed_rss_min_mib=min(rss_samples[len(rss_samples)//4:]),
        warmed_rss_max_mib=max(rss_samples[len(rss_samples)//4:]))
    (output/'summary.json').write_text(json.dumps(summary,indent=2)+'\n')
    print(f'Controller traversal completed: {json.dumps(summary)}',flush=True)
finally:
    subprocess.run(['xdotool','keyup','w'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    if game.poll() is None:
        game.terminate()
    try: game.wait(timeout=3)
    except subprocess.TimeoutExpired: game.kill();game.wait()
    log.close()
