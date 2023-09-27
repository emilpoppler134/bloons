#!/bin/bash
cc ../../entity.c ../../interval.c ./main.c `pkg-config --libs --cflags raylib` -o App