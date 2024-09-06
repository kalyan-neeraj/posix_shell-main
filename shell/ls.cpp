#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>
#include <cstring>
#include <string>
#include <ctime>
#include <pwd.h>
#include <grp.h>
#include <algorithm>
#include <iostream>

using namespace std;

class LS {
public:
    void custom_ls(const vector<string>& params, int output_fd) {
        vector<string> paths;
        bool show_hidden = false;
        bool long_format = false;

        for (const auto& param : params) {
            if (param == "-a") {
                show_hidden = true;
            } else if (param == "-l") {
                long_format = true;
            } else if (param == "-al" || param == "-la") {
                show_hidden = true;
                long_format = true;
            } else if (param[0] == '-') {
                for (size_t i = 1; i < param.size(); ++i) {
                    if (param[i] == 'a') show_hidden = true;
                    else if (param[i] == 'l') long_format = true;
                }
            } else {
                paths.push_back(param);
            }
        }
        if (paths.empty()) {
            paths.push_back(".");
        }

        for (const auto& path : paths) {
            if (paths.size() > 1) {
                string header = path + ":\n";
                write(output_fd, header.c_str(), header.size());
            }

            DIR* dir = opendir(path.c_str());
            if (!dir) {
                string error_msg = "Error: Unable to open directory\n";
                write(output_fd, error_msg.c_str(), error_msg.size());
                continue;
            }

            vector<string> entries;
            struct dirent* entry;
            off_t total_blocks = 0;

            while ((entry = readdir(dir)) != nullptr) {
                if (!show_hidden && entry->d_name[0] == '.') continue;
                entries.push_back(entry->d_name);

                // Calculate total blocks if long format
                if (long_format) {
                    struct stat file_stat;
                    string full_path = path + "/" + entry->d_name;
                    if (stat(full_path.c_str(), &file_stat) != -1) {
                        total_blocks += file_stat.st_blocks;
                    }
                }
            }
            closedir(dir);

            sort(entries.begin(), entries.end());

            if (long_format) {
                string total_output = "total " + to_string(total_blocks / 2) + "\n";  // Convert to 512-byte blocks
                write(output_fd, total_output.c_str(), total_output.size());
            }

            if (!long_format) {
                for (const auto& name : entries) {
                    string output = name + "\n";
                    write(output_fd, output.c_str(), output.size());
                }
            } else {
                for (const auto& name : entries) {
                    struct stat file_stat;
                    string full_path = path + "/" + name;
                    if (stat(full_path.c_str(), &file_stat) == -1) continue;

                    // Build permission string
                    string perms;
                    perms += (S_ISDIR(file_stat.st_mode) ? 'd' : '-');
                    perms += (file_stat.st_mode & S_IRUSR ? 'r' : '-');
                    perms += (file_stat.st_mode & S_IWUSR ? 'w' : '-');
                    perms += (file_stat.st_mode & S_IXUSR ? 'x' : '-');
                    perms += (file_stat.st_mode & S_IRGRP ? 'r' : '-');
                    perms += (file_stat.st_mode & S_IWGRP ? 'w' : '-');
                    perms += (file_stat.st_mode & S_IXGRP ? 'x' : '-');
                    perms += (file_stat.st_mode & S_IROTH ? 'r' : '-');
                    perms += (file_stat.st_mode & S_IWOTH ? 'w' : '-');
                    perms += (file_stat.st_mode & S_IXOTH ? 'x' : '-');

                    // Get owner and group names
                    struct passwd* pw = getpwuid(file_stat.st_uid);
                    struct group* gr = getgrgid(file_stat.st_gid);
                    string owner = pw ? pw->pw_name : "not user";
                    string group = gr ? gr->gr_name : "not user";

                    // Format modification time
                    char time_buf[80];
                    struct tm* tm_info = localtime(&file_stat.st_mtime);
                    strftime(time_buf, sizeof(time_buf), "%b %d %H:%M", tm_info);

                    // Build output string
                    string output = perms + " " +
                                    to_string(file_stat.st_nlink) + " " +
                                    owner + " " +
                                    group + " " +
                                    to_string(file_stat.st_size) + " " +
                                    time_buf + " " +
                                    name + "\n";
                    write(output_fd, output.c_str(), output.size());
                }
            }

            if (paths.size() > 1) {
                write(output_fd, "\n", 1);
            }
        }
    }
};
