#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/wait.h>
#define BUFFER_SIZE 100
int interrupted = 0;

void handle_interrupt(int signal) {
    interrupted = 1;
}
void execute_command(char *command) {
    char *args[BUFFER_SIZE];
    int i = 0;

    args[i] = strtok(command, " ");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, " ");
    }
    args[i] = NULL;

    execvp(args[0], args);
    perror("(Error) Execution: ");
}

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

void pwd() {
    char cwd[BUFFER_SIZE];
    getcwd(cwd, sizeof(cwd));
    printf("%s\n", cwd);
}

void rm(char *path) {
    if (remove(path) != 0) {
        perror("(Error) Rm: ");
    }
}

void mv(char *src, char *dest) {
    if (rename(src, dest) != 0) {
        perror("(Error) Mv: ");
    }
}

void cat(char *path) {
    FILE *file = fopen(path, "r");
    if (file == NULL) {
        perror("(Error) Cat: ");
        return;
    }

    char buffer[BUFFER_SIZE];
    while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
        printf("%s", buffer);
    }

    fclose(file);
}

void cp(char *src, char *dest) {
    FILE *source = fopen(src, "rb");
    if (source == NULL) {
        perror("(Error) Cp (Source): ");
        return;
    }

    FILE *destination = fopen(dest, "wb");
    if (destination == NULL) {
        fclose(source);
        perror("(Error) Cp (Destination): ");
        return;
    }

    char buffer[BUFFER_SIZE];
    size_t bytesRead;
    while ((bytesRead = fread(buffer, 1, BUFFER_SIZE, source)) > 0) {
        fwrite(buffer, 1, bytesRead, destination);
    }

    fclose(source);
    fclose(destination);
}

void ln(char *src, char *dest) {
    if (symlink(src, dest) != 0) {
        perror("(Error) Ln: ");
    }
}

void mkdir_func(char *path) {
    if (mkdir(path, 0777) != 0) {
        perror("(Error) Mkdir: ");
    }
}

void rmdir_func(char *path) {
    if (rmdir(path) != 0) {
        perror("(Error) Rmdir: ");
    }
}

void cd(char *path) {
    if (chdir(path) != 0) {
        perror("(Error) Cd: ");
    }
}


void handle_pipes(char *command) {
    char *args[BUFFER_SIZE];
    int i = 0;

    args[i] = strtok(command, "|");
    while (args[i] != NULL) {
        i++;
        args[i] = strtok(NULL, "|");
    }
    args[i] = NULL;

    int num_pipes = i - 1;

    int pipefds[2 * num_pipes];
    for (i = 0; i < num_pipes; i++) {
        if (pipe(pipefds + i * 2) < 0) {
            perror("(Error) Pipe: ");
            exit(EXIT_FAILURE);
        }
    }

    int pid;
    int status;
    int fd_in = 0;

    for (i = 0; i < num_pipes + 1; i++) {
        if ((pid = fork()) < 0) {
            perror("(Error) Fork: ");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            if (i < num_pipes) {
                if (dup2(pipefds[i * 2 + 1], 1) < 0) {
                    perror("(Error) Dup2: ");
                    exit(EXIT_FAILURE);
                }
            }

            if (i != 0) {
                if (dup2(fd_in, 0) < 0) {
                    perror("(Error) Dup2: ");
                    exit(EXIT_FAILURE);
                }
            }

            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipefds[j]);
            }

            execute_command(args[i]);

            exit(EXIT_FAILURE);
        } else {
            waitpid(pid, &status, 0);

            for (int j = 0; j < 2 * num_pipes; j++) {
                close(pipefds[j]);
            }

            fd_in = pipefds[i * 2];
        }
    }
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
	 // 파이프라인 명령어인지 확인
        if (strstr(command, "|") != NULL) {
            handle_pipes(command);  // 파이프라인 처리 함수 호출
            continue;
        }

        // 파이프라인이 아닌 일반 명령어 처리
        int pid = fork();
        if (pid < 0) {
            perror("(Error) Fork: ");
            exit(EXIT_FAILURE);
        } else if (pid == 0) {
            // 자식 프로세스에서 명령어 실행
            execute_command(command);
            exit(EXIT_SUCCESS);
        } else {
            // 부모 프로세스에서 자식 프로세스의 종료를 기다림
            int status;
            waitpid(pid, &status, 0);
        }
        if (strcmp(command, "exit") == 0) {
            break;  // "exit" 입력 시 종료
        }

        if (strcmp(command, "ls") == 0) {
            char *args[] = {"", NULL};  // 첫 번째 인자는 실행 파일명이므로 비워둠
            ls(1, args);  // ls 함수 호출
            continue;
        }

        if (strcmp(command, "pwd") == 0) {
            pwd();  // pwd 함수 호출
            continue;
        }

        if (strncmp(command, "rm ", 3) == 0) {
            char *path = command + 3;  // "rm "을 제외한 경로
            rm(path);  // rm 함수 호출
            continue;
        }

        if (strncmp(command, "mv ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            mv(src, dest);  // mv 함수 호출
            continue;
        }

        if (strncmp(command, "cat ", 4) == 0) {
            char *path = command + 4;  // "cat "을 제외한 경로
            cat(path);  // cat 함수 호출
            continue;
        }

        if (strncmp(command, "cp ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            cp(src, dest);  // cp 함수 호출
            continue;
        }

        if (strncmp(command, "ln ", 3) == 0) {
            char *src = strtok(command + 3, " ");
            char *dest = strtok(NULL, " ");
            ln(src, dest);  // ln 함수 호출
            continue;
        }

        if (strncmp(command, "mkdir ", 6) == 0) {
            char *path = command + 6;  // "mkdir "을 제외한 경로
            mkdir_func(path);  // mkdir 함수 호출
            continue;
        }

        if (strncmp(command, "rmdir ", 6) == 0) {
            char *path = command + 6;  // "rmdir "을 제외한 경로
            rmdir_func(path);  // rmdir 함수 호출
            continue;
        }

        if (strncmp(command, "cd ", 3) == 0) {
            char *path = command + 3;  // "cd "을 제외한 경로
            cd(path);  // cd 함수 호출
            continue;
        }

        if (strstr(command, "|") != NULL) {
            handle_pipes(command);
            continue;
        }

        // 경로를 포함한 명령어 생성
        printf(fullCommand, BUFFER_SIZE + 20, "%s/%s", cwd, command);

        // 입력받은 명령어 실행
        system(fullCommand);
}
	return 0;
}

