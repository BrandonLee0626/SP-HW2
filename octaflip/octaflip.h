#include <stdio.h>
#include <stdlib.h>

void make_board(int board[][10], char board_input[][10]) // R -> 2, B -> 3, . -> 0, # -> 1
{
    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            switch (board_input[i-1][j-1])
            {
            case 'R':
                board[i][j] = 2;
                break;

            case 'B':
                board[i][j] = 3;
                break;

            case '.':
                board[i][j] = 0;
                break;
            
            case '#':
                board[i][j] = 1;
                break;
            
            default:
                break;
            }
        }
    }
}