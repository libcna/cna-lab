#!/usr/bin/env python3
import math, os, pathlib, re, subprocess, time
# A normal-controller regression route for world format 12, seed 12345.
# It deliberately crosses negative chunk coordinates and returns over the path.
import argparse
parser=argparse.ArgumentParser(description="Walk a 326 m collision route and capture streaming views.")
parser.add_argument('--game',default='build/cna_backrooms')
parser.add_argument('--output',default='build/controller-qa')
args=parser.parse_args()
root=pathlib.Path(__file__).resolve().parents[1]
output=(root/args.output).resolve()
output.mkdir(parents=True,exist_ok=True)
# Planned with 0.55 m clearance, leaving tolerance around the 0.31 m player.
route=[
    [2.5, 2.5], [-27.5, 2.5], [-27.5, 5.5], [-28.5, 5.5],
    [-28.5, 6.5], [-44.5, 6.5], [-44.5, 17.5], [-47.5, 17.5],
    [-47.5, 61.5], [-58.5, 61.5], [-58.5, 67.5], [-71.5, 67.5],
    [-71.5, 68.5], [-73.5, 68.5], [-73.5, 72.5], [-81.5, 72.5],
    [-81.5, 76.5], [-77.5, 76.5], [-77.5, 75.5]
]
waypoints=route[1:]+list(reversed(route[:-1]))
env=os.environ.copy()
env.update(SDL_VIDEODRIVER='x11', SDL_AUDIODRIVER='dummy')
log=(output/'game.log').open('w')
game=subprocess.Popen([str((root/args.game).resolve()),'--seed','12345',
    '--walk-speed','2','--run-speed','4'],cwd=root,env=env,stdout=log,stderr=log)
def xdo(*args):
    return subprocess.check_output(['xdotool',*map(str,args)],text=True,timeout=5).strip()
try:
    time.sleep(3)
    window=xdo('search','--name','cna-backrooms').splitlines()[0]
    yaw=math.pi/2
    def position():
        title=xdo('getwindowname',window)
        if 'Level 0' not in title:
            raise RuntimeError('unexpected transition: '+title)
        match=re.search(r'pos ([\d.-]+),([\d.-]+)',title)
        if not match:
            raise RuntimeError('missing position: '+title)
        return float(match[1]),float(match[2]),title
    trace=(output/'trace.txt').open('w')
    start=time.monotonic()
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
            time.sleep(distance/2.0)
            xdo('keyup','w')
            time.sleep(1.08)
            nx,nz,title=position()
            if math.hypot(nx-px,nz-pz)<0.05 and distance>0.2:
                raise RuntimeError(f'blocked at {(nx,nz)} targeting {(tx,tz)}')
            px,pz=nx,nz
        if math.hypot(tx-px,tz-pz)>0.2:
            raise RuntimeError(f'waypoint missed: {(px,pz)} versus {(tx,tz)}')
        line=f'{index+1}/{len(waypoints)} target {tx},{tz} | {title}'
        print(line,flush=True)
        trace.write(line+'\n');trace.flush()
        if index%3==0 or index==len(waypoints)-1:
            subprocess.run(['import','-window',window,
                str(output/f'view-{index:02d}.png')],check=True)
        if index==len(route)-2:
            pixels=round(((yaw+math.pi)%(2*math.pi)-math.pi)/0.0022)
            xdo('mousemove_relative','--',pixels,0)
            yaw-=pixels*0.0022
            time.sleep(0.2)
            subprocess.run(['import','-window',window,
                str(output/'pooled-buffer-regression.png')],check=True)
    print(f'Controller traversal completed in {time.monotonic()-start:.1f} seconds',flush=True)
finally:
    subprocess.run(['xdotool','keyup','w'],stdout=subprocess.DEVNULL,stderr=subprocess.DEVNULL)
    if game.poll() is None:
        game.terminate()
    try: game.wait(timeout=3)
    except subprocess.TimeoutExpired: game.kill();game.wait()
    log.close()
