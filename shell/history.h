#ifndef HISTORY_H
#define HISTORY_H

#include <deque>
#include <string>

class History {
public:
    History();

    void add_to_history(const std::string& cmd);
    void display_history(int output_fd, int num = 10);
    std::string getRequired(long long num, bool isUp);
    int getCurrSize();
    void remove();

public:
    void load_history();
    void save_history();

    std::deque<std::string> history;
    const std::string history_file = "history.txt";
    const size_t MAX_HISTORY_SIZE = 20;
};

#endif