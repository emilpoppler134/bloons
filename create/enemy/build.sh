#!/bin/bash
cc ../../entity.c ./main.c `pkg-config --libs --cflags raylib` -o App