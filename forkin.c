#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {

    // Fork 1
    printf("About to fork!\n");
    int x = fork();
    if (x) {
        // Fork 2
        printf("Forking a second time!\n");
        x = fork();
    }

    if (x) {
        // parent
        int status;
        int child = wait(&status);

        printf("A Child with PID %d finished and took %d seconds\n", child, WEXITSTATUS(status));
        printf("Parent finished.\n");

        return 0;
    } else {
        //child 

        srand(time(NULL) + getpid());
        printf("I'm a child! PID: %d\n", getpid());

        int time = rand() % 8 + 2;
        sleep(time);

        printf("Child PID %d woke up.\n", getpid());
        return time;
    }
}
