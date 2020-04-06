#ifndef SETUP_H
#define SETUP_H
#include <cstdint>

class Setup
{
public:
    static int8_t CreateShortcut();
    static bool InstallShortcut(const wchar_t *shortcutPath);
};

#endif // SETUP_H
