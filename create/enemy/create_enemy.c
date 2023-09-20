#include <stdio.h>
#include <stdlib.h>

#include "../../entity.h"

#define TILE_SIZE 80
#define ENEMY_FILE_PATH "../../resources/enemies/enemy_4"

void serialize_enemy(const char *path, entity_t *enemy)
{
	FILE *file = fopen(path, "wb");
	if (!file)
	{
		perror("Failed to open enemy file");
		exit(1);
	}

	if (fwrite(enemy, sizeof(entity_t), 1, file) != 1)
	{
		perror("Failed to write enemy data");
		fclose(file);
		exit(1);
	}

	fclose(file);
}

void initialize_enemy(entity_t *enemy)
{
  enemy->texture_index = 3;
	enemy->speed = 300.0f;
  enemy->hp = 250;
}

int main()
{
	entity_t enemy = init_entity();
	initialize_enemy(&enemy);
	serialize_enemy(ENEMY_FILE_PATH, &enemy);

	return 0;
}