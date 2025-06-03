#include "../cjson/cJSON.h"
#include <stdio.h>

void show_board(int board[][10])
{
    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            switch (board[i][j])
            {
            case 0:
                printf(".");
                break;

            case 1:
                printf("#");
                break;
            
            case 2:
                printf("R");
                break;

            case 3:
                printf("B");
            
            default:
                break;
            }
        }
        printf("\n");
    }
}

void where_can_move(int i, int j, int board[][10], int can_move[20])
{
    for(int idx=0;idx<20;idx++) can_move[idx]=0;

    if(i-2>=1 && j-2>=1 && board[i-2][j-2]==0) can_move[1] = 1;
    else can_move[1] = 0;

    if (i-2>=1 && board[i-2][j]==0) can_move[2] = 1;
    else can_move[2] = 0;

    if (i-2>=1 && j+2<=8 && board[i-1][j+2]==0) can_move[3] = 1;
    else can_move[3] = 0;

    if(i-1>=1 && j-1>=1 && board[i-1][j-1]==0) can_move[4] = 1;
    else can_move[4] = 0;

    if(i-1>=1 && board[i-1][j]==0) can_move[5] = 1;
    else can_move[5] = 0;

    if(i-1>=1 && j+1<=8 && board[i-1][j+1]==0) can_move[6] = 1;
    else can_move[6] = 0;

    if(j-2>=1 && board[i][j-2]==0) can_move[7] = 1;
    else can_move[7] = 0;

    if(j-1>=1 && board[i][j-1]==0) can_move[8] = 1;
    else can_move[8] = 0;

    if(j+1<=8 && board[i][j+1]==0) can_move[9] = 1;
    else can_move[9] = 0;

    if(j+2<=8 && board[i][j+2]==0) can_move[10] = 1;
    else can_move[10] = 0;

    if(i+1<=8 && j-1>=1 && board[i+1][j-1]==0) can_move[11] = 1;
    else can_move[11] = 0;

    if(i+1<=8 && board[i+1][j]==0) can_move[12] = 1;
    else can_move[12] = 0;

    if(i+1<=8 && j+1<=8 && board[i+1][j+1]==0) can_move[13] = 1;
    else can_move[13] = 0;

    if(i+2<=8 && j-2>=1 && board[i+2][j-2]==0) can_move[14] = 1;
    else can_move[14] = 0;

    if(i+2<=8 && board[i+2][j]==0) can_move[15] = 1;
    else can_move[15] = 0;

    if(i+2<=8 && j+2<=8 && board[i+2][j+2]==0) can_move[16] = 1;
    else can_move[16] = 0;

}

int can_do_pass(int player, int board[][10]) // can't pass -> 0, can pass -> 1
{
    int can_move[20];

    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if(board[i][j] == player){
                where_can_move(i, j, board, can_move);
                for(int k=1;k<=16;k++){
                    if(can_move[k] == 1) return 0;
                }
            }
        }
    }

    return 1;
}

char map_int_char(int n)
{
    switch (n)
    {
    case 0: return '.';
    case 1: return '#';
    case 2: return 'R';
    case 3: return 'B';
    }

    return '\0';
}

void initialize_board(int board[][10])
{
    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if((i==1&&j==1) || (i==8&&j==8)) board[i][j] = 2;
            else if((i==1&&j==8) || (i==8&&j==1)) board[i][j] = 3;
            else board[i][j] = 0;
        }
    }
}

int direction(int r1, int c1, int r2, int c2)
{
    int dir;

    switch (r2-r1)
    {
    case -2:
        switch (c2-c1)
        {
        case -2:
            dir = 1;
            break;
        
        case 0:
            dir = 7;
            break;
        
        case 2:
            dir = 14;
            break;
        
        default:
            dir = -1;
            break;
        }
        break;

    case -1:
        switch (c2-c1)
        {
        case -1:
            dir = 4;
            break;

        case 0:
            dir = 8;
            break;
        
        case 11:
            dir = 11;
            break;
        
        default:
            dir = -1;
            break;
        }
        break;
    
    case 0:
        switch (c2-c1)
        {
        case -2:
            dir = 2;
            break;

        case -1:
            dir = 5;
            break;

        case 1:
            dir = 12;
            break;

        case 2:
            dir = 15;
            break;
        
        default:
            dir = -1;
            break;
        }
        break;

    case 1:
        switch (c2-c1)
        {
        case -1:
            dir = 6;
            break;

        case 0:
            dir = 9;
            break;

        case 1:
            dir = 13;
            break;
        
        default:
            dir = -1;
            break;
        }
        break;

    case 2:
        switch (c2-c1)
        {
        case -2:
            dir = 3;
            break;

        case 0:
            dir = 10;
            break;
        
        case 2:
            dir = 16;
            break;

        default:
            dir = -1;
            break;
        }
        break;
    
    default:
        dir = -1;
        break;
    }

    return dir;
}

int clone_or_jump(int dir) // clone -> 1, jump -> 2
{
    if(dir==1 || dir==2 || dir==3 || dir==7 || dir==10 || dir==14 || dir==15 || dir==16) return 2;
    else return 1;
}

void flip(int r, int c, int player, int board[][10])
{
    int opposite;
    if (player==2) opposite = 3;
    else opposite = 2;

    if(r-1>=1 && c-1>=1) if (board[r-1][c-1] == opposite) board[r-1][c-1] = player;
    if(r-1>=1) if (board[r-1][c] == opposite) board[r-1][c] = player;
    if(r-1>=1 && c+1<=8) if (board[r-1][c+1] == opposite) board[r-1][c+1] = player;
    if(c-1>=1) if (board[r][c-1] == opposite) board[r][c-1] = player;
    if(c+1<=8) if (board[r][c+1] == opposite) board[r][c+1] = player;
    if(r+1<=8 && c-1>=1) if (board[r+1][c-1] == opposite) board[r+1][c-1] = player;
    if(r+1<=8) if (board[r+1][c] == opposite) board[r+1][c] = player;
    if(r+1<=8 && c+1<=8) if (board[r+1][c+1] == opposite) board[r+1][c+1] = player;
}

void move(int r1, int c1, int r2, int c2, int dir, int board[10][10])
{
    if (clone_or_jump(dir) == 1) board[r2][c2] = board[r1][c1];
    if (clone_or_jump(dir) == 2){
        board[r2][c2] = board[r1][c1];
        board[r1][c1] = 0;
    }
    flip(r2, c2, board[r2][c2], board);
}

int terminate(int board[][10])
{
    int empty=0, R=0, B=0;

    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if (board[i][j] == 2) R++;
            else if (board[i][j] == 3) B++;
            else if (board[i][j] == 0) empty++;
        }
    }

    if(empty==0 || R==0 || B==0) return 1;
    else return 0;
}
