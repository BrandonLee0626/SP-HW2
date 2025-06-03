#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include "led-matrix.h"     // rpi-rgb-led-matrix의 led-matrix.h 헤더 파일
#include "graphics.h"       // rpi-rgb-led-matrix의 graphics.h 헤더 파일

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
// 3) 보드 상태를 LED 매트릭스에 그리기
//=========================
void draw_board_on_led(char board[SIZE][SIZE]) {
    // 64x64 매트릭스에 맞게 보드 상태를 그리기
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
