// client.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "cJSON.h"
#include "board.h"   // generate_move() 선언

#define BUF_SIZE 4096

// 전역 사용자명 버퍼
char g_username[32];

// recv_json: fd로부터 '\n' 단위로 JSON 문자열 한 줄을 읽어서 동적 할당된 문자열로 반환
static char *recv_json(int fd) {
    char buffer[BUF_SIZE];
    int idx = 0;
    while (1) {
        int n = recv(fd, buffer + idx, 1, 0);
        if (n <= 0) {
            return NULL;  // 오류 또는 연결 종료
        }
        if (buffer[idx] == '\n') {
            buffer[idx] = '\0';
            break;
        }
        idx++;
        if (idx >= BUF_SIZE - 1) {
            return NULL;  // 버퍼 초과
        }
    }
    return strdup(buffer);
}

// send_json: cJSON 객체를 직렬화하여 fd로 전송 (뒤에 '\n' 추가)
static int send_json(int fd, cJSON *obj) {
    char *json_str = cJSON_PrintUnformatted(obj);
    if (!json_str) return -1;
    int len = snprintf(NULL, 0, "%s\n", json_str);
    char *buf = malloc(len + 1);
    if (!buf) {
        free(json_str);
        return -1;
    }
    snprintf(buf, len + 1, "%s\n", json_str);
    int sent = send(fd, buf, strlen(buf), 0);
    free(buf);
    free(json_str);
    return sent;
}

static void print_usage(const char *progname) {
    fprintf(stderr,
            "Usage: %s -ip <server_ip> -port <server_port> -username <your_username>\n"
            "Example:\n"
            "  %s -ip 10.8.128.233 -port 8080 -username Moonyoung\n",
            progname, progname);
}

int main(int argc, char *argv[]) {
    if (argc != 7) {
        print_usage(argv[0]);
        return 1;
    }

    // 플래그 파싱 (-ip, -port, -username)
    char server_ip[64] = {0};
    char server_port[16] = {0};
    char username[32]   = {0};

    for (int i = 1; i < argc; i += 2) {
        if (strcmp(argv[i], "-ip") == 0) {
            strncpy(server_ip, argv[i + 1], sizeof(server_ip) - 1);
        }
        else if (strcmp(argv[i], "-port") == 0) {
            strncpy(server_port, argv[i + 1], sizeof(server_port) - 1);
        }
        else if (strcmp(argv[i], "-username") == 0) {
            strncpy(username, argv[i + 1], sizeof(username) - 1);
        }
        else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (server_ip[0] == '\0' || server_port[0] == '\0' || username[0] == '\0') {
        print_usage(argv[0]);
        return 1;
    }

    // 전역 버퍼에 username 복사
    strncpy(g_username, username, sizeof(g_username) - 1);
    g_username[sizeof(g_username) - 1] = '\0';

    struct addrinfo hints, *res;
    int sockfd, status;

    // 1) 서버 주소 해석
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((status = getaddrinfo(server_ip, server_port, &hints, &res)) != 0) {
        fprintf(stderr, "getaddrinfo 오류: %s\n", gai_strerror(status));
        return 1;
    }

    // 2) 소켓 생성 및 서버 연결
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sockfd == -1) {
        perror("socket 생성 오류");
        freeaddrinfo(res);
        return 1;
    }
    if (connect(sockfd, res->ai_addr, res->ai_addrlen) == -1) {
        perror("connect 실패");
        close(sockfd);
        freeaddrinfo(res);
        return 1;
    }
    freeaddrinfo(res);

    // 3) register 메시지 전송
    cJSON *reg = cJSON_CreateObject();
    cJSON_AddStringToObject(reg, "type", "register");
    cJSON_AddStringToObject(reg, "username", g_username);
    send_json(sockfd, reg);
    cJSON_Delete(reg);
    printf("[클라이언트] register 전송: %s\n", g_username);

    // 4) 서버 메시지 수신 루프
    while (1) {
        char *msg = recv_json(sockfd);
        if (!msg) {
            printf("서버 연결 종료 또는 수신 실패\n");
            break;
        }

        cJSON *root = cJSON_Parse(msg);
        free(msg);
        if (!root) {
            printf("JSON 파싱 실패\n");
            continue;
        }

        cJSON *type = cJSON_GetObjectItem(root, "type");
        if (!cJSON_IsString(type)) {
            cJSON_Delete(root);
            continue;
        }

        //──────────────────────────────────────────────────────────────────────
        // register_ack
        //──────────────────────────────────────────────────────────────────────
        if (strcmp(type->valuestring, "register_ack") == 0) {
            printf("[서버] register_ack 수신\n");
        }
        //──────────────────────────────────────────────────────────────────────
        // game_start
        //──────────────────────────────────────────────────────────────────────
        else if (strcmp(type->valuestring, "game_start") == 0) {
            printf("[서버] game_start 수신\n");
            // 첫 플레이어 정보는 board.c 내부 로직에서 필요 시 사용
        }
        //──────────────────────────────────────────────────────────────────────
        // your_turn
        //──────────────────────────────────────────────────────────────────────
        else if (strcmp(type->valuestring, "your_turn") == 0) {
            printf("[서버] your_turn 수신\n");
            cJSON *board_json = cJSON_GetObjectItem(root, "board");
            cJSON *timeout    = cJSON_GetObjectItem(root, "timeout");

            if (cJSON_IsArray(board_json) && cJSON_IsNumber(timeout)) {
                int sx = 0, sy = 0, tx = 0, ty = 0;

                // generate_move 함수 호출: 보드 상태에서 최적 수 계산
                generate_move(board_json, &sx, &sy, &tx, &ty);

                cJSON *mv = cJSON_CreateObject();
                cJSON_AddStringToObject(mv, "type", "move");
                cJSON_AddStringToObject(mv, "username", g_username);
                cJSON_AddNumberToObject(mv, "sx", sx);
                cJSON_AddNumberToObject(mv, "sy", sy);
                cJSON_AddNumberToObject(mv, "tx", tx);
                cJSON_AddNumberToObject(mv, "ty", ty);
                send_json(sockfd, mv);
                cJSON_Delete(mv);
                printf("[클라이언트] move 전송: (%d,%d) -> (%d,%d)\n", sx, sy, tx, ty);
            }
        }
        //──────────────────────────────────────────────────────────────────────
        // move_ok: 보드 출력
        //──────────────────────────────────────────────────────────────────────
        else if (strcmp(type->valuestring, "move_ok") == 0) {
            printf("[서버] move_ok 수신\n");
            cJSON *board_arr = cJSON_GetObjectItem(root, "board");
            if (cJSON_IsArray(board_arr)) {
                printf("----- 현재 보드 상태 (move_ok) -----\n");
                int rows = cJSON_GetArraySize(board_arr);
                for (int i = 0; i < rows; i++) {
                    cJSON *row = cJSON_GetArrayItem(board_arr, i);
                    if (cJSON_IsString(row)) {
                        printf("%s\n", row->valuestring);
                    }
                }
                printf("-----------------------------------\n");
            }
        }
        //──────────────────────────────────────────────────────────────────────
        // invalid_move
        //──────────────────────────────────────────────────────────────────────
        else if (strcmp(type->valuestring, "invalid_move") == 0) {
            printf("[서버] invalid_move 수신: 잘못된 수\n");
        }
        //──────────────────────────────────────────────────────────────────────
        // pass
        //──────────────────────────────────────────────────────────────────────
        else if (strcmp(type->valuestring, "pass") == 0) {
            cJSON *uname = cJSON_GetObjectItem(root, "username");
            cJSON *nextp = cJSON_GetObjectItem(root, "next_player");
            if (cJSON_IsString(uname) && cJSON_IsString(nextp)) {
                printf("[서버] %s 패스 → 다음 턴: %s\n", uname->valuestring, nextp->valuestring);
            }
        }
        //──────────────────────────────────────────────────────────────────────
        // game_over: 최종 점수 출력 후 종료
        //──────────────────────────────────────────────────────────────────────
        else if (strcmp(type->valuestring, "game_over") == 0) {
            printf("[서버] game_over 수신\n");

            // 1) 보드(JSON 배열) 파싱해서 줄 단위로 출력 (서버에서 board 필드 추가된 경우)
            cJSON *board_arr = cJSON_GetObjectItem(root, "board");
            if (cJSON_IsArray(board_arr)) {
                printf("----- 최종 보드 상태 (game_over) -----\n");
                int rows = cJSON_GetArraySize(board_arr);
                for (int i = 0; i < rows; i++) {
                    cJSON *row = cJSON_GetArrayItem(board_arr, i);
                    if (cJSON_IsString(row)) {
                        printf("%s\n", row->valuestring);
                    }
                }
                printf("------------------------------------\n");
            }

            // 2) 점수 출력
            cJSON *scores = cJSON_GetObjectItem(root, "scores");
            if (scores && cJSON_IsObject(scores)) {
                cJSON *me = cJSON_GetObjectItem(scores, g_username);
                printf("최종 점수 - %s: %d\n", g_username, me ? me->valueint : 0);
                cJSON *tmp = NULL;
                cJSON_ArrayForEach(tmp, scores) {
                    if (strcmp(tmp->string, g_username) != 0) {
                        printf("상대(%s) 점수: %d\n", tmp->string, tmp->valueint);
                    }
                }
            }
            cJSON_Delete(root);
            break;  // game_over이므로 루프 탈출
        }
        //──────────────────────────────────────────────────────────────────────
        // 그 외 알 수 없는 메시지
        //──────────────────────────────────────────────────────────────────────
        else {
            printf("[서버] 알 수 없는 메시지 유형: %s\n", type->valuestring);
        }

        cJSON_Delete(root);
    }

    close(sockfd);
    printf("클라이언트 종료\n");
    return 0;
}
