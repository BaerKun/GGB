#include <stdlib.h>

#include "board.h"
#include "geom_errors.h"
#include "graphical.h"
#include "object.h"
#include "geom_utils.h"
#include "utils.h"

extern Window *imageWindow;
extern Point2i origin;
extern GeomObject *pointSet, *lineSet, *circleSet;

static inline float getCircleRadius(CircleObject *cr) {
    if (cr->pt == NULL)
        return cr->radius;
    return cr->radius = dist2f(cr->center->coord, cr->pt->coord);
}

static inline float sqrdist_lv(const Vector2f line_dir, const Vector2f vec) {
    return sqr_vec2(vec) - sqr(vec2_dot(vec, line_dir)) / sqr_vec2(line_dir);
}

static float sqrdist_lp(const GeomObject *line, const Point2f p) {
    const Point2f p1 = line->ptr->line.pt1->coord;
    const Point2f p2 = line->ptr->line.pt2->coord;
    const Vector2f vec1 = vec2_from_2p(p1, p),
            vec2 = vec2_from_2p(p2, p),
            lineDir = vec2_from_2p(p1, p2);

    if (line->type == LINE)
        return sqrdist_lv(lineDir, vec1);

    if (line->type == SEG) {
        if (vec2_dot(vec1, lineDir) > 0.f && vec2_dot(vec2, lineDir) < 0.f)
            return sqrdist_lv(lineDir, vec1);
        return A_HUGE_VALF;
    }

    if (vec2_dot(vec1, lineDir) > 0.f)
        return sqrdist_lv(lineDir, vec1);
    return A_HUGE_VALF;
}

void refreshBoard() {
    windowFill(imageWindow, 255, 255, 255);

    for (GeomObject *cr = circleSet; cr != NULL; cr = cr->next)
        if (cr->show)
            drawCircle(imageWindow, toImageCoord(cr->ptr->circle.center->coord, origin),
                       (int) getCircleRadius(&cr->ptr->circle), cr->color, 2);

    for (const GeomObject *ln = lineSet; ln != NULL; ln = ln->next)
        if (ln->show)
            drawLine(imageWindow, toImageCoord(ln->ptr->line.showPt1->coord, origin),
                     toImageCoord(ln->ptr->line.showPt2->coord, origin), ln->color, 2);

    for (const GeomObject *pt = pointSet; pt != NULL; pt = pt->next)
        if (pt->show)
            drawPoint(imageWindow, toImageCoord(pt->ptr->point->coord, origin), pt->color);
}

GeomObject *mouseSelect(const int x, const int y) {
    const Point2f mouse = toMathCoord((Point2i){x, y}, origin);
    const float threshold = 25.f;

    for (GeomObject *pt = pointSet; pt != NULL; pt = pt->next)
        if (pt->show && sqrdist(mouse, pt->ptr->point->coord) < threshold)
            return pt;

    for (GeomObject *ln = lineSet; ln != NULL; ln = ln->next)
        if (ln->show && sqrdist_lp(ln, mouse) < threshold)
            return ln;

    for (GeomObject *cr = circleSet; cr != NULL; cr = cr->next)
        if (cr->show && dist2f(mouse, cr->ptr->circle.center->coord) - cr->ptr->circle.radius < 5.f)
            return cr;

    return NULL;
}

int show(const int argc, const char **argv) {
    if (argc == 1)
        return throwError(ERROR_NO_ARG_GIVEN, noArgGiven(*argv));

    const unsigned long long id = strhash64(argv[1]);
    GeomObject *obj = findObject(ANY, id);

    if (obj == NULL)
        return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(argv[1]));

    if (argc == 2) {
        obj->show = 1;
        refreshBoard();
        return 0;
    }

    char *end;
    const int color = (int) strtol(argv[2], &end, 16);
    if (*end != '\0')
        return throwError(ERROR_INVALID_ARG, invalidColor());

    obj->show = 1;
    obj->color = color;
    refreshBoard();
    return 0;
}

int hide(const int argc, const char **argv) {
    if (argc == 1)
        return throwError(ERROR_NO_ARG_GIVEN, noArgGiven(*argv));

    const uint64_t id = strhash64(argv[1]);
    GeomObject *obj = findObject(ANY, id);

    if (obj == NULL)
        return throwError(ERROR_NOT_FOUND_OBJECT, objectNotFound(argv[1]));

    obj->show = 0;
    refreshBoard();
    return 0;
}
