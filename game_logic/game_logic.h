#ifndef GAME_LOGIC_H
#define GAME_LOGIC_H

void show_board(int board[][10]);
void initialize_board(int board[][10]);
void where_can_move(int i, int j, int board[][10], int can_move[20]);
int can_do_pass(int player, int board[][10]);
int direction(int r1, int c1, int r2, int c2);
int clone_or_jump(int dir);
void flip(int r, int c, int player, int board[][10]);
void move(int r1, int c1, int r2, int c2, int dir, int board[][10]);
char map_int_char(int n);
int terminate(int board[][10]);

#endif