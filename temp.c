#include <stdio.h>
#include <stdlib.h>
#include<unistd.h>
#include<string.h>
#include "board.h"

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

int winner(int board[][10]) // Draw -> 1, Red -> 2, Blue -> 3
{
    int R=0, B=0;

    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if (board[i][j] == 2) R++;
            else if (board[i][j] == 3) B++;
        }
    }

    if(R==B) return 1;
    else if(R>B) return 2;
    else return 3;
}

void game_result(int board[][10])
{
    show_board(board);
    switch (winner(board))
    {
    case 1:
        printf("Draw");
        break;

    case 2:
        printf("Red");
        break;
    
    case 3:
        printf("Blue");
        break;
    
    default:
        break;
    }

    exit(0);
}

int main()
{

    RGBLedMatrixOptions options;
    memset(&options, 0, sizeof(options));
    options.rows = 64;
    options.cols = 64;
    options.chain_length = 1;
    options.parallel = 1;
    options.hardware_mapping = "regular";
    options.brightness = 100;
    options.disable_hardware_pulsing = 1;
    options.scan_mode = 1;

    struct RGBLedMatrix *matrix = led_matrix_create_from_options(&options, NULL, NULL);
    if (matrix == NULL) {
        return 1;
    }

    struct LedCanvas *canvas = led_matrix_get_canvas(matrix);

    int board[10][10];
    float N, r1_, c1_, r2_, c2_, temp;
    int pass_flag=0;

    char line[10], input[100];
    
    for(int i=1;i<=8;i++){
        fgets(line, 10, stdin);
        for(int j=0;j<8;j++){
            switch (line[j])
            {
            case '.':
                board[i][j+1] = 0;
                break;

            case '#':
                board[i][j+1] = 1;
                break;

            case 'R':
                board[i][j+1] = 2;
                break;

            case 'B':
                board[i][j+1] = 3;
                break;
            
            default:
                printf("Board input error");
                exit(0);
                break;
            }
        }
    }

    draw_board(canvas, board);

    if (fgets(input, sizeof(input), stdin) != NULL){
        if(input[0] == '\n'){
            printf("Invalid input at turn 0");
            exit(0);
        }
        else{
            if(sscanf(input, "%f", &N) == 1){
                if(N<0 || N!=(int) N){
                    printf("Invalid input at turn 0");
                    exit(0);
                }
            }
            else{
                printf("Invalid input at turn 0");
                exit(0);
            }
        }
    }

    
    for(int i=1;i<N+1;i++){
        led_matrix_delete(matrix);
        int player = 3-i%2;
        
        if (fgets(input, sizeof(input), stdin) != NULL){
            if(input[0] == '\n'){
                printf("Invalid input at turn 0");
                exit(0);
            }
            else{
                if(sscanf(input, "%f %f %f %f %f", &r1_, &c1_, &r2_, &c2_, &temp) == 4){
                    if((r1_ != (int) r1_) || (c1_ != (int) c1_) || (r2_ != (int) r2_) || (c2_ != (int) c2_)) {
                        printf("Invalid input at turn %d", i);
                        exit(0);
                    }
                }
                else{
                    printf("Invalid input at turn %d", i);
                    exit(0);
                }
            }
        } 

        int r1 = (int) r1_;
        int c1 = (int) c1_;
        int r2 = (int) r2_;
        int c2 = (int) c2_;

        int dir = direction(r1, c1, r2, c2);

        if (r1==0 && r2==0 && c1==0 && c2==0){
            if(can_do_pass(player, board)){
                if(pass_flag) game_result(board);
                else pass_flag = 1;
            }
            else{
                printf("Invalid move at turn %d", i);
                exit(0);
            }
        }

        else if(r1<0 || c1<0 || r2<0 || c2<0){
            printf("Invalid input at turn %d", i);
            exit(0);
        }

        else if (board[r2][c2]!=0 || board[r1][c1]!=player || dir==-1){
            printf("Invalid move at turn %d", i);
            exit(0);
        }
            
        else {
            move(r1, c1, r2, c2, dir, board);

            pass_flag = 0;
        }

        draw_board(canvas, board);

        if (terminate(board)) game_result(board);
    }

    game_result(board);
}