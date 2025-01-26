#include "points_manage.h"
#include "object.h"
#include "geom_utils.h"
#include "geom_errors.h"
#include "board.h"
#include "utils.h"

#include <stdlib.h>

// private
GeomObject *pointSet = NULL, *lineSet = NULL, *circleSet = NULL;

static int getArgs(ObjectType type, const char *arg1, const char *arg2, ObjectSelector *arg);

static void createGeomObject(ObjectType type, const ObjectSelector *arg, uint64_t id, int show, int rgb);

static int getOptionalObjectArgs(const char **argv, const char **endptr, uint64_t *id, int *show, int *rgb);

static GeomObject *findObjectHelper(GeomObject *head, uint64_t id);

static Point2f midpointCallback(PointObject **pt);


// public
GeomObject *findObject(const ObjectType type, const uint64_t id) {
    GeomObject *obj;
    switch (type) {
        case POINT:
            return findObjectHelper(pointSet, id);
        case CIRCLE:
            return findObjectHelper(circleSet, id);
        case ANY:
            obj = findObjectHelper(pointSet, id);
            if (obj != NULL)
                return obj;
            obj = findObjectHelper(lineSet, id);
            if (obj != NULL)
                return obj;
            obj = findObjectHelper(circleSet, id);
            return obj;
        default:
            obj = findObjectHelper(lineSet, id);
            if (obj != NULL && obj->type != type)
                return NULL;
            return obj;
    }
}

int create(const int argc, const char **argv) {
    if (argc == 1)
        return throwError(ERROR_NO_ARG_GIVEN, noArgGiven(*argv));

    ObjectType type;
    ObjectSelector arg;
    switch (strhash64(argv[1])) {
        case STR_HASH64('p', 'o', 'i', 'n', 't', 0, 0, 0):
            type = POINT;
            break;
        case STR_HASH64('l', 'i', 'n', 'e', 0, 0, 0, 0):
            type = LINE;
            break;
        case STR_HASH64('r', 'a', 'y', 0, 0, 0, 0, 0):
            type = RAY;
            break;
        case STR_HASH64('s', 'e', 'g', 0, 0, 0, 0, 0):
            type = SEG;
            break;
        case STR_HASH64('c', 'i', 'r', 'c', 'l', 'e', 0, 0):
            type = CIRCLE;
            break;
        case STR_HASH64('h', 'e', 'l', 'p', 0, 0, 0, 0):
            return throwError(ERROR_HELP, "Usage: create <object> <arg1> <arg2> [as <name>]");
        default:
            return throwError(ERROR_INVALID_ARG, invalidArg("object-type", "Please point/line/ray/seg/circle"));
    }

    if (argc < 4)
        return throwError(ERROR_NOT_ENOUGH_ARG, notEnoughArg(*argv));

    int error = getArgs(type, argv[2], argv[3], &arg);
    if (error != 0)
        return error;

    uint64_t id;
    int show, rgb;
    error = getOptionalObjectArgs(argv + 4, argv + argc, &id, &show, &rgb);
    if (error != 0)
        return error;

    createGeomObject(type, &arg, id, show, rgb);
    refreshBoard();
    return 0;
}

int midpoint(const int argc, const char **argv) {
    if (argc == 1)
        return throwError(ERROR_NO_ARG_GIVEN, noArgGiven(*argv));

    const GeomObject *pt2,
            *pt1 = findObject(POINT, strhash64(argv[1]));
    if (pt1 == NULL)
        return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(argv[1]));
    if (argc >= 3) {
        pt2 = findObject(POINT, strhash64(argv[2]));
        if (pt2 == NULL)
            return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(argv[2]));
    } else {
        return throwError(ERROR_NOT_ENOUGH_ARG, notEnoughArg(*argv));
    }

    uint64_t id;
    int show, rgb;
    const int error = getOptionalObjectArgs(argv + 3, argv + argc, &id, &show, &rgb);
    if (error != 0)
        return error;

    PointObject *parents[2] = {pt1->ptr->point, pt2->ptr->point};
    const PointObject *mid = createPointData(midpt(pt1->ptr->point->coord, pt2->ptr->point->coord),
                                             parents, 2, &midpointCallback);

    createGeomObject(POINT, (ObjectSelector *) &mid, id, show, rgb);
    refreshBoard();
    return 0;
}

int move_pt(const int argc, const char **argv) {
    if (argc == 1)
        return throwError(ERROR_NO_ARG_GIVEN, noArgGiven(*argv));

    PointObject *pts[16];
    int countpts = 0;
    for (const char **arg = argv + 1; countpts < 16; ++countpts, ++arg) {
        const uint64_t hash = strhash64(*arg);
        if(hash == STR_HASH64('t', 'o', 0, 0, 0, 0, 0, 0))
            break;

        const GeomObject *src = findObject(POINT, hash);
        if (src == NULL)
            return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(*arg));

        pts[countpts] = src->ptr->point;
    }
    if (countpts == 0)
        return throwError(ERROR_NOT_ENOUGH_ARG, notEnoughArg(*argv));

    Point2f dst[16];
    int countdst = 0;
    for(const char **arg = argv + 2 + countpts, **end = argv + argc; arg != end && countdst < countpts; ++countdst, ++arg) {
        const uint64_t hash = strhash64(*arg);
        const GeomObject *dst_ = findObject(POINT, hash);
        if (dst_ == NULL)
            return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(*arg));

        dst[countdst] = dst_->ptr->point->coord;
    }
    if (countdst != countpts)
        return throwError(ERROR_INVALID_ARG, "The count of dst is different from pts");

    movePoints(pts, dst, countpts);
    refreshBoard();
    return 0;
}

// private
static Point2f midpointCallback(PointObject **pt) {
    return midpt(pt[0]->coord, pt[1]->coord);
}

static GeomObject *findObjectHelper(GeomObject *head, const uint64_t id) {
    while (head != NULL) {
        if (head->id == id)
            break;
        head = head->next;
    }
    return head;
}

static GeomObject *getNewObject(const ObjectType type) {
    GeomObject *obj;
    switch (type) {
        case POINT:
            obj = malloc(sizeof(GeomObject) + sizeof(PointObject *));
            obj->next = pointSet;
            pointSet = obj;
            return obj;
        case CIRCLE:
            obj = malloc(sizeof(GeomObject) + sizeof(CircleObject));
            obj->next = circleSet;
            circleSet = obj;
            return obj;
        case LINE:
        case RAY:
        case SEG:
            obj = malloc(sizeof(GeomObject) + sizeof(LineObject));
            obj->next = lineSet;
            lineSet = obj;
            return obj;
        default:
            return NULL;
    }
}

static void createGeomObject(const ObjectType type, const ObjectSelector *arg, const uint64_t id, const int show,
                             const int rgb) {
    GeomObject *obj = getNewObject(type);
    if (obj == NULL)
        return;

    obj->id = id;
    obj->type = type;
    obj->show = show;
    obj->color = rgb;

    switch (type) {
        case POINT:
            obj->ptr->point = arg->point;
            break;
        case CIRCLE:
            obj->ptr->circle = arg->circle;
            break;
        case LINE:
        case RAY:
        case SEG:
            obj->ptr->line = arg->line;
        default:
            break;
    }
}

static inline int randomColor() {
    return (int) (random32() & 0xffffff);
}

static uint64_t getDefaultId() {
    static uint64_t id = STR_HASH64('#', '0', '0', '0', 0, 0, 0, 0);
    char *c = (char *) &id + 3;
    do {
        if (*c != '9') {
            ++*c;
            return id;
        }
        *c = '0';
        --c;
    } while (*c != '#');

    return 0;
}

static int getPointArg(const char *arg1, const char *arg2, PointObject **arg) {
    char *end;

    const float x = strtof(arg1, &end);
    if (*end != '\0')
        return throwError(ERROR_INVALID_ARG, invalidArg("x-coord", NULL));

    const float y = strtof(arg2, &end);
    if (*end != '\0')
        return throwError(ERROR_INVALID_ARG, invalidArg("y-coord", NULL));

    *arg = createPointData((Point2f){x, y}, NULL, 0, NULL);
    return 0;
}

static int getLineArg(const char *arg1, const char *arg2, LineObject *arg) {
    const uint64_t id1 = strhash64(arg1);
    const GeomObject *obj = findObjectHelper(pointSet, id1);
    if (obj == NULL)
        return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(arg1));
    arg->pt1 = obj->ptr->point;

    const uint64_t id2 = strhash64(arg2);
    obj = findObjectHelper(pointSet, id2);
    if (obj == NULL)
        return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(arg2));
    arg->pt2 = obj->ptr->point;

    return 0;
}

static int getCircleArg(const char *arg1, const char *arg2, CircleObject *arg) {
    const uint64_t id1 = strhash64(arg1);
    const GeomObject *obj = findObjectHelper(pointSet, id1);
    if (obj == NULL)
        return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(arg1));
    arg->center = obj->ptr->point;

    if ((*arg2 >= '0' && *arg2 <= '9') || *arg2 == '.') {
        char *end;
        const float radus = strtof(arg2, &end);
        if (*end != '\0')
            return throwError(ERROR_INVALID_ARG, invalidArg("radius", NULL));

        arg->pt = NULL;
        arg->radius = radus;
        return 0;
    }

    const uint64_t id2 = strhash64(arg2);
    obj = findObjectHelper(pointSet, id2);
    if (obj == NULL)
        return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(arg2));
    arg->pt = obj->ptr->point;

    return 0;
}

static Point2f lineCallback(PointObject **pt) {
    const Point2f pt1 = pt[0]->coord;
    const Vector2f vec = vec2_from_2p(pt1, pt[1]->coord);
    const float norm = norm_vec2(vec);
    return (Point2f){pt1.x + vec.x / norm * A_HUGE_VALF, pt1.y + vec.y / norm * A_HUGE_VALF};
}

static int getArgs(const ObjectType type, const char *arg1, const char *arg2, ObjectSelector *arg) {
    int error;
    switch (type) {
        case POINT:
            return getPointArg(arg1, arg2, &arg->point);
        case CIRCLE:
            return getCircleArg(arg1, arg2, &arg->circle);
        default:
            error = getLineArg(arg1, arg2, &arg->line);
            if (error != 0)
                return error;

            LineObject *line = &arg->line;
            PointObject *parents[2] = {line->pt1, line->pt2};
            switch (type) {
                case SEG:
                    line->showPt1 = line->pt1;
                    line->showPt2 = line->pt2;
                    break;
                case RAY:
                    line->showPt1 = line->pt1;
                    line->showPt2 = createPointData(lineCallback(parents), parents, 2, lineCallback);
                    break;
                default:
                    line->showPt2 = createPointData(lineCallback(parents), parents, 2, lineCallback);
                    parents[0] = line->pt2;
                    parents[1] = line->pt1;
                    line->showPt1 = createPointData(lineCallback(parents), parents, 2, lineCallback);
            }
    }
    return 0;
}

static int getOptionalObjectArgs(const char **argv, const char **endptr, uint64_t *id, int *show, int *rgb) {
    *id = 0;
    *show = 1;
    *rgb = -1;
    while (argv != endptr) {
        const char *end;
        switch (strhash64(*argv)) {
            case STR_HASH64('a', 's', 0, 0, 0, 0, 0, 0):
                if (++argv == endptr)
                    break;
                *id = strhash64(*argv++);
                break;

            case STR_HASH64('-', '-', 's', 'h', 'o', 'w', 0, 0):
                if (++argv == endptr)
                    break;
                *show = strtobool(*argv++, &end);
                if (*end != '\0')
                    return throwError(ERROR_INVALID_ARG, invalidArg("show", "Please true/false"));
                break;

            case STR_HASH64('-', '-', 'c', 'o', 'l', 'o', 'r', 0):
                if (++argv == endptr)
                    break;
                *rgb = (int) strtol(*argv++, (char **) &end, 16);
                if (*end != '\0')
                    return throwError(ERROR_INVALID_ARG, invalidColor());
                break;

            default:
                return throwError(ERROR_UNKOWN_ARG, unknownArgs(*argv));
        }
    }
    if (*id == 0)
        *id = getDefaultId();
    if (*rgb)
        *rgb = randomColor();
    return 0;
}
