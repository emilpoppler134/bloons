#!/bin/bash
cc main.c launcher.c entity.c server.c interval.c `pkg-config --libs --cflags raylib` -lpthread -o Game