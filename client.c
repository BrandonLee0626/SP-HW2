#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "cjson/cJSON.h"
#include <limits.h>

// 구조체: 후보 수를 담을 구조체
typedef struct {
    int src_x, src_y;
    int dst_x, dst_y;
    int is_clone;        // 1: clone, 0: jump
    int is_safe_jump;    // 1: 안전한 점프, 0: 불안전 점프 (clone일 땐 무시)
    int friend_count;    // 이동한 칸 주변 친구 수 + 모서리/꼭짓점 보너스
    int final_value;     // (내 그리디 값) - (상대 최대 그리디 값)
} MoveOption;

typedef struct{
    int x;
    int y;
} coordinate;

#define null_coordinate (coordinate) {-1, -1};
#define pass_coordinate (coordinate) {0, 0};

int is_null_coordinate(coordinate c)
{
    return c.x==-1 && c.y==-1;
}

int map_char_int(char c)
{
    switch (c)
    {
    case '.': return 0;
    case '#': return 1;
    case 'R': return 2;
    case 'B': return 3;
    }

    return -1;
}

cJSON* receive_payload(int sockfd)
{
    size_t len = 0;
    char buffer[1024];
    cJSON* payload;

    len += recv(sockfd, buffer + len, sizeof(buffer) - len - 1, 0);
    buffer[len] ='\0';
    payload = cJSON_Parse(buffer);

    printf("payload:%s\n", payload->valuestring);
    
    return payload;
}

void send_payload(int sockfd, cJSON* Payload)
{
    const char *json = cJSON_PrintUnformatted(Payload);

    char payload[1024];
    snprintf(payload, sizeof(payload), "%s\n", json);

    send(sockfd, payload, strlen(payload), 0);
}

void send_register(char *username, int sockfd)
{
    cJSON *Payload = cJSON_CreateObject();

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("register"));
    cJSON_AddItemToObject(Payload, "username", cJSON_CreateString(username));

    send_payload(sockfd, Payload);
}

void verify_register_ack(int sockfd)
{
    cJSON* payload = receive_payload(sockfd);
    
    if(strcmp(cJSON_GetObjectItemCaseSensitive(payload, "type")->valuestring, "register_ack") != 0) exit(1);
}

void send_move(char *username, coordinate src, coordinate trg, int sockfd)
{
    cJSON *Payload = cJSON_CreateObject();

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("move"));
    cJSON_AddItemToObject(Payload, "username", cJSON_CreateString(username));
    cJSON_AddItemToObject(Payload, "sx", cJSON_CreateNumber(src.x));
    cJSON_AddItemToObject(Payload, "sy", cJSON_CreateNumber(src.y));
    cJSON_AddItemToObject(Payload, "tx", cJSON_CreateNumber(trg.x));
    cJSON_AddItemToObject(Payload, "ty", cJSON_CreateNumber(trg.y));

    send_payload(sockfd, Payload);
}

void where_can_move(int board[][10], int player, coordinate can_move[][10]) // player R: 2, B: 3
{
    for(int i=1;i<=8;i++)
        for(int j=1;j<=8;j++)
            can_move[i][j] = null_coordinate;

    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if(board[i][j]==player){
                coordinate src = (coordinate) {i, j};

                if(i-2>=1 && j-2>=1 && board[i-2][j-2]==0) can_move[i-2][j-2] = src;

                if (i-2>=1 && board[i-2][j]==0) can_move[i-2][j] = src;

                if (i-2>=1 && j+2<=8 && board[i-1][j+2]==0) can_move[i-2][j+2] = src;

                if(i-1>=1 && j-1>=1 && board[i-1][j-1]==0) can_move[i-1][j-1] = src;

                if(i-1>=1 && board[i-1][j]==0) can_move[i-1][j] = src;

                if(i-1>=1 && j+1<=8 && board[i-1][j+1]==0) can_move[i-1][j+1] = src;

                if(j-2>=1 && board[i][j-2]==0) can_move[i][j-2] = src;

                if(j-1>=1 && board[i][j-1]==0) can_move[i][j-1] = src;

                if(j+1<=8 && board[i][j+1]==0) can_move[i][j+1] = src;

                if(j+2<=8 && board[i][j+2]==0) can_move[i][j+2] = src;

                if(i+1<=8 && j-1>=1 && board[i+1][j-1]==0) can_move[i+1][j-1] = src;

                if(i+1<=8 && board[i+1][j]==0) can_move[i+1][j] = src;

                if(i+1<=8 && j+1<=8 && board[i+1][j+1]==0) can_move[i+1][j+1] = src;

                if(i+2<=8 && j-2>=1 && board[i+2][j-2]==0) can_move[i+2][j-2] = src;

                if(i+2<=8 && board[i+2][j]==0) can_move[i+2][j] = src;

                if(i+2<=8 && j+2<=8 && board[i+2][j+2]==0) can_move[i+2][j+2] = src;
            }
        }
    }
}

void move_generate(int board[][10], int player, char *username, int sockfd)
{
    coordinate can_move[10][10];
    coordinate src, trg;

    where_can_move(board, player, can_move);

    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if(!is_null_coordinate(can_move[i][j])){
                src = can_move[i][j];
                trg = (coordinate) {i, j};
                break;
            }
        }
    }

    send_move(username, src, trg, sockfd);
}

void get_board(cJSON* board_array, int board[][10])
{
    for(int i=1;i<=8;i++){
        cJSON* row = cJSON_GetArrayItem(board_array, i-1);
        const char* line = row -> valuestring;
        for(int j=1;j<=8;j++){
            board[i][j] = map_char_int(line[j-1]);
        }
    }
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int sockfd, status, opt, player;
    char buffer[1024];

    int board[10][10];

    size_t len;
    cJSON *payload;

    char *ip = NULL;
    char *port = NULL;
    char *username = NULL;
    
    // Parse command-line arguments
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-ip") == 0 && i + 1 < argc) {
            ip = argv[i + 1];
            i++; // Skip the next argument since we've used it
        } else if (strcmp(argv[i], "-port") == 0 && i + 1 < argc) {
            port = argv[i + 1];
            i++; // Skip the next argument
        } else if (strcmp(argv[i], "-username") == 0 && i + 1 < argc) {
            username = argv[i + 1];
            i++; // Skip the next argument
        }
    }
    
    // Check if all required arguments were provided
    if (ip == NULL || port == NULL || username == NULL) {
        fprintf(stderr, "Usage: %s -ip <IP_ADDRESS> -port <PORT> -username <USERNAME>\n", argv[0]);
        exit(1);
    }

    // Set up socket parameters
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    // Resolve the IP address of the remote server using getaddrinfo
    status = getaddrinfo(ip, port, &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // Create a socket object and connect to the remote server
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket error");
        exit(1);
    }

    status = connect(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        perror("connect error");
        exit(1);
    }

    printf("++++++++++\n");
    // Send a register to the server
    send_register(username, sockfd);

    printf("------\n");

    verify_register_ack(sockfd);

    printf("=======\n");

    // Receive game_start from the server
    payload = receive_payload(sockfd);

    printf("//////////\n");

    if (strcmp(username, cJSON_GetObjectItemCaseSensitive(payload, "first_player")->valuestring) == 0)
        player = 2;
    else
        player = 3;

    while (1)
    {
        payload = receive_payload(sockfd);

        if (strcmp(cJSON_GetObjectItemCaseSensitive(payload, "type")->valuestring, "game_over") == 0) break;

        cJSON* board_array = cJSON_GetObjectItemCaseSensitive(payload, "board");
        get_board(board_array, board);

        move_generate(board, player, username, sockfd);
    }

    // Close the socket connection
    close(sockfd);

    return 0;
}

