#!/bin/zsh

gcc main.c parser.c -o leadme `pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0 appindicator3-0.1`
