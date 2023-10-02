#!/bin/bash
cc main.c entity.c server.c interval.c `pkg-config --libs --cflags raylib` -lpthread -o Game