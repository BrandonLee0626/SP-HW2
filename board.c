#include <string.h>
#include <unistd.h>
#include "led-matrix-c.h"

#define LED_ROW 64
#define LED_COL 64

void draw_box(struct LedCanvas *canvas, int x, int y, uint8_t r, uint8_t g, uint8_t b) {
  int size = 8;

  draw_line(canvas, x, y, x + size - 1, y, r, g, b);
  draw_line(canvas, x, y + size - 1, x + size - 1, y + size - 1, r, g, b);
  draw_line(canvas, x, y, x, y + size - 1, r, g, b);
  draw_line(canvas, x + size - 1, y, x + size - 1, y + size - 1, r, g, b);
}

void draw_board(struct LedCanvas *canvas, int board[][10]) {
    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if(board[i][j] == 2) draw_box(canvas, (i-1)*8, (j-1)*8, 255, 0, 0);
            if(board[i][j] == 3) draw_box(canvas, (i-1)*8, (j-1)*8, 0, 0, 255);
        }
    }
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

    struct RGBLedMatrix *matrix = led_matrix_create_from_options(&options, NULL, NULL);
    if (matrix == NULL) {
        return 1;
    }

    struct LedCanvas *canvas = led_matrix_get_canvas(matrix);

    int board[10][10] = {0};
    board[2][3] = 2; 
    board[5][6] = 3;

    draw_board(canvas, board);

    sleep(10);

    led_matrix_delete(matrix);
    return 0;
}