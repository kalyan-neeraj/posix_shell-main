#include <iostream>
#include <string>
#include <unistd.h>
#include <vector>

using namespace std;

class Echo {
    public: static void display(vector<string> params, int output_fd) {
        params.emplace_back("\n");
        unsigned long long size = params.size();
        for(unsigned long long i = 0; i < size; i++) {
            string s = params[i];
            if (i != size - 1) s += " ";
            const size_t length = s.length();
            size_t written = 0;

            while (written < length) {
                const ssize_t result = write(output_fd, s.c_str() + written, length - written);
                if (result == -1) {
                    perror("write");
                    return;
                }
                written += result;
            }
        }
    }
};
