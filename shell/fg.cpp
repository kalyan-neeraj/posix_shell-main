#include "fg.h"
#include <iostream>
#include <csignal>
#include <queue>
#include <sys/wait.h>
#include <unistd.h>

void FG::addJob(pid_t pid, const std::string& command) {
    jobQueue.push({pid, command});
    printf("Job [%d] with command '%s' moved to background.\n", pid, command.c_str());
}


void FG::bringToForeground(const std::string& jobId) {
    if (jobQueue.empty()) {
        printf("No job found\n");
        return;
    }

    pid_t jobPid;
    bool found = false;

    if (jobId.empty()) {
        BackgroundJob job = jobQueue.front();
        jobQueue.pop();
        jobPid = job.pid;
        found = true;
        std::cout << "Bringing job " << jobPid << " (" << job.command << ") to foreground...\n";
    } else {
        try {
            jobPid = stoi(jobId);
        } catch (const std::invalid_argument& e) {
            std::cout << "Invalid job ID: " << jobId << ".\n";
            return;
        } catch (const std::out_of_range& e) {
            std::cout << "Job ID out of range: " << jobId << ".\n";
            return;
        }

        std::queue<BackgroundJob> tempQueue;

        while (!jobQueue.empty()) {
            BackgroundJob job = jobQueue.front();
            jobQueue.pop();
            if (job.pid == jobPid) {
                found = true;
                std::cout << "Bringing job " << jobPid << " (" << job.command << ") to foreground...\n";
            } else {
                tempQueue.push(job);
            }
        }

        jobQueue = tempQueue;

        if (!found) {
            std::cout << "No background job found with PID " << jobPid << ".\n";
            return;
        }
    }

    // Save the original signal handlers
    struct sigaction old_int_action, old_tstp_action, old_quit_action;
    sigaction(SIGINT, nullptr, &old_int_action);
    sigaction(SIGTSTP, nullptr, &old_tstp_action);
    sigaction(SIGQUIT, nullptr, &old_quit_action);

    // Ignore signals in the shell while waiting for the foreground job
    signal(SIGINT, SIG_IGN);
    signal(SIGTSTP, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    // Set the job to the foreground process group and resume it
    tcsetpgrp(STDIN_FILENO, jobPid);
    int status;
    if (waitpid(jobPid, &status, WUNTRACED) == -1) {
        std::cout << "Failed to wait for the process.\n";
    }

    tcsetpgrp(STDIN_FILENO, getpid());

    // Restore the original signal handlers
    sigaction(SIGINT, &old_int_action, nullptr);
    sigaction(SIGTSTP, &old_tstp_action, nullptr);
    sigaction(SIGQUIT, &old_quit_action, nullptr);

    // Check if the process was stopped again
    if (WIFSTOPPED(status)) {
        printf("Process %d was stopped again.\n", jobPid);
        addJob(jobPid, "Stopped Job");  // Re-add the stopped job back to the job queue
    } else if (WIFEXITED(status) || WIFSIGNALED(status)) {
        // The process has exited or was terminated, so it's not re-added
        printf("Process %d has completed.\n", jobPid);
    }
}