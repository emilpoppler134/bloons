#include <stdio.h>
#include <stdlib.h>

#include "../../entity.h"

#define TILE_SIZE 80
#define PLAYER_FILE_PATH "../../resources/players/player_1"

void serialize_player(const char *path, entity_t *player)
{
	FILE *file = fopen(path, "wb");
	if (!file)
	{
		perror("Failed to open player file");
		exit(1);
	}

	if (fwrite(player, sizeof(entity_t), 1, file) != 1)
	{
		perror("Failed to write player data");
		fclose(file);
		exit(1);
	}

	fclose(file);
}

void initialize_player(entity_t *player)
{
  player->texture_index = 0;
  player->cost = 100;
  player->damage = 50;
	player->radius = 300;
}

int main()
{
	entity_t player;
	initialize_player(&player);
	serialize_player(PLAYER_FILE_PATH, &player);

	return 0;
}