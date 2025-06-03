#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include "cjson/cJSON.h"
#include "game_logic/game_logic.h"

int clientfd1 = -1;
int clientfd2 = -1;
cJSON* username1 = NULL;
cJSON* username2 = NULL;

typedef struct{
    int x;
    int y;
} coordinate;

cJSON* receive_payload(int clientfd, int t)
{
    size_t len = 0;
    char buffer[1024];
    cJSON* payload;

    if(t>0){
        struct timeval timeout;
        timeout.tv_sec = t;
        timeout.tv_usec = 0;
        setsockopt(clientfd, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof timeout);
    }

    len += recv(clientfd, buffer + len, sizeof(buffer) - len - 1, 0);
    buffer[len] ='\0';
    payload = cJSON_Parse(buffer);

    return payload;
}

void send_payload(int clientfd, cJSON* Payload)
{
    const char *json = cJSON_PrintUnformatted(Payload);

    char payload[1024];
    snprintf(payload, sizeof(payload), "%s\n", json);

    send(clientfd, payload, strlen(payload), 0);
}

void send_register_ack(int clientfd)
{
    cJSON* Payload = cJSON_CreateObject();

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("register_ack"));

    send_payload(clientfd, Payload);
}

void send_register_nack(int clientfd)
{
    cJSON* Payload = cJSON_CreateObject();

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("register_nack"));
    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("invalid"));

    send_payload(clientfd, Payload);
}

void* accept_client(void* arg)
{
    int sockfd = *((int*)arg);
    int n_client = 0;
    int clientfd;
    cJSON* payload = NULL;
    cJSON* username = NULL;

    while(1){
        clientfd = accept(sockfd, NULL, NULL);
        if (clientfd == -1) {
            perror("accept clientfd failed");
            exit(1);
        }

        payload = receive_payload(clientfd, -1);
        username = cJSON_GetObjectItemCaseSensitive(payload, "username");

        if(n_client == 0){
            if (username == NULL || username->valuestring == NULL || strlen(username->valuestring) == 0) send_register_nack(clientfd);
            else{
                username1 = cJSON_Duplicate(username, 1);
                clientfd1 = clientfd;
                send_register_ack(clientfd);
                n_client++;
            }
        }
        else if(n_client == 1){
            if (username == NULL || username->valuestring == NULL || strlen(username->valuestring) == 0 || strcmp(username->valuestring, username1->valuestring) == 0) send_register_nack(clientfd);
            else{
                username2 = cJSON_Duplicate(username, 1);
                clientfd2 = clientfd;
                send_register_ack(clientfd);
                n_client++;
            }
        }
        else{
            send_register_nack(clientfd);
        }
    }
}

cJSON* return_board_array(int board[][10])
{
    char line[10];

    cJSON* board_array = cJSON_CreateArray();

    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            line[j-1] = map_int_char(board[i][j]);
        }
        line[8] = '\0';
        cJSON_AddItemToArray(board_array, cJSON_CreateString(line));
    }

    return board_array;
}

void game_start()
{
    cJSON *Payload = cJSON_CreateObject();
    cJSON *players = cJSON_CreateArray();

    cJSON_AddItemToArray(players,cJSON_CreateString(username1->valuestring));
    cJSON_AddItemToArray(players,cJSON_CreateString(username2->valuestring));

    cJSON_AddStringToObject(Payload, "type", "game_start");
    cJSON_AddItemToObject(Payload, "players", players);
    cJSON_AddStringToObject(Payload, "first_player", username1->valuestring);

    send_payload(clientfd1, Payload);
    send_payload(clientfd2, Payload);
}

void send_your_turn(int board[][10], int clientfd)
{
    cJSON* Payload = cJSON_CreateObject();  

    cJSON* board_array = return_board_array(board);

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("your_turn"));
    cJSON_AddItemToObject(Payload, "board", board_array);
    cJSON_AddItemToObject(Payload, "timeout", cJSON_CreateNumber(5.0));

    send_payload(clientfd, Payload);
}

void send_move_ok(int clientfd, int board[][10], cJSON* next_player)
{
    cJSON* Payload = cJSON_CreateObject();

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("move_ok"));
    cJSON_AddItemToObject(Payload, "board", return_board_array(board));
    cJSON_AddItemToObject(Payload, "next_player", next_player);

    send_payload(clientfd, Payload);
}

void send_invalid_move(int clientfd, int not_your_turn, int board[][10], cJSON* next_player)
{
    cJSON* Payload = cJSON_CreateObject();

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("invalid_move"));

    if(not_your_turn) cJSON_AddItemToObject(Payload, "reason", cJSON_CreateString("not your turn"));
    else{
        cJSON_AddItemToObject(Payload, "board", return_board_array(board));
        cJSON_AddItemToObject(Payload, "next_player", next_player);
    }

    send_payload(clientfd, Payload);
}

void send_pass(int clientfd, cJSON* next_player)
{
    cJSON* Payload = cJSON_CreateObject();

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("pass"));
    cJSON_AddItemToObject(Payload, "next_player", next_player);

    send_payload(clientfd, Payload);
}

void game_over(int score1, int score2)
{
    cJSON* Payload = cJSON_CreateObject();
    cJSON* scores = cJSON_CreateObject();

    cJSON_AddItemToObject(scores, username1->valuestring, cJSON_CreateNumber(score1));
    cJSON_AddItemToObject(scores, username2->valuestring, cJSON_CreateNumber(score2));

    cJSON_AddItemToObject(Payload, "type", cJSON_CreateString("game_over"));
    cJSON_AddItemToObject(Payload, "scores", scores);

    send_payload(clientfd1, Payload);
    send_payload(clientfd2, Payload);
}

void game_result(int board[][10])
{
    int score1=0, score2=0;

    for(int i=1;i<=8;i++){
        for(int j=1;j<=8;j++){
            if(board[i][j] == 2) score1++;
            else if(board[i][j] == 3) score2++;
        }
    }

    game_over(score1, score2);
}

int main(int argc, char *argv[]) {
    struct addrinfo hints, *res;
    int sockfd, clientfd, status, bytes_received;
    int count_turn = 0, player;
    char buffer[1024];

    int board[10][10];
    int tx, ty, sx, sy;

    int pass_flag = 0;

    cJSON* payload = NULL;

    pthread_t accept, cli1, cli2;

    // Set up socket parameters
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    
    // Resolve the IP address of the local host using getaddrinfo
    status = getaddrinfo(NULL, "5001", &hints, &res);
    if (status != 0) {
        fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
        exit(1);
    }

    // Create a socket object and bind it to the local address
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket error");
        exit(1);
    }

    int reuse = 1;
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof reuse);

    status = bind(sockfd, res->ai_addr, res->ai_addrlen);
    if (status == -1) {
        perror("bind error");
        exit(1);
    }

    // Listen for incoming connections
    status = listen(sockfd, 1);
    if (status == -1) {
        perror("listen error");
        exit(1);
    }
    
    printf("Listening on port 5001...\n");

    int accept_id = pthread_create(&accept, NULL, accept_client, &sockfd);
    
    while(clientfd1 ==  -1 || clientfd2 == -1){
        sleep(1);
    }

    game_start();
    initialize_board(board);

    while (1) {
        int clientfd;
        cJSON* next_player;
        
        player = 2 + count_turn%2;
        
        if(player == 2) {
            clientfd = clientfd1;
            next_player = username2;
        }
        else if(player == 3) {
            clientfd = clientfd2;
            next_player = username1;
        }
        
        send_your_turn(board, clientfd);
        
        payload = receive_payload(clientfd, -1);

        if(payload == NULL) {
            send_pass(clientfd, next_player);
            count_turn++;
            continue;
        }
        
        sx = cJSON_GetObjectItemCaseSensitive(payload, "sx")->valueint;
        sy = cJSON_GetObjectItemCaseSensitive(payload, "sy")->valueint;
        tx = cJSON_GetObjectItemCaseSensitive(payload, "tx")->valueint;
        ty = cJSON_GetObjectItemCaseSensitive(payload, "ty")->valueint;
        
        int dir = direction(sx, sy, tx, ty);

        if(sx==0 && sy==0 && tx==0 && ty==0){
            if(can_do_pass(player, board)){
                if(pass_flag){
                    send_move_ok(clientfd, board, next_player);
                    break;
                }
                else{
                    send_move_ok(clientfd, board, next_player);
                    pass_flag = 1;
                    count_turn++;
                    continue;
                }
            }
            else{
                send_invalid_move(clientfd, 0, board, next_player);
                pass_flag = 0;
                count_turn++;
                continue;
            }
        }
        else pass_flag = 0;
        
        if(sx<0 || sy<0 || tx<0 || ty<0 || board[tx][ty]!=0 || board[sx][sy]!=player || dir==-1) {
            send_invalid_move(clientfd, 0, board, next_player);
            count_turn++;
            continue;
        }

        else if (strcmp(cJSON_GetObjectItemCaseSensitive(payload, "username")->valuestring, next_player->valuestring) == 0) {
            send_invalid_move(clientfd, 1, board, next_player);
            count_turn++;
            continue;
        }

        else move(sx, sy, tx, ty, dir, board);
        
        count_turn++;
        if(terminate(board)) break;
    }

    game_result(board);

    // Free the address information
    freeaddrinfo(res);

    return 0;
}