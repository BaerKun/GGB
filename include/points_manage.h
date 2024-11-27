#ifndef POINTS_MANAGE_H
#define POINTS_MANAGE_H

#include "geometry.h"

typedef struct PointObject_ PointObject;
typedef struct SubPoint_ SubPoint;

struct SubPoint_ {
    PointObject *pt;
    SubPoint *next;
};

struct PointObject_ {
    Point2f coord;
    int indegree;
    PointObject *next;
    SubPoint *children;

    Point2f (*derive)(PointObject **);

    PointObject *parents[0];
};

PointObject *createPointData(Point2f pt, PointObject **parents, int numParents,
                             Point2f (*derive)(PointObject **));

void movePoints(PointObject **pts, const Point2f *dst, int count);

#endif //POINTS_MANAGE_H
