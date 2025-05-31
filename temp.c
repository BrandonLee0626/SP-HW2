#include <string.h>
#include <stdio.h>
#include "cjson/cJSON.h"

void game_start(char *username1, char *username2)
{
    cJSON *Payload = cJSON_CreateObject();
    cJSON *players = cJSON_CreateArray();

    cJSON_AddItemToArray(players,cJSON_CreateString(username1));
    cJSON_AddItemToArray(players,cJSON_CreateString(username2));

    cJSON_AddStringToObject(Payload, "type", "game_start");
    cJSON_AddItemToObject(Payload, "players", players);
    cJSON_AddStringToObject(Payload, "first_player", username1);

    printf("%s\n", cJSON_Print(Payload));
}

int main()
{
    char username1[10] = "Alice";
    char username2[10] = "Bob";

    game_start(username1, username2);
}