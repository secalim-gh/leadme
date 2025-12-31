#!/bin/sh

set -xe

if pkg-config --exists gtk-layer-shell-0; then
	LIBS=$(pkg-config --cflags --libs gtk+-3.0 gtk-layer-shell-0)
	FLAGS="-DUSE_LAYER_SHELL"
else
	LIBS=$(pkg-config --cflags --libs gtk+-3.0)
	FLAGS=""
fi

gcc $FLAGS main.c parser.c -lm -o leadme $LIBS
