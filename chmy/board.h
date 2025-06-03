#ifndef BOARD_H
#define BOARD_H

#include "cJSON.h"

void generate_move(const cJSON *board_json, int *sx, int *sy, int *tx, int *ty);

#endif // BOARD_H
