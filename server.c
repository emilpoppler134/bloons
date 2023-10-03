#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>

#include "server.h"

package_t init_package()
{
  package_t package = {
    .action = ACTION_NULL,
    .entity = init_entity()
  };
  return package;
}


socket_t init_server()
{
  int server_socket = 0;
  int client_socket = 0;
  char *ip_address = (char *)malloc(16);
  
  if (ip_address == NULL) {
    perror("Error in memory allocation");
    exit(1);
  }

  int choice;
  printf("\nHow do you want to play?\n");
  printf("1. Create party\n");
  printf("2. Join party\n");
  printf("Enter your choice: ");
  scanf("%d", &choice);

  switch (choice) {
    case 1:
    {
      struct sockaddr_in server_addr, client_addr;
      socklen_t addr_size;

      server_socket = socket(AF_INET, SOCK_STREAM, 0);
      if (server_socket < 0)
      {
        perror("Error in socket");
        exit(1);
      }

      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(8080);
      server_addr.sin_addr.s_addr = INADDR_ANY;

      if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
      {
        perror("Error in bind");
        exit(1);
      }

      if (listen(server_socket, 10) == 0) 
      {
        printf("Waiting for players...\n");
      } else {
        perror("Error in listen");
        exit(1);
      }

      addr_size = sizeof(client_addr);
      client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &addr_size);
    } break;

    case 2:
    {
      printf("Ip address: ");
      scanf("%15s", ip_address);

      struct sockaddr_in server_addr;

      client_socket = socket(AF_INET, SOCK_STREAM, 0);
      if (client_socket < 0)
      {
        perror("Error in socket");
        exit(1);
      }

      server_addr.sin_family = AF_INET;
      server_addr.sin_port = htons(8080);
      server_addr.sin_addr.s_addr = inet_addr(ip_address);

      if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
      {
        perror("Error in connect");
        exit(1);
      }
    } break;

    default:
    {
      printf("Invalid choice\n");
    } break;
  }

  free(ip_address);

  return (socket_t){.server_socket = server_socket, .client_socket = client_socket};
}