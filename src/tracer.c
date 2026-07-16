#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <signal.h>

typedef struct {
    pid_t target_pid;
    int status;
}TracerContext;


void handle_error(const char* msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}      


void run_child_process(char* commands, char** args) {
    if (ptrace(PTRACE_TRACEME, 0, NULL, NULL) == -1) {
        handle_error("ptrace");
    }

    //kill(getpid(), SIGSTOP); // Stop the child process to allow the parent to trace it

    execvp(commands, args);     
    handle_error("execvp");
}

void trace_target(TracerContext* ctx) {
    int status;
    waitpid(ctx->target_pid, &status, 0);

    if(!WIFSTOPPED(status)) {
        fprintf(stderr, "Child process did not stop as expected.\n");
        exit(EXIT_FAILURE);
    }

    printf("Child process %d stopped, tracing...\n", ctx->target_pid);


    ptrace(PTRACE_CONT, ctx->target_pid, NULL, NULL);

    waitpid(ctx->target_pid, &status, 0);

    
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <program_to_trace>\n", argv[0]);
        return EXIT_FAILURE;
    }
    pid_t spawned_pid = fork();

    if (spawned_pid < 0) {
        handle_error("fork");
        
    } else if (spawned_pid == 0) {
        // Child process
        run_child_process(argv[1], &argv[1]);
    } else {
        // Parent process
        TracerContext context = { .target_pid = spawned_pid, .status = 0 };
        trace_target(&context);
    }

    return EXIT_SUCCESS;

}   

