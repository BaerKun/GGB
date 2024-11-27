#ifndef BOARD_H
#define BOARD_H

#include "object.h"

void reflashBorad();

GeomObject *mouseSelect(int x, int y);

int show(int argc, const char **argv);

int hide(int argc, const char **argv);

#endif //BOARD_H
