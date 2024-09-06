#include "raw_mode.h"
#include <cstdio> // For perror

Raw::Raw() : raw_mode_enabled(false) {}

Raw::~Raw() {
    if (raw_mode_enabled) {
        disableRawMode();
    }
}

void Raw::enableRawMode() {
    if (raw_mode_enabled) {
        return;
    }

    if (tcgetattr(STDIN_FILENO, &old_settings) == -1) {
        perror("Error getting terminal attributes");
        return;
    }

    struct termios raw = old_settings;
    raw.c_lflag &= ~(ECHO | ICANON); // Disable echo and canonical mode

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw) == -1) {
        perror("Error setting terminal attributes");
        return;
    }

    raw_mode_enabled = true;
}

void Raw::disableRawMode() {
    if (!raw_mode_enabled) {
        return;
    }

    if (tcsetattr(STDIN_FILENO, TCSAFLUSH, &old_settings) == -1) {
        perror("Error restoring terminal attributes");
    }

    raw_mode_enabled = false;
}
