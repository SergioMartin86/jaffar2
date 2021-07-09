#!/bin/bash

source .set_env.sh

#mpirun -n 1 jaffar-train --maxSteps 250 --savFile saves/lvl01.sav scripts/lvl01.jaffar 
mpirun -n 1 jaffar2-train --maxSteps 250 --romFile ../rom/pop2.bin --savFile saves/lvl01.sav scripts/lvl01.jaffar 
