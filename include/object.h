#ifndef OBJECT_H
#define OBJECT_H

#include <stdint.h>
#include "points_manage.h"

typedef enum {
    ANY, POINT, CIRCLE, LINE, RAY, SEG
} ObjectType;

typedef struct LineObject_ LineObject;
typedef struct CircleObject_ CircleObject;
typedef struct GeomObject_ GeomObject;
typedef union ObjectSelector_ ObjectSelector;

struct LineObject_ {
    PointObject *pt1, *pt2;
    PointObject *showPt1, *showPt2;
};

struct CircleObject_ {
    PointObject *center, *pt;
    float radius;
};

union ObjectSelector_ {
    PointObject *point;
    LineObject line;
    CircleObject circle;
};

struct GeomObject_ {
    unsigned long long id;
    int show, color;
    ObjectType type;
    GeomObject *next;
    ObjectSelector ptr[0];
};

GeomObject *findObject(ObjectType type, uint64_t id);

int create(int argc, const char **argv);

int midpoint(int argc, const char **argv);

int move_pt(int argc, const char **argv);

#endif //OBJECT_H
