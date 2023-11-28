#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>  // getcwd 함수를 사용하기 위해 추가

#define BUFFER_SIZE 100

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

        // 경로를 포함한 명령어 생성
        printf(fullCommand, BUFFER_SIZE + 20, "%s/%s", cwd, command);

        // 입력받은 명령어 실행
        system(fullCommand);
    }

    return 0;
}

