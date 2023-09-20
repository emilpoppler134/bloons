#!/bin/bash
cc create_enemy.c `pkg-config --libs --cflags raylib` -o Create