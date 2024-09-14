/****************
LE2: Introduction to Unnamed Pipes
****************/
#include <unistd.h> // pipe, fork, dup2, execvp, close
using namespace std;

int main () {
    // lists all the files in the root directory in the long format
    char* cmd1[] = {(char*) "ls", (char*) "-al", (char*) "/", nullptr};
    // translates all input from lowercase to uppercase
    char* cmd2[] = {(char*) "tr", (char*) "a-z", (char*) "A-Z", nullptr};

    // TODO: add functionality
    // Create pipe
    int pipefd[2];

    if (pipe(pipefd) == -1) {
        return 0;
    }

    // Create child to run first command
    int pid1 = fork();
    if (pid1 == 0) {
        // In child, redirect output to write end of pipe
        dup2(pipefd[1], STDOUT_FILENO);

        // Close the read end of the pipe on the child side.
        close(pipefd[0]);
        close(pipefd[1]);

        // In child, execute the command
        execvp(cmd1[0], cmd1);
    }

    // Create another child to run second command
    int pid2 = fork();
    if (pid2 == 0) {
        // In child, redirect input to the read end of the pipe
        dup2(pipefd[0], STDIN_FILENO);

        // Close the write end of the pipe on the child side.
        close(pipefd[0]);
        close(pipefd[1]);

        // Execute the second command.
        execvp(cmd2[0], cmd2);
    }

    // Reset the input and output file descriptors of the parent.
    close(pipefd[0]);
    close(pipefd[1]);

    return 0;
}
