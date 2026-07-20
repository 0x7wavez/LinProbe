#ifndef LINPROBE_TRACER_H
#define LINPROBE_TRACER_H

#include <sys/types.h>

typedef struct {
    pid_t target_pid;
    int status;
} TracerContext;

void handle_error(const char *msg);
void run_child_process(char *commands, char **args);
void trace_target(TracerContext *ctx);

#endif
