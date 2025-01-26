#include "points_manage.h"

#define QUEUE_ELEMENT_TYPE PointObject *
#include "queue.h"

static PointObject *pointDataSet = NULL;
static int pointDataCount = 0;

PointObject *createPointData(const Point2f pt, PointObject **parents, const int numParents,
                             Point2f (*derive)(PointObject **)) {
    PointObject *obj = malloc(sizeof(PointObject) + sizeof(PointObject *) * numParents);
    obj->next = pointDataSet;
    pointDataSet = obj;

    obj->coord = pt;
    obj->indegree = 0;
    obj->children = NULL;
    obj->derive = derive;

    pointDataCount++;

    if (numParents == 0)
        return obj;

    for (int i = 0; i < numParents; ++i) {
        PointObject *parent = parents[i];
        SubPoint *subpt = malloc(sizeof(SubPoint));

        *subpt = (SubPoint){obj, parent->children};
        parent->children = subpt;
        obj->parents[i] = parent;
    }

    return obj;
}

static void initIndegree(Queue *queue) {
    const int count = queue->size;
    while (queue->size) {
        const PointObject *pt = dequeue(queue);
        for (const SubPoint *subpt = pt->children; subpt; subpt = subpt->next) {
            PointObject *child = subpt->pt;
            if (child->indegree == 0)
                enqueue(queue, child);
            child->indegree++;
        }
    }
    queue->front = 0;
    queue->rear = count;
    queue->size = count;
}

void movePoints(PointObject **pts, const Point2f *dst, const int count) {
    Queue *queue = newQueue(pointDataCount);
    for (int i = 0; i < count; ++i) {
        pts[i]->coord = dst[i];
        enqueue(queue, pts[i]);
    }

    initIndegree(queue);
    while(queue->size) {
        PointObject *pt = dequeue(queue);
        if(pt->derive != NULL)
            pt->coord = pt->derive(pt->parents);

        for (const SubPoint *subpt = pt->children; subpt; subpt = subpt->next) {
            PointObject *child = subpt->pt;
            if (--child->indegree == 0)
                enqueue(queue, child);
        }
    }
    queue_destroy(queue);
}
