#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>

#define BUFFER_SIZE 100

void ls(int narg, char **argv) {
    char temp[256];
    if (narg == 1) {
        getcwd(temp, 256);
        printf("%s\n", temp);
        argv[1] = temp;
    }
    
    DIR *pdir;
    struct dirent *pde;
    int i = 0;
    if ((pdir = opendir(argv[1])) == NULL) {
        perror("(Error) Opendir: ");
        return;
    }
    printf("\n");
    while ((pde = readdir(pdir)) != NULL) {
        printf("%-20s", pde->d_name);
        if (++i % 3 == 0)
            printf("\n");
    }
    printf("\n");
    closedir(pdir);
}

int main() {
    char command[BUFFER_SIZE];
    char fullCommand[BUFFER_SIZE + 20];  // 경로 포함한 명령어 저장할 변수

    while (1) {
        char cwd[BUFFER_SIZE];
        getcwd(cwd, sizeof(cwd));  // 현재 작업 디렉토리 경로 얻기

        printf("%s $ ", cwd);  // 현재 작업 디렉토리 경로와 함께 프롬프트 출력
        fgets(command, BUFFER_SIZE, stdin);  // 사용자 입력 받기

        // 개행 문자 제거
        command[strcspn(command, "\n")] = '\0';

        if (strcmp(command, "exit") == 0) {
            break;  // "exit" 입력 시 종료
        }

        if (strcmp(command, "ls") == 0) {
            char *args[] = {"", NULL};  // 첫 번째 인자는 실행 파일명이므로 비워둠
            ls(1, args);  // ls 함수 호출
            continue;
        }

        // 경로를 포함한 명령어 생성
        printf(fullCommand, BUFFER_SIZE + 20, "%s/%s", cwd, command);

        // 입력받은 명령어 실행
        system(fullCommand);
    }

    return 0;
}

