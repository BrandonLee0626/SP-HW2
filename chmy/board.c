// board.c

#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "board.h"
#include "cJSON.h"

// hzeller 라이브러리 헤더 (rpi-rgb-led-matrix)
#include "led-matrix.h"
#include "graphics.h"

#define SIZE 8

using rgb_matrix::RGBMatrix;
using rgb_matrix::Canvas;

// 전역 LED 매트릭스/캔버스 객체
static RGBMatrix *matrix = nullptr;
static Canvas    *canvas = nullptr;

// 64×64 픽셀 버퍼
static uint32_t led_buffer[64][64];

// 색상 정의 (0xRRGGBB)
#define COLOR_GRID        0x202020
#define COLOR_BG          0x000000
#define COLOR_RED_PIECE   0xFF0000
#define COLOR_BLUE_PIECE  0x0000FF

//=========================
// 1) LED 매트릭스 초기화
//=========================
static void init_led_matrix() {
    if (matrix) return;

    rgb_matrix::RGBMatrix::Options options;
    rgb_matrix::CanvasOptions    canvas_options;

    // 패널 설정 (64×64, 1체인, 1병렬, 하드웨어 매핑)
    options.rows            = 64;
    options.cols            = 64;
    options.chain_length    = 1;
    options.parallel        = 1;
    options.hardware_mapping = "adafruit-hat"; 
    // ("adafruit-hat" 대신 사용 중인 HAT/매핑 이름으로 변경)

    matrix = rgb_matrix::CreateMatrixFromOptions(options, canvas_options);
    if (!matrix) {
        fprintf(stderr, "ERROR: LED matrix 초기화 실패\n");
        exit(1);
    }
    canvas = matrix->CreateFrameCanvas();
    if (!canvas) {
        fprintf(stderr, "ERROR: Canvas 생성 실패\n");
        exit(1);
    }
}

//=========================
// 2) send_led_buffer
//=========================
static void send_led_buffer(void) {
    if (!matrix) {
        init_led_matrix();
    }

    // led_buffer를 Canvas로 복사
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            uint32_t rgb = led_buffer[y][x];
            uint8_t r = (rgb >> 16) & 0xFF;
            uint8_t g = (rgb >>  8) & 0xFF;
            uint8_t b = (rgb      ) & 0xFF;
            canvas->SetPixel(x, y, r, g, b);
        }
    }
    canvas = matrix->SwapOnVSync(canvas);
}

//=========================
// 3) 그리디 알고리즘 부분
//=========================
static void copy_board(char dest[SIZE][SIZE], char src[SIZE][SIZE]) {
    memcpy(dest, src, SIZE * SIZE);
}

static void parse_board(const cJSON *board_json, char board[SIZE][SIZE]) {
    for (int i = 0; i < SIZE; i++) {
        const cJSON *row = cJSON_GetArrayItem(board_json, i);
        const char *str = row->valuestring;
        for (int j = 0; j < SIZE; j++) {
            board[i][j] = str[j];
        }
    }
}

static int is_valid_move(char board[SIZE][SIZE], int r1, int c1, int r2, int c2, char player) {
    if (r1 < 0 || r1 >= SIZE || c1 < 0 || c1 >= SIZE) return 0;
    if (r2 < 0 || r2 >= SIZE || c2 < 0 || c2 >= SIZE) return 0;
    if (board[r1][c1] != player) return 0;
    if (board[r2][c2] != '.') return 0;
    int dr = abs(r2 - r1), dc = abs(c2 - c1);
    int clone = (dr <= 1 && dc <= 1 && !(dr == 0 && dc == 0));
    int jump  = ((dr == 2 && dc == 0) || (dr == 0 && dc == 2) || (dr == 2 && dc == 2));
    return clone || jump;
}

static void apply_move(char board[SIZE][SIZE], int r1, int c1, int r2, int c2, char player) {
    int dr = abs(r2 - r1), dc = abs(c2 - c1);
    if (dr <= 1 && dc <= 1) {
        board[r2][c2] = player;
    } else {
        board[r1][c1] = '.';
        board[r2][c2] = player;
    }
    // flip
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 && dx == 0) continue;
            int nr = r2 + dy, nc = c2 + dx;
            if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE) continue;
            if (board[nr][nc] != player && board[nr][nc] != '.') {
                board[nr][nc] = player;
            }
        }
    }
}

static int count_pieces(char board[SIZE][SIZE], char player) {
    int cnt = 0;
    for (int i = 0; i < SIZE; i++)
        for (int j = 0; j < SIZE; j++)
            if (board[i][j] == player) cnt++;
    return cnt;
}

static int can_opponent_move_to(char board[SIZE][SIZE], int sr, int sc, char player_other) {
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            if (board[r][c] != player_other) continue;
            int dr = abs(sr - r), dc = abs(sc - c);
            if ((dr <= 1 && dc <= 1) && !(dr == 0 && dc == 0)) return 1;
            if (((dr == 2 && dc == 0) || (dr == 0 && dc == 2) || (dr == 2 && dc == 2))) return 1;
        }
    }
    return 0;
}

static int calc_greedy_value(char board[SIZE][SIZE], int r1, int c1, int r2, int c2, char player) {
    char sim[SIZE][SIZE];
    copy_board(sim, board);
    int before = count_pieces(sim, player);
    apply_move(sim, r1, c1, r2, c2, player);
    int after = count_pieces(sim, player);
    return after - before;
}

static int gather_moves(char board[SIZE][SIZE], char player, int moves[][4]) {
    int cnt = 0;
    for (int r1 = 0; r1 < SIZE; r1++) {
        for (int c1 = 0; c1 < SIZE; c1++) {
            if (board[r1][c1] != player) continue;
            for (int dr = -2; dr <= 2; dr++) {
                for (int dc = -2; dc <= 2; dc++) {
                    int r2 = r1 + dr, c2 = c1 + dc;
                    if (is_valid_move(board, r1, c1, r2, c2, player)) {
                        moves[cnt][0] = r1;
                        moves[cnt][1] = c1;
                        moves[cnt][2] = r2;
                        moves[cnt][3] = c2;
                        cnt++;
                    }
                }
            }
        }
    }
    return cnt;
}

static int calc_opponent_max_greedy(char board[SIZE][SIZE], int r1, int c1, int r2, int c2, char player) {
    char sim[SIZE][SIZE];
    copy_board(sim, board);
    apply_move(sim, r1, c1, r2, c2, player);
    char opp = (player == 'R') ? 'B' : 'R';

    int opp_moves[SIZE*SIZE*8][4];
    int opp_cnt = gather_moves(sim, opp, opp_moves);
    int max_gv = 0;
    for (int i = 0; i < opp_cnt; i++) {
        int or1 = opp_moves[i][0], oc1 = opp_moves[i][1];
        int or2 = opp_moves[i][2], oc2 = opp_moves[i][3];
        int gv = calc_greedy_value(sim, or1, oc1, or2, oc2, opp);
        if (gv > max_gv) max_gv = gv;
    }
    return max_gv;
}

static int is_safe_jump(char board[SIZE][SIZE], int r1, int c1, int r2, int c2, char player) {
    int dr = abs(r2 - r1), dc = abs(c2 - c1);
    int jump = ((dr == 2 && dc == 0) || (dr == 0 && dc == 2) || (dr == 2 && dc == 2));
    if (!jump) return 0;
    char sim[SIZE][SIZE];
    copy_board(sim, board);
    sim[r1][c1] = '.';
    char opp = (player == 'R') ? 'B' : 'R';
    if (can_opponent_move_to(sim, r1, c1, opp)) return 0;
    return 1;
}

static int calc_friend_count(char board[SIZE][SIZE], int r2, int c2, char player) {
    int cnt = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dy == 0 && dx == 0) continue;
            int nr = r2 + dy, nc = c2 + dx;
            if (nr < 0 || nr >= SIZE || nc < 0 || nc >= SIZE) continue;
            if (board[nr][nc] == player) cnt++;
        }
    }
    int edge = (r2 == 0 || r2 == SIZE-1) + (c2 == 0 || c2 == SIZE-1);
    if (edge == 2) cnt += 4;
    else if (edge == 1) cnt += 3;
    return cnt;
}

void generate_move(const cJSON *board_json, int *sx, int *sy, int *tx, int *ty) {
    char board[SIZE][SIZE];
    parse_board(board_json, board);

    // 현재 player를 'R'로 가정 (client.c에서 my_color에 맞춰 변경 필요)
    char me = 'R';

    int moves[SIZE*SIZE*8][4];
    int n_moves = gather_moves(board, me, moves);
    if (n_moves == 0) {
        *sx = *sy = *tx = *ty = 0;
        return;
    }

    int bestEval   = -1000000;
    int bestType   =  3; 
    int bestFriend = -1;
    int bestR1=0, bestC1=0, bestR2=0, bestC2=0;

    for (int i = 0; i < n_moves; i++) {
        int r1 = moves[i][0], c1 = moves[i][1];
        int r2 = moves[i][2], c2 = moves[i][3];

        int myGV  = calc_greedy_value(board, r1, c1, r2, c2, me);
        int oppGV = calc_opponent_max_greedy(board, r1, c1, r2, c2, me);
        int eval  = myGV - oppGV;

        int dr = abs(r2 - r1), dc = abs(c2 - c1);
        int jump = ((dr == 2 && dc == 0) || (dr == 0 && dc == 2) || (dr == 2 && dc == 2));
        int safe = is_safe_jump(board, r1, c1, r2, c2, me);
        int type;
        if (jump && safe) type = 0;
        else if (!jump)  type = 1;
        else              type = 2;

        char sim[SIZE][SIZE];
        copy_board(sim, board);
        apply_move(sim, r1, c1, r2, c2, me);
        int friendCnt = calc_friend_count(sim, r2, c2, me);

        int better = 0;
        if (eval > bestEval) better = 1;
        else if (eval == bestEval) {
            if (type < bestType) better = 1;
            else if (type == bestType) {
                if (friendCnt > bestFriend) better = 1;
                else if (friendCnt == bestFriend) {
                    if (r2 < bestR2 || (r2 == bestR2 && c2 < bestC2)) better = 1;
                }
            }
        }

        if (better) {
            bestEval   = eval;
            bestType   = type;
            bestFriend = friendCnt;
            bestR1 = r1;    bestC1 = c1;
            bestR2 = r2;    bestC2 = c2;
        }
    }

    *sx = bestR1 + 1;
    *sy = bestC1 + 1;
    *tx = bestR2 + 1;
    *ty = bestC2 + 1;

    //=========================
    // 4) LED 매트릭스 출력
    //=========================
    // 4-1) 전체 배경 채우기
    for (int y = 0; y < 64; y++) {
        for (int x = 0; x < 64; x++) {
            led_buffer[y][x] = COLOR_BG;
        }
    }
    // 4-2) 격자선 그리기
    for (int i = 0; i <= 8; i++) {
        int offset = i * 8;
        for (int x = 0; x < 64; x++) {
            led_buffer[offset][x] = COLOR_GRID;
        }
        for (int y = 0; y < 64; y++) {
            led_buffer[y][offset] = COLOR_GRID;
        }
    }
    // 4-3) 돌 그리기 (각 셀 내부 6×6)
    for (int r = 0; r < SIZE; r++) {
        for (int c = 0; c < SIZE; c++) {
            char piece = board[r][c];
            if (piece == '.') continue;
            int cell_x = c * 8;
            int cell_y = r * 8;
            uint32_t color = (piece == 'R') ? COLOR_RED_PIECE : COLOR_BLUE_PIECE;
            for (int dy = 1; dy <= 6; dy++) {
                for (int dx = 1; dx <= 6; dx++) {
                    int px = cell_x + dx;
                    int py = cell_y + dy;
                    led_buffer[py][px] = color;
                }
            }
        }
    }
    // 4-4) 매트릭스에 전송
    send_led_buffer();
}
