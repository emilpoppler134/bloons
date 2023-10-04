#ifndef __LAUNCHER_H__
#define __LAUNCHER_H__

#include "server.h"
#include "raylib.h"

typedef enum game_mode_e
{
  MODE_NULL,
  MODE_SINGLEPLAYER,
  MODE_MULTIPLAYER,
  MODE_NUM
} game_mode_e;

typedef enum launch_state_e
{
  LAUNCH_STATE_GAME_MODE,
  LAUNCH_STATE_SOCKET_INIT,
  LAUNCH_STATE_SOCKET_CREATE,
  LAUNCH_STATE_SOCKET_JOIN,
  LAUNCH_STATE_NUM,
} launch_state_e;

game_mode_e open_launcher(int *client_socket);

#endif