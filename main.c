#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/inotify.h>

// use inotify to wait for file creation
void wait_for_file_create(char* filepath) {
    int event_fd = inotify_init();
    if (event_fd < 0) {
        perror("fail to create inotify fd");
        exit(1);
    }

    int wd = inotify_add_watch(event_fd, filepath, IN_CREATE);
    if (wd < 0) {
        perror("fail to add inotify watch");
        exit(1);
    }

    char buf[1024];

    while (1) {
        ssize_t len = read(event_fd, buf, sizeof(buf));
        if (len == -1) {
            break;
        }

        ssize_t i = 0;
        while (i < len) {
            struct inotify_event* event = (struct inotify_event*)&buf[i];
            if (event->mask & IN_CREATE) {
                if (event->mask & IN_ISDIR) {
                    printf("the directory %s was created.\n", event->name);
                } else {
                    printf("the file %s was created.\n", event->name);
                }
//                close(wd);
//                close(event_fd);
                return;
            }
            i += sizeof(struct inotify_event) + event->len;
        }
    }

//    close(wd);
//    close(event_fd);
}

int main(int argc, char* argv[]) {
    if(argc < 5) {
        printf("Usage: %s -waitfile <filename> -entrypoint <command>\n", argv[0]);
        exit(1);
    }

    char* wait_file;
    char* entrypoint;

    for(int i = 1; i < argc; i++) {
        if(strcmp(argv[i], "-waitfile") == 0 && i < argc-1) {
            wait_file = argv[++i];
        }
        else if(strcmp(argv[i], "-entrypoint") == 0 && i < argc-1) {
            entrypoint = argv[++i];
        }
    }

    printf("entrypoint: %s\n", entrypoint);

    wait_for_file_create(wait_file);


    // parse args use strtok
    char* token;
    char* command;
    char* args[11]; // max 10 args
    int i = 0;

    token = strtok(entrypoint, " ");
    command = token;
    while (token != NULL) {
        token = strtok(NULL, " ");
        args[i++] = token;
    }
    args[i] = NULL;

    execv(command, args);

    // execve only return if error
    perror("execve failed. ");
    exit(1);
}