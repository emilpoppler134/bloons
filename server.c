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