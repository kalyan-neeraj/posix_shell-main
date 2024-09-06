#include "pinfo.h"
#include <cstdio>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>
#include <vector>
#include <string>
#include <sstream>
#include <climits>
#include <errno.h>


std::string error = "not found";
std::string readFile(const std::string& path) {
    int fd = open(path.c_str(), O_RDONLY);
    if (fd < 0) {
        printf("%s", error.c_str());
        exit(1);
        return "";
    }

    char buffer[8192];
    ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
    if (bytes_read < 0) {
        printf("%s", error.c_str());
        close(fd);
        return "";
    }

    close(fd);
    buffer[bytes_read] = '\0';
    return std::string(buffer);
}

std::string extractField(const std::string& content, const std::string& field_name) {
    std::istringstream stream(content);
    std::string line;
    while (std::getline(stream, line)) {
        if (line.find(field_name) == 0) {
            std::istringstream line_stream(line);
            std::string ignore;
            std::string value;
            line_stream >> ignore >> value;
            return value;
        }
    }
    return "";
}

void printPinfo(const std::string& pid, const std::string& process_state, const std::string& memory, const std::string& exe_path) {
    printf("pid -- %s\n", pid.c_str());
    printf("Process Status -- %s\n", process_state.c_str());
    printf("memory -- %s {Virtual Memory}\n", memory.c_str());
    printf("Executable Path -- %s\n", exe_path.c_str());
}

void handlePinfo(const std::vector<std::string>& arguments) {
    std::string pid;
    if (arguments.size() == 0) {
        pid = std::to_string(getpid());
    } else if (arguments.size() == 1) {
        pid = arguments[0];
    } else {
        printf("Format: pinfo [pid]\n");
        return;
    }

    error = pid + " :" + error + "\n";

    std::string status_path = "/proc/" + pid + "/status";
    std::string stat_path = "/proc/" + pid + "/stat";
    std::string exe_path = "/proc/" + pid + "/exe";

    std::string status_content = readFile(status_path);
    if (status_content.empty()) {
        printf("%s", error.c_str());
        return;
    }

    std::string process_state = extractField(status_content, "State:");
    std::string memory = extractField(status_content, "VmSize:");

    std::string stat_content = readFile(stat_path);
    if (stat_content.empty()) {
        printf("%s", error.c_str());
        return;
    }

    std::istringstream stat_stream(stat_content);
    std::vector<std::string> stat_fields;
    std::string field;

    while (stat_stream >> field) {
        stat_fields.push_back(field);
    }

    if (stat_fields.size() < 7) {
        printf("%s", error.c_str());
        return;
    }

    std::string tty_nr = stat_fields[6];
    std::string sid = stat_fields[5];

    if (tty_nr == sid) {
        process_state += "+";
    }

    char exe_path_buffer[PATH_MAX];
    ssize_t len = readlink(exe_path.c_str(), exe_path_buffer, sizeof(exe_path_buffer) - 1);
    if (len != -1) {
        exe_path_buffer[len] = '\0';
    } else {
        printf("%s", error.c_str());
        exe_path_buffer[0] = '\0';
        exit(EXIT_FAILURE);
    }

    printPinfo(pid, process_state, memory, exe_path_buffer);
}
