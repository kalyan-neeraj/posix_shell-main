#ifndef RAW_MODE_H
#define RAW_MODE_H

#include <termios.h>
#include <unistd.h>

class Raw {
public:
    Raw();
    ~Raw();

    void enableRawMode();
    void disableRawMode();

private:
    struct termios old_settings;
    bool raw_mode_enabled;
};

#endif
