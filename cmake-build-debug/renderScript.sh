#!/bin/bash
# render images for part 4

./pathtracer -r 480 360 -t 8 -m 2 -s 1 -l 1 -f ../docs/images/sphere_rays1.png ../dae/sky/CBbunny.dae

./pathtracer -r 480 360 -t 8 -m 2 -s 2 -l 1 -f ../docs/images/sphere_rays2.png ../dae/sky/CBbunny.dae

./pathtracer -r 480 360 -t 8 -m 2 -s 4 -l 1 -f ../docs/images/sphere_rays4.png ../dae/sky/CBbunny.dae

./pathtracer -r 480 360 -t 8 -m 2 -s 8 -l 1 -f ../docs/images/sphere_rays8.png ../dae/sky/CBbunny.dae

./pathtracer -r 480 360 -t 8 -m 2 -s 16 -l 1 -f ../docs/images/sphere_rays16.png ../dae/sky/CBbunny.dae

./pathtracer -r 480 360 -t 8 -m 2 -s 64 -l 1 -f ../docs/images/sphere_rays64.png ../dae/sky/CBbunny.dae

./pathtracer -r 480 360 -t 8 -m 2 -s 1024 -l 1 -f ../docs/images/sphere_rays1024.png ../dae/sky/CBbunny.dae
