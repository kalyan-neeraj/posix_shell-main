#include <iostream>
#include <string>
#include <unistd.h>
#include <climits>

using namespace std;

class CD {
public:
    string root_directory;
    string home_directory;
    string prev_directory;

    CD() {
        root_directory = "";
        home_directory = "";
        prev_directory = "";
    }

    string get_current_directory() {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd)) != nullptr) {
            return string(cwd);
        }
        perror("getcwd() error");
        return "";
    }

    string get_root_directory() {
        return root_directory;
    }

    string get_home_directory() {
        return home_directory;
    }

    void set_home_directory(string dir) {
        home_directory = dir;
    }

    void set_prev_directory(string dir) {
        prev_directory = dir;
    }

    bool changeDirectory(const string& path) {
        if (path.empty()) {
            string curr = get_current_directory();
            printf("%s", curr.c_str());
        }

        if (path == "-") {
            string temp = get_current_directory();
            if (prev_directory.empty()) {
                prev_directory = temp;
            }
            if (chdir(prev_directory.c_str()) == 0) {
                prev_directory = temp;
                return true;
            }
            perror("chdir() error");
            return false;
        }
        string temp = get_current_directory();
        if (chdir(path.c_str()) == 0) {
            prev_directory = temp;
            return true;
        }
        return false;
    }
};
