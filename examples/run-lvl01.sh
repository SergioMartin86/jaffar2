#!/bin/bash

source .set_env.sh

OMP_NUM_THREADS=8 jaffar2-train --maxSteps 1500 --romFile ../rom/pop2.bin --savFile saves/lvl01_b.state scripts/lvl01.jaffar 
