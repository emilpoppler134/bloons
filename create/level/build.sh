#!/bin/bash
cc main.c `pkg-config --libs --cflags raylib` -o App