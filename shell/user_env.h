
#ifndef ENV_RN_H
#define ENV_RN_H

#include <iostream>
#include <unistd.h>
#include <sys/wait.h>
#include <string>
#include <vector>
#include <cstring>
#include <climits>
#include <sys/utsname.h>
#include <pwd.h>

using namespace std;

class Env_rn {
public:
    string user_name;
    string system_name;
    string root_dir;
    string curr_dir;
    int uid;

    Env_rn();

    void updateCurrentDirectory(string path);
};

#endif 
