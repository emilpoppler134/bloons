#!/bin/bash
cc *.c `pkg-config --libs --cflags raylib` -o Game