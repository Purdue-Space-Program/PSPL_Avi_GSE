#!/bin/bash

# Trap interrupts and kill the whole process group
# "$$" is the script's PID. 
# "-" before the PID tells kill to target the Process Group.
trap "kill 0" EXIT

synnax start -im --no-driver &
python3 mock/command.py &
python3 mock/telem.py &

wait
