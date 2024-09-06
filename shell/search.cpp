
#include "search.h"
#include <iostream>
#include <dirent.h>
#include <cstring>
#include <string>
#include <queue>
#include <vector>

using namespace std;

bool search_by_bfs(const string& target) {
    string startDir = ".";
    queue<string> directories;
    directories.push(startDir);

    while (!directories.empty()) {
        string currentDir = directories.front();
        directories.pop();

        DIR* dir = opendir(currentDir.c_str());
        if (dir == nullptr) {
            cerr << "Error opening directory: " << currentDir << '\n';
            continue;
        }

        struct dirent* entry;
        while ((entry = readdir(dir)) != nullptr) {
            if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
                continue;
            }

            string path = currentDir + "/" + entry->d_name;
            if (entry->d_type == DT_REG || entry->d_type == DT_DIR) {
                if (entry->d_name == target) {
                    closedir(dir);
                    return true;
                }
                if (entry->d_type == DT_DIR) {
                    directories.push(path);
                }
            }
        }
        closedir(dir);
    }

    return false;
}