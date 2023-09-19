#!/bin/bash
cc create_player.c `pkg-config --libs --cflags raylib` -o Create