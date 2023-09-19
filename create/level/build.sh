#!/bin/bash
cc create_level.c `pkg-config --libs --cflags raylib` -o Create