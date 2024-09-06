#ifndef FG_H
#define FG_H

#include <vector>
#include <string>
#include <sys/wait.h>
#include <queue>
using namespace std;

struct BackgroundJob {
    pid_t pid;
    string command;
};

class FG {
    queue<BackgroundJob> jobQueue;

public:
    void addJob(pid_t pid, const string& command);
    void bringToForeground(const string& jobId = "");
};

#endif