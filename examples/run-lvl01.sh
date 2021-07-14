#!/bin/bash

source .set_env.sh

mpirun -n 1 jaffar2-train --maxSteps 1500 --romFile ../rom/pop2.bin --savFile saves/lvl01.state scripts/lvl01.jaffar 
