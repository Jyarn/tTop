#!/bin/bash
python load.py&
alacritty -e htop -d 10&
alacritty -e ./ttop.out&