#ifndef __SERVER_H__
#define __SERVER_H__

#include "raylib.h"
#include "entity.h"

typedef enum action_e
{
  ACTION_NULL,
  ACTION_PLACE,
  ACTION_REMOVE,
  ACTION_START_GAME,
} action_e;

typedef struct package_t
{
  action_e action;
  entity_t entity;
  int index;
} package_t;

typedef struct socket_t
{
  int server_socket;
  int client_socket;
} socket_t;

package_t init_package();
void init_create_server();
void init_join_server(char *ip_address);

#endif