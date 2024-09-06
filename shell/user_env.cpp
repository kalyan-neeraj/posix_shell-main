#include "user_env.h"

Env_rn::Env_rn() {
    struct passwd *pw = getpwuid(getuid());
    this->user_name = pw ? pw->pw_name : "root";
    
    struct utsname sys_info{};
    uname(&sys_info); 
    
    char path_buffer[PATH_MAX];
    this->root_dir = getcwd(path_buffer, PATH_MAX);
    
    this->system_name = sys_info.nodename;
    this->curr_dir = this->root_dir;
    this->uid = getuid();
}

void Env_rn::updateCurrentDirectory(std::string path) {
    this->curr_dir = path;
}
