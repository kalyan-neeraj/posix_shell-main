#include <cstdio>
#include <cstring>
#include <string>
#include <vector>
#include <unistd.h>
#include <sys/wait.h>
#include <csignal>
#include <termios.h>
#include <fcntl.h>
#include <limits.h>
#include "cd.cpp"
#include "user_env.h"
#include "ls.cpp"
#include "echo.cpp"
#include "fg.h"
#include "search.cpp"
#include "history.h"
#include "raw_mode.h"
#include "auto_complete.cpp"
#include "pinfo.h"

using namespace std;

Env_rn user_env;
CD cd;
LS ls;
Echo echo;
FG fg;
History history;
Raw raw;
AutoComplete auto_comp;
long long h_index = 0;
bool exit_flag = false;

int backGroundId = -1;

string get_Last_Item_from_History(bool isUp) {
    return history.getRequired(h_index, isUp);
}

int get_curr_history_size() {
    return history.getCurrSize();
}

void clearLine() {
    printf("r\33[2K\r");
}

string getShellPrompt() {
    string curr = (user_env.curr_dir == user_env.root_dir) ? "~" : user_env.curr_dir;
    string shell = user_env.user_name + "@" + user_env.system_name + ":" + curr + ">";
    return shell;
}


void set_initial_cd() {
    cd.set_home_directory(user_env.root_dir);
}

struct commandStructure {
    string command;
    vector<string> params;
    bool is_background = false;
};


void signalHandler(int signum) {
    pid_t fg_pid = tcgetpgrp(STDIN_FILENO);

    switch (signum) {
        case SIGTSTP:
             if (fg_pid != getpid()) {
                    kill(-fg_pid, SIGTSTP);
                    printf("\n[%d] Stopped\n", fg_pid);
                    fg.addJob(fg_pid, "stopped process");
                    tcsetpgrp(STDIN_FILENO, getpid());
            }
            break;
        case SIGINT:
            if (fg_pid != getpid()) {
                printf("\nReceived SIGINT. Interrupting the process...\n");
                kill(-fg_pid, SIGINT);
            }
            break;

        case SIGQUIT:
            if (fg_pid != getpid()) {
                clearLine();
                printf("\nReceived SIGQUIT. Quitting the process...\n");
                kill(-fg_pid, SIGQUIT);
            }
            break;

        default:
            break;
    }
}



void setupSignalHandlers() {
    struct sigaction sa;
    sa.sa_handler = signalHandler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    // Handle Ctrl+Z
    sigaction(SIGTSTP, &sa, nullptr);

    // Handle Ctrl+C
    sigaction(SIGINT, &sa, nullptr);

    // Handle Ctrl+D
    sigaction(SIGQUIT, &sa, nullptr);

    signal(SIGTTOU, SIG_IGN);
    signal(SIGTTIN, SIG_IGN);
 }

void execute_custom_pwd() {
    char cwd[PATH_MAX];
    if (getcwd(cwd, sizeof(cwd)) != nullptr) {
        printf("%s\n", cwd);
    } else {
        printf("Error retrieving current directory\n");
    }
}

void convert_string_to_char_array(const vector<string>& parameters, vector<char*>& argv) {
    for (const auto& param : parameters) {
        argv.push_back(const_cast<char*>(param.c_str()));
    }
    argv.push_back(nullptr);
}

//command executions//
void execute_custom_command(commandStructure& cmd) {
    if (cmd.command == "cd") {
        if (!cmd.params.empty()) {
            if (cmd.params.size() > 1) {
                printf("Error: cd can only have one path to change\n");
    
                return;
            }
            string path = cmd.params[0];
            if (path.find('~') != string::npos) {
                path = user_env.root_dir;
            }
            if (cd.changeDirectory(path)) {
                user_env.updateCurrentDirectory(cd.get_current_directory());
            } else {
                printf("Error: Failed to change directory to '%s'\n", path.c_str());
    
            }
        } else {
            if (cd.changeDirectory(cd.get_home_directory())) {
                user_env.updateCurrentDirectory(cd.get_current_directory());
            } else {
                printf("Error: Failed to change to home directory\n");
    
            }
        }
    }
}

void handle_background(commandStructure& cmd) {
    pid_t pid = fork(); 
    if (pid == 0)  
    {

    signal(SIGTSTP, SIG_DFL);
    signal(SIGINT, SIG_DFL);
    signal(SIGQUIT, SIG_DFL);

    // If this is a background job
    if (cmd.is_background) {
        // Redirect standard input, output, and error for background jobs
        int dev_null_fd = open("/dev/null", O_RDWR);
        if (dev_null_fd < 0) {
            perror("Error: Cannot open /dev/null");
            exit(EXIT_FAILURE);
        }
        dup2(dev_null_fd, STDIN_FILENO);
        dup2(dev_null_fd, STDOUT_FILENO);
        dup2(dev_null_fd, STDERR_FILENO);
        close(dev_null_fd);

        // No need to print anything here, the parent will handle that.
    }
        
        vector<char*> argv;
        convert_string_to_char_array(cmd.params, argv);
        argv.insert(argv.begin(), const_cast<char*>(cmd.command.c_str()));

        setpgid(0,0);
        execvp(cmd.command.c_str(), argv.data());
    }
    else if (pid > 0) 
    {

        printf("Job [%d] with command '%s' moved to background.\n", pid, cmd.command.c_str());
        sigaction(SIGTSTP, NULL, nullptr);        
    }
    else  
    {
        printf("Failed to create a child process for background execution.\n");
    }
}

   
void execute_single_command(commandStructure& cmd, int input_fd, int output_fd) {
    if(cmd.command.empty()) {
        return;
    }

    if (cmd.is_background) {
        handle_background(cmd);
        return;
    }

    int custom_input_fd = input_fd;
    int custom_output_fd = output_fd;
        for (unsigned long long i = 0; i < cmd.params.size(); ++i) {
        if (cmd.params[i] == "<") {
            if (i + 1 < cmd.params.size()) {
                custom_input_fd = open(cmd.params[i + 1].c_str(), O_RDONLY);
                if (custom_input_fd < 0) {
                    printf("Error: Cannot open input file '%s'\n", cmd.params[i + 1].c_str());
        
                    return;
                }
                cmd.params.erase(cmd.params.begin() + i, cmd.params.begin() + i + 2);
                i--;
            }
        } else if (cmd.params[i] == ">" || cmd.params[i] == ">>") {
            if (i + 1 < cmd.params.size()) {
                int created_flags = O_WRONLY | O_CREAT;
                if (cmd.params[i] == ">>") {
                    created_flags |= O_APPEND;
                } else {
                    created_flags |= O_TRUNC;
                }

                custom_output_fd = open(cmd.params[i + 1].c_str(), created_flags, 0644);
                if (custom_output_fd < 0) {
                    printf("Error: Cannot open or create output file '%s'\n", cmd.params[i + 1].c_str());
        
                    return;
                }
                cmd.params.erase(cmd.params.begin() + i, cmd.params.begin() + i + 2);
                i--;
            }
        }
    }

    if (cmd.command == "fg") {
        fg.bringToForeground();
        return;
    }
    if (cmd.command == "cd") {
        execute_custom_command(cmd);
        return;
    }

    pid_t pid = fork();

    if (pid == -1) {
        printf("Error in Fork\n");
        return;
    }

    if (pid == 0) {

        if (custom_input_fd != STDIN_FILENO) {
            dup2(custom_input_fd, STDIN_FILENO);
            close(custom_input_fd);
        }
        if (custom_output_fd != STDOUT_FILENO) {
            dup2(custom_output_fd, STDOUT_FILENO);
            close(custom_output_fd);
        }
        // Handle custom commands
        if (cmd.command == "pwd") {
            execute_custom_pwd();

            exit(EXIT_SUCCESS);
        }
        if (cmd.command == "ls") {
            ls.custom_ls(cmd.params, STDOUT_FILENO);

            exit(EXIT_SUCCESS);
        }

        if (cmd.command == "echo") {
            echo.display(cmd.params, STDOUT_FILENO);

            exit(EXIT_SUCCESS);
        }

        if (cmd.command == "search") {
            bool found = search_by_bfs(cmd.params[0]);
            string s = found ? "True" : "False";
            printf("%s\n", s.c_str());

            exit(EXIT_SUCCESS);
        }

        if (cmd.command == "history") {

            int num = cmd.params.size() ? stoi(cmd.params[0]) : 10;
            history.display_history(STDOUT_FILENO, num);
            exit(EXIT_SUCCESS);
        }

        if (cmd.command == "pinfo") {
            handlePinfo(cmd.params);
            exit(EXIT_SUCCESS);
        }

        // For non-custom commands
        vector<char*> argv;
        convert_string_to_char_array(cmd.params, argv);
        argv.insert(argv.begin(), const_cast<char*>(cmd.command.c_str()));
        
        execvp(cmd.command.c_str(), argv.data());
        printf("%s%s\n", "Error in executing command : ", cmd.command.c_str());
        exit(EXIT_FAILURE);
    }

    if (custom_input_fd != STDIN_FILENO) {
        close(custom_input_fd);
    }
    if (custom_output_fd != STDOUT_FILENO) {
        close(custom_output_fd);
    }
    waitpid(pid, NULL, WUNTRACED);

}

void execute_command(vector<vector<commandStructure>>& allCommandsList) {
    for (auto& commandsList : allCommandsList) {
        int fd[2];
        int in_fd = STDIN_FILENO;
        unsigned long long num_commands = commandsList.size();

        for (unsigned long long i = 0; i < num_commands; i++) {
            if (i < num_commands - 1) {
                if (pipe(fd) == -1) {
                    printf("Error in establihing a pipe\n");
                    return;
                }
            }

            int output_fd = (i < num_commands - 1) ? fd[1] : STDOUT_FILENO;

            execute_single_command(commandsList[i], in_fd, output_fd);

            if (in_fd != STDIN_FILENO) {
                close(in_fd);
            }
            if (output_fd != STDOUT_FILENO) {
                close(output_fd);
            }

            in_fd = fd[0];
        }
    }
}


string trim(const string& str) {
    unsigned long long start = str.find_first_not_of(" \t\n\r");
    unsigned long long end = str.find_last_not_of(" \t\n\r");
    if (start == string::npos) {
        return "";
    }
    return str.substr(start, end - start + 1);
}


void process_command(const string& command, vector<commandStructure>& commandsList) {
    commandStructure commandStruct;
    long long pos = 0;
    bool firstToken = true;
    string cmd = command;
    if (cmd.back() == '&') {
        commandStruct.is_background = true;
        cmd.pop_back();
    }
    while ((pos = cmd.find(' ')) != static_cast<long long>(string::npos)) {
        string trimmed_cmd = trim(cmd.substr(0, pos));
        cmd.erase(0, pos + 1);

        if (!trimmed_cmd.empty()) {
            if (firstToken) {
                commandStruct.command = trimmed_cmd;
                firstToken = false;
            } else {
                commandStruct.params.push_back(trimmed_cmd);
            }
        }
    }
    string rem_cmd = trim(cmd);
    if (!rem_cmd.empty()) {
        if (firstToken) {
            commandStruct.command = rem_cmd;
        } else {
            commandStruct.params.push_back(rem_cmd);
        }
    }
    commandsList.push_back(commandStruct);
}


vector<commandStructure> parsePipedCommands(const string& input) {
    vector<commandStructure> commandsList;
    long long pos = 0, lastPos = 0;
 
    while ((pos = input.find('|', lastPos)) != static_cast<long long>(string::npos)) {
        string command = trim(input.substr(lastPos, pos - lastPos));
        if (!command.empty()) {
            process_command(command, commandsList);
        }
        lastPos = pos + 1;
    }

    string lastCommand = trim(input.substr(lastPos));
    if (!lastCommand.empty()) {
        process_command(lastCommand, commandsList);
    }

    return commandsList;
}

vector<vector<commandStructure>> getParseInput(string input) {
    vector<vector<commandStructure>> command_list;
    if (input.length() == 0) {
        return command_list;
    }

    size_t first_non_null = input.find_first_not_of('\0');
    if (first_non_null == std::string::npos) {
        input = "";
    }

    input = input.substr(first_non_null);


    vector<unsigned long>getSemiColons;
    for (unsigned long long i = 0; i < input.length(); i++) {
        if (input[i] == ';') {
            getSemiColons.push_back(i);
        }
    }

    if (getSemiColons.empty()) {
        getSemiColons.push_back(input.length()+1);
    }
    unsigned start = 0;
    for (unsigned long long i = 0; i < getSemiColons.size(); i++) {
        string command = trim(input.substr(start, getSemiColons[i]));
        vector<commandStructure> get_commands = parsePipedCommands(command);
        start = getSemiColons[i];
        command_list.emplace_back(get_commands);
    }
    return command_list;
}



string read_input_from_shell() {
    string input;
    char c;
    string current_input;

   while (true) {
        c = getchar();
        if (c == EOF || c == 4) {
            exit_flag = true;
            printf("\n");
            return input;
        } else if (c == '\n') { 
            printf("\n");
            break;
        } else if (c == '\t') {
            string shell = getShellPrompt();
            auto_comp.displaySuggestions(input, shell);
            continue;
        } else if (c == 127) {
            string shell = getShellPrompt();
             if (!input.empty()) {
                input.pop_back();
                string shell = getShellPrompt() + input;
                printf("%s%s", "\033[2K\r", shell.c_str());
                fflush(stdout);
            }
        }   
         else if (c == 27) {
             c = getchar();
            int curr_his_size = get_curr_history_size();
             h_index = (h_index) % curr_his_size;
             if (c == 91) {
                 c = getchar();
                 if (c == 'A') {
                     string last_command = get_Last_Item_from_History(true);
                     if (!last_command.empty()) {
                         clearLine();
                         string shell = getShellPrompt();
                         input = last_command;
                         printf("%s%s", shell.c_str(), input.c_str());
                     }
                 } else if (c == 'B') {
                      string last_command = get_Last_Item_from_History(false);
                     if (!last_command.empty()) {
                         clearLine();
                         string shell = getShellPrompt();
                         input = last_command;
                         printf("%s%s", shell.c_str(), input.c_str());
                     }
                 }
                 h_index++;
             }
         }
        else {
            if (c != '\0') {
                 input += c;
                 putchar(c);
            }
           
        }
    }

    return input;
}

void start_shell() {
    while (!exit_flag) {
        raw.enableRawMode();
        string shell = getShellPrompt();
        printf("%s", shell.c_str());
        string input = read_input_from_shell();
        if (input == "A") {
            string last_command = get_Last_Item_from_History(true);
        }
        input = trim(input);
        if (input.length() != 0) {
            vector<vector<commandStructure>> allCommandsList = getParseInput(input);
            history.add_to_history(input);
            if (!allCommandsList.empty()) {
            execute_command(allCommandsList);
        }
        }
        raw.disableRawMode();
    }
}

int main() {
    setupSignalHandlers();
    set_initial_cd();
    start_shell();
    return 0;
}