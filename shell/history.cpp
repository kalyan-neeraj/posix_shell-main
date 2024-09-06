#include "history.h"
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <cstdio>
#include <cstdlib>

History::History() {
    load_history();
}

void History::load_history() {
    int fd = open(history_file.c_str(), O_RDONLY | O_CREAT, 0644);
    if (fd == -1) {
        printf("Error: Failed to open history file '%s' for reading.\n", history_file.c_str());
        exit(EXIT_FAILURE);
    }

    char buffer[4096];
    ssize_t bytes_read;
    std::string command;

    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        for (ssize_t i = 0; i < bytes_read; ++i) {
            if (buffer[i] == '\n') {
                history.push_back(command);
                if (history.size() > MAX_HISTORY_SIZE) {
                    history.pop_front();
                }
                command.clear();
            } else {
                command += buffer[i];
            }
        }
    }

    if (bytes_read == -1) {
        printf("Error: Failed to read from history file '%s'.\n", history_file.c_str());
        close(fd);
        exit(EXIT_FAILURE);
    }

    if (!command.empty()) {
        history.push_back(command);
        if (history.size() > MAX_HISTORY_SIZE) {
            history.pop_front();
        }
    }

    if (close(fd) == -1) {
        printf("Error: Failed to close history file '%s' after reading.\n", history_file.c_str());
        exit(EXIT_FAILURE);
    }
}

void History::save_history() {
    int fd = open(history_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd == -1) {
        printf("Error: Failed to open history file '%s' for writing.\n", history_file.c_str());
        exit(EXIT_FAILURE);
    }

    for (const auto& cmd : history) {
        std::string line = cmd + "\n";
        if (write(fd, line.c_str(), line.size()) == -1) {
            printf("Error: Failed to write to history file '%s'.\n", history_file.c_str());
            close(fd);
            exit(EXIT_FAILURE);
        }
    }

    if (close(fd) == -1) {
        printf("Error: Failed to close history file '%s' after writing.\n", history_file.c_str());
        exit(EXIT_FAILURE);
    }
}

void History::add_to_history(const std::string& cmd) {
    if (!cmd.empty()) {
        if (history.size() >= MAX_HISTORY_SIZE) {
            history.pop_front();
        }
        history.push_back(cmd);
        save_history();
    }
}

void History::display_history(int output_fd, int num) {
    std::string output;
    int start = std::max(0, static_cast<int>(history.size()) - num);
    for (auto it = history.begin() + start; it != history.end(); ++it) {
        output += *it + "\n";
    }
    if (write(output_fd, output.c_str(), output.size()) == -1) {
        printf("Error: Failed to write history to output file descriptor.\n");

        exit(EXIT_FAILURE);
    }
}

int History::getCurrSize() {
    return static_cast<int>(history.size());
}

void History::remove() {
    if (!history.empty()) {
        history.pop_back();
    }
}

std::string History::getRequired(long long num, bool isUp) {
    long long history_size = static_cast<long long>(history.size());

    if (history.empty()) {
        return "";
    }

    if (isUp) {
        num = (history_size - num - 1) % history_size;
    } else {
        num = num % history_size;
    }

    return history[num];
}
