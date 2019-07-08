/*
    pygame - Python Game Library
    Copyright (C) 2000-2001  Pete Shinners

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Pete Shinners
    pete@shinners.org
*/

/*
 *  drawing module for pygame
 */
#include "pygame.h"

#include "pgcompat.h"

#include "doc/draw_doc.h"

#include <math.h>

#include <float.h>

/*
    Many C libraries seem to lack the trunc call (added in C99).

    Not sure int() is usable for all cases where trunc is used in this code?
    However casting to int gives quite a speedup over the one defined.
    Now sure how it compares to the trunc built into the C library.
    #define trunc(d) ((int)(d))
*/
#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L) && !defined(trunc)
#define trunc(d) (((d) >= 0.0) ? (floor(d)) : (ceil(d)))
#endif

#define FRAC(z) ((z)-trunc(z))
#define INVFRAC(z) (1 - FRAC(z))

/* Float versions.
 *
 * See comment above about some C libraries lacking the trunc function. The
 * functions truncf, floorf, and ceilf could also be missing as they were
 * added in C99 as well. Just use the double functions and cast to a float.
 */
#if (!defined(__STDC_VERSION__) || __STDC_VERSION__ < 199901L) && \
    !defined(truncf)
#define truncf(x) ((float)(((x) >= 0.0f) ? (floor(x)) : (ceil(x))))
#endif

#define FRAC_FLT(z) ((z)-truncf(z))
#define INVFRAC_FLT(z) (1 - FRAC_FLT(z))

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

static int
clip_and_draw_line(SDL_Surface *surf, SDL_Rect *rect, Uint32 color, int *pts);
static int
clip_and_draw_aaline(SDL_Surface *surf, SDL_Rect *rect, Uint32 color,
                     float *pts, int blend);
static int
clip_and_draw_line_width(SDL_Surface *surf, SDL_Rect *rect, Uint32 color,
                         int width, int *pts);
static int
clipline(int *pts, int left, int top, int right, int bottom);
static int
clip_aaline(float *pts, int left, int top, int right, int bottom);
static void
drawline(SDL_Surface *surf, Uint32 color, int startx, int starty, int endx,
         int endy);
static void
draw_aaline(SDL_Surface *surf, Uint32 color, float startx, float starty,
           float endx, float endy, int blend);
static void
drawhorzline(SDL_Surface *surf, Uint32 color, int startx, int starty,
             int endx);
static void
drawvertline(SDL_Surface *surf, Uint32 color, int x1, int y1, int y2);
static void
draw_arc(SDL_Surface *dst, int x, int y, int radius1, int radius2,
         double angle_start, double angle_stop, Uint32 color);
static void
draw_ellipse(SDL_Surface *dst, int x, int y, int width, int height, int solid,
             Uint32 color);
static void
draw_fillpoly(SDL_Surface *dst, int *vx, int *vy, Py_ssize_t n, Uint32 color);

// validation of a draw color
#define CHECK_LOAD_COLOR(colorobj)                                         \
    if (PyInt_Check(colorobj))                                             \
        color = (Uint32)PyInt_AsLong(colorobj);                            \
    else if (pg_RGBAFromColorObj(colorobj, rgba))                          \
        color =                                                            \
            SDL_MapRGBA(surf->format, rgba[0], rgba[1], rgba[2], rgba[3]); \
    else                                                                   \
        return RAISE(PyExc_TypeError, "invalid color argument");

/* Draws an antialiased line on the given surface.
 *
 * Returns a Rect bounding the drawn area.
 */
static PyObject *
aaline(PyObject *self, PyObject *arg, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *start = NULL, *end = NULL;
    SDL_Surface *surf = NULL;
    float startx, starty, endx, endy;
    int top, left, bottom, right, anydraw;
    int blend = 1; /* Default blend. */
    float pts[4];
    Uint8 rgba[4];
    Uint32 color;
    static char *keywords[] = {"surface", "color", "start_pos",
                               "end_pos", "blend", NULL};

    if (!PyArg_ParseTupleAndKeywords(arg, kwargs, "O!OOO|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &start, &end, &blend)) {
        return NULL; /* Exception already set. */
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    if (!pg_TwoFloatsFromObj(start, &startx, &starty)) {
        return RAISE(PyExc_TypeError, "invalid start_pos argument");
    }

    if (!pg_TwoFloatsFromObj(end, &endx, &endy)) {
        return RAISE(PyExc_TypeError, "invalid end_pos argument");
    }

    if (!pgSurface_Lock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    pts[0] = startx;
    pts[1] = starty;
    pts[2] = endx;
    pts[3] = endy;
    anydraw = clip_and_draw_aaline(surf, &surf->clip_rect, color, pts, blend);

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    /* Compute return rect. */
    if (!anydraw) {
        return pgRect_New4((int)startx, (int)starty, 0, 0);
    }

    if (pts[0] < pts[2]) {
        left = (int)(pts[0]);
        right = (int)(pts[2]);
    }
    else {
        left = (int)(pts[2]);
        right = (int)(pts[0]);
    }

    if (pts[1] < pts[3]) {
        top = (int)(pts[1]);
        bottom = (int)(pts[3]);
    }
    else {
        top = (int)(pts[3]);
        bottom = (int)(pts[1]);
    }

    return pgRect_New4(left, top, right - left + 2, bottom - top + 2);
}

/* Draws a line on the given surface.
 *
 * Returns a Rect bounding the drawn area.
 */
static PyObject *
line(PyObject *self, PyObject *arg, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *start = NULL, *end = NULL;
    SDL_Surface *surf = NULL;
    int startx, starty, endx, endy, anydraw;
    int pts[4];
    Uint8 rgba[4];
    Uint32 color;
    int width = 1; /* Default width. */
    static char *keywords[] = {"surface", "color", "start_pos",
                               "end_pos", "width", NULL};

    if (!PyArg_ParseTupleAndKeywords(arg, kwargs, "O!OOO|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &start, &end, &width)) {
        return NULL; /* Exception already set. */
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    if (!pg_TwoIntsFromObj(start, &startx, &starty)) {
        return RAISE(PyExc_TypeError, "invalid start_pos argument");
    }

    if (!pg_TwoIntsFromObj(end, &endx, &endy)) {
        return RAISE(PyExc_TypeError, "invalid end_pos argument");
    }

    if (width < 1) {
        return pgRect_New4(startx, starty, 0, 0);
    }

    if (!pgSurface_Lock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    pts[0] = startx;
    pts[1] = starty;
    pts[2] = endx;
    pts[3] = endy;
    anydraw =
        clip_and_draw_line_width(surf, &surf->clip_rect, color, width, pts);

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    if (!anydraw) {
        return pgRect_New4(startx, starty, 0, 0);
    }

    /* The pts array was updated with the top left and bottom right corners
     * of the bounding rect: {left, top, right, bottom}. That is used to
     * construct the rect bounding the changed area. */
    return pgRect_New4(pts[0], pts[1], pts[2] - pts[0] + 1,
                       pts[3] - pts[1] + 1);
}

/* Draws a series of antialiased lines on the given surface.
 *
 * Returns a Rect bounding the drawn area.
 */
static PyObject *
aalines(PyObject *self, PyObject *arg, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *closedobj = NULL;
    PyObject *points = NULL, *item = NULL;
    SDL_Surface *surf = NULL;
    Uint32 color;
    Uint8 rgba[4];
    float pts[4];
    float *xlist, *ylist;
    float x, y;
    float top = FLT_MAX, left = FLT_MAX;
    float bottom = FLT_MIN, right = FLT_MIN;
    int result;
    int closed = 0; /* Default closed. */
    int blend = 1;  /* Default blend. */
    Py_ssize_t loop, length;
    static char *keywords[] = {"surface", "color", "closed",
                               "points",  "blend", NULL};

    if (!PyArg_ParseTupleAndKeywords(arg, kwargs, "O!OOO|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &closedobj, &points, &blend)) {
        return NULL; /* Exception already set. */
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    closed = PyObject_IsTrue(closedobj);

    if (-1 == closed) {
        return RAISE(PyExc_TypeError, "closed argument is invalid");
    }

    if (!PySequence_Check(points)) {
        return RAISE(PyExc_TypeError,
                     "points argument must be a sequence of number pairs");
    }

    length = PySequence_Length(points);

    if (length < 2) {
        return RAISE(PyExc_ValueError,
                     "points argument must contain 2 or more points");
    }

    xlist = PyMem_New(float, length);
    ylist = PyMem_New(float, length);

    if (NULL == xlist || NULL == ylist) {
        return RAISE(PyExc_MemoryError,
                     "cannot allocate memory to draw aalines");
    }

    for (loop = 0; loop < length; ++loop) {
        item = PySequence_GetItem(points, loop);
        result = pg_TwoFloatsFromObj(item, &x, &y);
        Py_DECREF(item);

        if (!result) {
            PyMem_Del(xlist);
            PyMem_Del(ylist);
            return RAISE(PyExc_TypeError, "points must be number pairs");
        }

        xlist[loop] = x;
        ylist[loop] = y;
        left = MIN(x, left);
        top = MIN(y, top);
        right = MAX(x, right);
        bottom = MAX(y, bottom);
    }

    if (!pgSurface_Lock(surfobj)) {
        PyMem_Del(xlist);
        PyMem_Del(ylist);
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    for (loop = 1; loop < length; ++loop) {
        pts[0] = xlist[loop - 1];
        pts[1] = ylist[loop - 1];
        pts[2] = xlist[loop];
        pts[3] = ylist[loop];
        clip_and_draw_aaline(surf, &surf->clip_rect, color, pts, blend);
    }
    if (closed && length > 2) {
        pts[0] = xlist[length - 1];
        pts[1] = ylist[length - 1];
        pts[2] = xlist[0];
        pts[3] = ylist[0];
        clip_and_draw_aaline(surf, &surf->clip_rect, color, pts, blend);
    }

    PyMem_Del(xlist);
    PyMem_Del(ylist);

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    /* Compute return rect. */
    return pgRect_New4((int)left, (int)top, (int)(right - left + 2),
                       (int)(bottom - top + 2));
}

/* Draws a series of lines on the given surface.
 *
 * Returns a Rect bounding the drawn area.
 */
static PyObject *
lines(PyObject *self, PyObject *arg, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *closedobj = NULL;
    PyObject *points = NULL, *item = NULL;
    SDL_Surface *surf = NULL;
    Uint32 color;
    Uint8 rgba[4];
    int pts[4];
    int x, y, closed, result;
    int top = INT_MAX, left = INT_MAX;
    int bottom = INT_MIN, right = INT_MIN;
    int *xlist = NULL, *ylist = NULL;
    int width = 1; /* Default width. */
    Py_ssize_t loop, length;
    static char *keywords[] = {"surface", "color", "closed",
                               "points",  "width", NULL};

    if (!PyArg_ParseTupleAndKeywords(arg, kwargs, "O!OOO|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &closedobj, &points, &width)) {
        return NULL; /* Exception already set. */
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    closed = PyObject_IsTrue(closedobj);

    if (-1 == closed) {
        return RAISE(PyExc_TypeError, "closed argument is invalid");
    }

    if (!PySequence_Check(points)) {
        return RAISE(PyExc_TypeError,
                     "points argument must be a sequence of number pairs");
    }

    length = PySequence_Length(points);

    if (length < 2) {
        return RAISE(PyExc_ValueError,
                     "points argument must contain 2 or more points");
    }

    xlist = PyMem_New(int, length);
    ylist = PyMem_New(int, length);

    if (NULL == xlist || NULL == ylist) {
        return RAISE(PyExc_MemoryError,
                     "cannot allocate memory to draw lines");
    }

    for (loop = 0; loop < length; ++loop) {
        item = PySequence_GetItem(points, loop);
        result = pg_TwoIntsFromObj(item, &x, &y);
        Py_DECREF(item);

        if (!result) {
            PyMem_Del(xlist);
            PyMem_Del(ylist);
            return RAISE(PyExc_TypeError, "points must be number pairs");
        }

        xlist[loop] = x;
        ylist[loop] = y;
        left = MIN(x, left);
        top = MIN(y, top);
        right = MAX(x, right);
        bottom = MAX(y, bottom);
    }

    if (width < 1) {
        PyMem_Del(xlist);
        PyMem_Del(ylist);
        return pgRect_New4(left, top, 0, 0);
    }

    if (!pgSurface_Lock(surfobj)) {
        PyMem_Del(xlist);
        PyMem_Del(ylist);
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    for (loop = 1; loop < length; ++loop) {
        pts[0] = xlist[loop - 1];
        pts[1] = ylist[loop - 1];
        pts[2] = xlist[loop];
        pts[3] = ylist[loop];
        clip_and_draw_line_width(surf, &surf->clip_rect, color, width, pts);
    }

    if (closed && length > 2) {
        pts[0] = xlist[length - 1];
        pts[1] = ylist[length - 1];
        pts[2] = xlist[0];
        pts[3] = ylist[0];
        clip_and_draw_line_width(surf, &surf->clip_rect, color, width, pts);
    }

    PyMem_Del(xlist);
    PyMem_Del(ylist);

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    /* Compute return rect. */
    return pgRect_New4(left, top, right - left + 1, bottom - top + 1);
}

static PyObject *
arc(PyObject *self, PyObject *arg, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *rectobj = NULL;
    GAME_Rect *rect = NULL, temp;
    SDL_Surface *surf = NULL;
    Uint8 rgba[4];
    Uint32 color;
    int loop, t, l, b, r;
    int width = 1; /* Default width. */
    double angle_start, angle_stop;
    static char *keywords[] = {"surface", "color", "rect", "start_angle",
                               "stop_angle", "width", NULL};

    if (!PyArg_ParseTupleAndKeywords(arg, kwargs, "O!OOdd|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &rectobj, &angle_start, &angle_stop,
                                     &width)) {
        return NULL; /* Exception already set. */
    }

    rect = pgRect_FromObject(rectobj, &temp);

    if (!rect) {
        return RAISE(PyExc_TypeError, "rect argument is invalid");
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    if (width < 0) {
        return RAISE(PyExc_ValueError, "negative width");
    }

    if (width > rect->w / 2 || width > rect->h / 2) {
        return RAISE(PyExc_ValueError, "width greater than arc radius");
    }

    if (angle_stop < angle_start) {
        // Angle is in radians
        angle_stop += 2 * M_PI;
    }

    if (!pgSurface_Lock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    width = MIN(width, MIN(rect->w, rect->h) / 2);

    for (loop = 0; loop < width; ++loop) {
        draw_arc(surf, rect->x + rect->w / 2, rect->y + rect->h / 2,
                 rect->w / 2 - loop, rect->h / 2 - loop, angle_start,
                 angle_stop, color);
    }

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    l = MAX(rect->x, surf->clip_rect.x);
    t = MAX(rect->y, surf->clip_rect.y);
    r = MIN(rect->x + rect->w, surf->clip_rect.x + surf->clip_rect.w);
    b = MIN(rect->y + rect->h, surf->clip_rect.y + surf->clip_rect.h);
    return pgRect_New4(l, t, MAX(r - l, 0), MAX(b - t, 0));
}

static PyObject *
ellipse(PyObject *self, PyObject *arg, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *rectobj = NULL;
    GAME_Rect *rect = NULL, temp;
    SDL_Surface *surf = NULL;
    Uint8 rgba[4];
    Uint32 color;
    int loop, t, l, b, r;
    int width = 0;  /* Default width. */
    static char *keywords[] = {"surface", "color", "rect", "width", NULL};

    if (!PyArg_ParseTupleAndKeywords(arg, kwargs, "O!OO|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &rectobj, &width)) {
        return NULL; /* Exception already set. */
    }

    rect = pgRect_FromObject(rectobj, &temp);

    if (!rect) {
        return RAISE(PyExc_TypeError, "rect argument is invalid");
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    if (width < 0) {
        return RAISE(PyExc_ValueError, "negative width");
    }

    if (width > rect->w / 2 || width > rect->h / 2) {
        return RAISE(PyExc_ValueError, "width greater than ellipse radius");
    }

    if (!pgSurface_Lock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    if (!width) {
        /* Draw a filled ellipse. */
        draw_ellipse(surf, rect->x + rect->w / 2, rect->y + rect->h / 2,
                     rect->w, rect->h, 1, color);
    }
    else {
        width = MIN(width, MIN(rect->w, rect->h) / 2);
        for (loop = 0; loop < width; ++loop) {
            draw_ellipse(surf, rect->x + rect->w / 2, rect->y + rect->h / 2,
                         rect->w - loop, rect->h - loop, 0, color);
        }
    }

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    l = MAX(rect->x, surf->clip_rect.x);
    t = MAX(rect->y, surf->clip_rect.y);
    r = MIN(rect->x + rect->w, surf->clip_rect.x + surf->clip_rect.w);
    b = MIN(rect->y + rect->h, surf->clip_rect.y + surf->clip_rect.h);
    return pgRect_New4(l, t, MAX(r - l, 0), MAX(b - t, 0));
}

static PyObject *
circle(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL;
    SDL_Surface *surf = NULL;
    Uint8 rgba[4];
    Uint32 color;
    PyObject *posobj;
    int posx, posy, radius, t, l, b, r;
    int width = 0; /* Default width. */
    static char *keywords[] = {"surface", "color", "center",
                               "radius",  "width", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!OOi|i", keywords,
                          &pgSurface_Type, &surfobj,
                          &colorobj,
                          &posobj,
                          &radius, &width))
        return NULL; /* Exception already set. */

    if (!pg_TwoIntsFromObj(posobj, &posx, &posy)) {
        PyErr_SetString(PyExc_TypeError,
                        "expected a pair of numbers");
        return 0;
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    if (radius < 0) {
        return RAISE(PyExc_ValueError, "negative radius");
    }

    if (width < 0) {
        return RAISE(PyExc_ValueError, "negative width");
    }

    if (width > radius) {
        return RAISE(PyExc_ValueError, "width greater than radius");
    }

    if (!pgSurface_Lock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    if (!width) {
        draw_ellipse(surf, posx, posy, radius * 2, radius * 2, 1, color);
    }
    else {
        int loop;

        for (loop = 0; loop < width; ++loop) {
            draw_ellipse(surf, posx, posy, 2 * (radius - loop),
                         2 * (radius - loop), 0, color);
            /* To avoid moiré pattern. Don't do an extra one on the outer
             * ellipse.  We draw another ellipse offset by a pixel, over
             * drawing the missed spots in the filled circle caused by which
             * pixels are filled.
             */
            // if (width > 1 && loop > 0)
            // removed due to: 'Gaps in circle for width greater than 1 #736'
            draw_ellipse(surf, posx + 1, posy, 2 * (radius - loop),
                         2 * (radius - loop), 0, color);
        }
    }

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    l = MAX(posx - radius, surf->clip_rect.x);
    t = MAX(posy - radius, surf->clip_rect.y);
    r = MIN(posx + radius, surf->clip_rect.x + surf->clip_rect.w);
    b = MIN(posy + radius, surf->clip_rect.y + surf->clip_rect.h);
    return pgRect_New4(l, t, MAX(r - l, 0), MAX(b - t, 0));
}

static PyObject *
polygon(PyObject *self, PyObject *arg, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *points = NULL, *item = NULL;
    SDL_Surface *surf = NULL;
    Uint8 rgba[4];
    Uint32 color;
    int *xlist = NULL, *ylist = NULL;
    int width = 0; /* Default width. */
    int top = INT_MAX, left = INT_MAX;
    int bottom = INT_MIN, right = INT_MIN;
    int x, y, result;
    Py_ssize_t loop, length;
    static char *keywords[] = {"surface", "color", "points", "width", NULL};

    if (!PyArg_ParseTupleAndKeywords(arg, kwargs, "O!OO|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &points, &width)) {
        return NULL; /* Exception already set. */
    }

    if (width) {
        PyObject *ret = NULL;
        PyObject *args =
            Py_BuildValue("(OOiOi)", surfobj, colorobj, 1, points, width);

        if (!args) {
            return NULL; /* Exception already set. */
        }

        ret = lines(NULL, args, NULL);
        Py_DECREF(args);
        return ret;
    }

    surf = pgSurface_AsSurface(surfobj);

    if (surf->format->BytesPerPixel <= 0 || surf->format->BytesPerPixel > 4) {
        return PyErr_Format(PyExc_ValueError,
                            "unsupported surface bit depth (%d) for drawing",
                            surf->format->BytesPerPixel);
    }

    CHECK_LOAD_COLOR(colorobj)

    if (!PySequence_Check(points)) {
        return RAISE(PyExc_TypeError,
                     "points argument must be a sequence of number pairs");
    }

    length = PySequence_Length(points);

    if (length < 3) {
        return RAISE(PyExc_ValueError,
                     "points argument must contain more than 2 points");
    }

    xlist = PyMem_New(int, length);
    ylist = PyMem_New(int, length);

    if (NULL == xlist || NULL == ylist) {
        return RAISE(PyExc_MemoryError,
                     "cannot allocate memory to draw polygon");
    }

    for (loop = 0; loop < length; ++loop) {
        item = PySequence_GetItem(points, loop);
        result = pg_TwoIntsFromObj(item, &x, &y);
        Py_DECREF(item);

        if (!result) {
            PyMem_Del(xlist);
            PyMem_Del(ylist);
            return RAISE(PyExc_TypeError, "points must be number pairs");
        }

        xlist[loop] = x;
        ylist[loop] = y;
        left = MIN(x, left);
        top = MIN(y, top);
        right = MAX(x, right);
        bottom = MAX(y, bottom);
    }

    if (!pgSurface_Lock(surfobj)) {
        PyMem_Del(xlist);
        PyMem_Del(ylist);
        return RAISE(PyExc_RuntimeError, "error locking surface");
    }

    draw_fillpoly(surf, xlist, ylist, length, color);
    PyMem_Del(xlist);
    PyMem_Del(ylist);

    if (!pgSurface_Unlock(surfobj)) {
        return RAISE(PyExc_RuntimeError, "error unlocking surface");
    }

    left = MAX(left, surf->clip_rect.x);
    top = MAX(top, surf->clip_rect.y);
    right = MIN(right, surf->clip_rect.x + surf->clip_rect.w);
    bottom = MIN(bottom, surf->clip_rect.y + surf->clip_rect.h);
    return pgRect_New4(left, top, right - left + 1, bottom - top + 1);
}

static PyObject *
rect(PyObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *surfobj = NULL, *colorobj = NULL, *rectobj = NULL;
    PyObject *points = NULL, *poly_args = NULL, *ret = NULL;
    GAME_Rect *rect = NULL, temp;
    int t, l, b, r;
    int width = 0; /* Default width. */
    static char *keywords[] = {"surface", "color", "rect", "width", NULL};

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!OO|i", keywords,
                                     &pgSurface_Type, &surfobj, &colorobj,
                                     &rectobj, &width)) {
        return NULL; /* Exception already set. */
    }

    if (!(rect = pgRect_FromObject(rectobj, &temp))) {
        return RAISE(PyExc_TypeError, "rect argument is invalid");
    }

    l = rect->x;
    r = rect->x + rect->w - 1;
    t = rect->y;
    b = rect->y + rect->h - 1;

    points = Py_BuildValue("((ii)(ii)(ii)(ii))", l, t, r, t, r, b, l, b);
    poly_args = Py_BuildValue("(OONi)", surfobj, colorobj, points, width);
    if (NULL == poly_args) {
        return NULL; /* Exception already set. */
    }

    ret = polygon(NULL, poly_args, NULL);
    Py_DECREF(poly_args);
    return ret;
}

/*internal drawing tools*/

static int
clip_and_draw_aaline(SDL_Surface *surf, SDL_Rect *rect, Uint32 color,
                     float *pts, int blend)
{
    if (!clip_aaline(pts, rect->x, rect->y, rect->x + rect->w - 1,
                     rect->y + rect->h - 1))
        return 0;

    draw_aaline(surf, color, pts[0], pts[1], pts[2], pts[3], blend);
    return 1;
}

static int
clip_and_draw_line(SDL_Surface *surf, SDL_Rect *rect, Uint32 color, int *pts)
{
    if (!clipline(pts, rect->x, rect->y, rect->x + rect->w - 1,
                  rect->y + rect->h - 1))
        return 0;
    if (pts[1] == pts[3])
        drawhorzline(surf, color, pts[0], pts[1], pts[2]);
    else if (pts[0] == pts[2])
        drawvertline(surf, color, pts[0], pts[1], pts[3]);
    else
        drawline(surf, color, pts[0], pts[1], pts[2], pts[3]);
    return 1;
}

/* This is an internal helper function.
 *
 * This function draws a line that is clipped by the given rect. To draw thick
 * lines (width > 1), multiple parallel lines are drawn.
 *
 * Params:
 *     surf - pointer to surface to draw on
 *     rect - pointer to clipping rect
 *     color - color of line to draw
 *     width - width/thickness of line to draw (expected to be > 0)
 *     pts - array of 4 points which are the endpoints of the line to
 *         draw: {x0, y0, x1, y1}
 *
 * Returns:
 *     int - 1 indicates that something was drawn on the surface
 *           0 indicates that nothing was drawn
 *
 *     If something was drawn, the 'pts' parameter is changed to contain the
 *     min/max x/y values of the pixels changed: {xmin, ymin, xmax, ymax}.
 *     These points represent the minimum bounding box of the affected area.
 *     The top left corner is xmin, ymin and the bottom right corner is
 *     xmax, ymax.
 */
static int
clip_and_draw_line_width(SDL_Surface *surf, SDL_Rect *rect, Uint32 color,
                         int width, int *pts)
{
    int loop;
    int xinc = 0, yinc = 0;
    int newpts[4];
    int range[4]; /* {xmin, ymin, xmax, ymax} */
    int anydrawn = 0;

    /* Decide which direction to grow (width/thickness). */
    if (abs(pts[0] - pts[2]) > abs(pts[1] - pts[3])) {
        /* The line's thickness will be in the y direction. The left/right
         * ends of the line will be flat. */
        yinc = 1;
    }
    else {
        /* The line's thickness will be in the x direction. The top/bottom
         * ends of the line will be flat. */
        xinc = 1;
    }

    memcpy(newpts, pts, sizeof(int) * 4);

    /* Draw the line or center line if width > 1. */
    if (clip_and_draw_line(surf, rect, color, newpts)) {
        anydrawn = 1;

        if (newpts[0] > newpts[2]) {
            range[0] = newpts[2]; /* xmin */
            range[2] = newpts[0]; /* xmax */
        }
        else {
            range[0] = newpts[0]; /* xmin */
            range[2] = newpts[2]; /* xmax */
        }

        if (newpts[1] > newpts[3]) {
            range[1] = newpts[3]; /* ymin */
            range[3] = newpts[1]; /* ymax */
        }
        else {
            range[1] = newpts[1]; /* ymin */
            range[3] = newpts[3]; /* ymax */
        }
    }
    else {
        range[0] = range[1] = INT_MAX; /* Default to big values for min. */
        range[2] = range[3] = INT_MIN; /* Default to small values for max. */
    }

    for (loop = 1; loop < width; loop += 2) {
        newpts[0] = pts[0] + xinc * (loop / 2 + 1);
        newpts[1] = pts[1] + yinc * (loop / 2 + 1);
        newpts[2] = pts[2] + xinc * (loop / 2 + 1);
        newpts[3] = pts[3] + yinc * (loop / 2 + 1);

        /* Draw to the right and/or under the center line. */
        if (clip_and_draw_line(surf, rect, color, newpts)) {
            anydrawn = 1;
            range[0] = MIN(range[0], MIN(newpts[0], newpts[2]));
            range[1] = MIN(range[1], MIN(newpts[1], newpts[3]));
            range[2] = MAX(range[2], MAX(newpts[0], newpts[2]));
            range[3] = MAX(range[3], MAX(newpts[1], newpts[3]));
        }

        if (loop + 1 < width) {
            newpts[0] = pts[0] - xinc * (loop / 2 + 1);
            newpts[1] = pts[1] - yinc * (loop / 2 + 1);
            newpts[2] = pts[2] - xinc * (loop / 2 + 1);
            newpts[3] = pts[3] - yinc * (loop / 2 + 1);

            /* Draw to the left and/or above the center line. */
            if (clip_and_draw_line(surf, rect, color, newpts)) {
                anydrawn = 1;
                range[0] = MIN(range[0], MIN(newpts[0], newpts[2]));
                range[1] = MIN(range[1], MIN(newpts[1], newpts[3]));
                range[2] = MAX(range[2], MAX(newpts[0], newpts[2]));
                range[3] = MAX(range[3], MAX(newpts[1], newpts[3]));
            }
        }
    }

    if (anydrawn) {
        memcpy(pts, range, sizeof(int) * 4);
    }

    return anydrawn;
}

#define SWAP(a, b, tmp) \
    tmp = b;            \
    b = a;              \
    a = tmp;

/*this line clipping based heavily off of code from
http://www.ncsa.uiuc.edu/Vis/Graphics/src/clipCohSuth.c */
#define LEFT_EDGE 0x1
#define RIGHT_EDGE 0x2
#define BOTTOM_EDGE 0x4
#define TOP_EDGE 0x8
#define INSIDE(a) (!a)
#define REJECT(a, b) (a & b)
#define ACCEPT(a, b) (!(a | b))

static int
encode(int x, int y, int left, int top, int right, int bottom)
{
    int code = 0;
    if (x < left)
        code |= LEFT_EDGE;
    if (x > right)
        code |= RIGHT_EDGE;
    if (y < top)
        code |= TOP_EDGE;
    if (y > bottom)
        code |= BOTTOM_EDGE;
    return code;
}

static int
encodeFloat(float x, float y, int left, int top, int right, int bottom)
{
    int code = 0;
    if (x < left)
        code |= LEFT_EDGE;
    if (x > right)
        code |= RIGHT_EDGE;
    if (y < top)
        code |= TOP_EDGE;
    if (y > bottom)
        code |= BOTTOM_EDGE;
    return code;
}

static int
clip_aaline(float *segment, int left, int top, int right, int bottom)
{
    /*
     * Algorithm to calculate the clipped anti-aliased line.
     *
     * We write the coordinates of the part of the line
     * segment within the bounding box defined
     * by (left, top, right, bottom) into the "segment" array.
     * Returns 0 if we don't have to draw anything, eg if the
     * segment = [from_x, from_y, to_x, to_y]
     * doesn't cross the bounding box.
     */
    float x1 = segment[0];
    float y1 = segment[1];
    float x2 = segment[2];
    float y2 = segment[3];
    int code1, code2;
    float swaptmp;
    int intswaptmp;
    float m; /*slope*/

    while (1) {
        code1 = encodeFloat(x1, y1, left, top, right, bottom);
        code2 = encodeFloat(x2, y2, left, top, right, bottom);
        if (ACCEPT(code1, code2)) {
            segment[0] = x1;
            segment[1] = y1;
            segment[2] = x2;
            segment[3] = y2;
            return 1;
        }
        else if (REJECT(code1, code2)) {
            return 0;
        }
        else {
            if (INSIDE(code1)) {
                SWAP(x1, x2, swaptmp)
                SWAP(y1, y2, swaptmp)
                SWAP(code1, code2, intswaptmp)
            }
            if (x2 != x1)
                m = (y2 - y1) / (x2 - x1);
            else
                m = 1.0f;
            if (code1 & LEFT_EDGE) {
                y1 += ((float)left - x1) * m;
                x1 = (float)left;
            }
            else if (code1 & RIGHT_EDGE) {
                y1 += ((float)right - x1) * m;
                x1 = (float)right;
            }
            else if (code1 & BOTTOM_EDGE) {
                if (x2 != x1)
                    x1 += ((float)bottom - y1) / m;
                y1 = (float)bottom;
            }
            else if (code1 & TOP_EDGE) {
                if (x2 != x1)
                    x1 += ((float)top - y1) / m;
                y1 = (float)top;
            }
        }
    }
}

static int
clipline(int *segment, int left, int top, int right, int bottom)
{
    /*
     * Algorithm to calculate the clipped line.
     * It's like clip_aaline, but for integer coordinate endpoints.
     */
    int x1 = segment[0];
    int y1 = segment[1];
    int x2 = segment[2];
    int y2 = segment[3];
    int code1, code2;
    int swaptmp;
    float m; /*slope*/

    while (1) {
        code1 = encode(x1, y1, left, top, right, bottom);
        code2 = encode(x2, y2, left, top, right, bottom);
        if (ACCEPT(code1, code2)) {
            segment[0] = x1;
            segment[1] = y1;
            segment[2] = x2;
            segment[3] = y2;
            return 1;
        }
        else if (REJECT(code1, code2))
            return 0;
        else {
            if (INSIDE(code1)) {
                SWAP(x1, x2, swaptmp)
                SWAP(y1, y2, swaptmp)
                SWAP(code1, code2, swaptmp)
            }
            if (x2 != x1)
                m = (y2 - y1) / (float)(x2 - x1);
            else
                m = 1.0f;
            if (code1 & LEFT_EDGE) {
                y1 += (int)((left - x1) * m);
                x1 = left;
            }
            else if (code1 & RIGHT_EDGE) {
                y1 += (int)((right - x1) * m);
                x1 = right;
            }
            else if (code1 & BOTTOM_EDGE) {
                if (x2 != x1)
                    x1 += (int)((bottom - y1) / m);
                y1 = bottom;
            }
            else if (code1 & TOP_EDGE) {
                if (x2 != x1)
                    x1 += (int)((top - y1) / m);
                y1 = top;
            }
        }
    }
}

static int
set_at(SDL_Surface *surf, int x, int y, Uint32 color)
{
    SDL_PixelFormat *format = surf->format;
    Uint8 *pixels = (Uint8 *)surf->pixels;
    Uint8 *byte_buf, rgb[4];

    if (x < surf->clip_rect.x || x >= surf->clip_rect.x + surf->clip_rect.w ||
        y < surf->clip_rect.y || y >= surf->clip_rect.y + surf->clip_rect.h)
        return 0;

    switch (format->BytesPerPixel) {
        case 1:
            *((Uint8 *)pixels + y * surf->pitch + x) = (Uint8)color;
            break;
        case 2:
            *((Uint16 *)(pixels + y * surf->pitch) + x) = (Uint16)color;
            break;
        case 4:
            *((Uint32 *)(pixels + y * surf->pitch) + x) = color;
            /*              *((Uint32*)(pixels + y * surf->pitch) + x) =
                            ~(*((Uint32*)(pixels + y * surf->pitch) + x)) * 31;
            */
            break;
        default: /*case 3:*/
            SDL_GetRGB(color, format, rgb, rgb + 1, rgb + 2);
            byte_buf = (Uint8 *)(pixels + y * surf->pitch) + x * 3;
#if (SDL_BYTEORDER == SDL_LIL_ENDIAN)
            *(byte_buf + (format->Rshift >> 3)) = rgb[0];
            *(byte_buf + (format->Gshift >> 3)) = rgb[1];
            *(byte_buf + (format->Bshift >> 3)) = rgb[2];
#else
            *(byte_buf + 2 - (format->Rshift >> 3)) = rgb[0];
            *(byte_buf + 2 - (format->Gshift >> 3)) = rgb[1];
            *(byte_buf + 2 - (format->Bshift >> 3)) = rgb[2];
#endif
            break;
    }
    return 1;
}

static Uint32
get_pixel_32(Uint8 *pixels, SDL_PixelFormat *format)
{
    switch (format->BytesPerPixel) {
        case 4:
            return *((Uint32 *)pixels);
        case 3:
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            return *pixels | *(pixels+1) << 8 | *(pixels+2) << 16;
#else
            return *pixels << 16 | *(pixels + 1) << 8 | *(pixels + 2);
#endif
        case 2:
            return *((Uint16 *)pixels);
        case 1:
            return *pixels;
    }
    return 0;
}

static void
set_pixel_32(Uint8 *pixels, SDL_PixelFormat *format, Uint32 pixel)
{
    switch (format->BytesPerPixel) {
        case 4:
            *(Uint32 *)pixels = pixel;
            break;
        case 3:
#if SDL_BYTEORDER == SDL_LIL_ENDIAN
            *(Uint16*)pixels = pixel;
            pixels[2] = pixel >> 16;
#else
            pixels[2] = pixel;
            pixels[1] = pixel >> 8;
            pixels[0] = pixel >> 16;
#endif
            break;
        case 2:
            *(Uint16 *)pixels = pixel;
            break;
        case 1:
            *pixels = pixel;
            break;
    }
}

static void
draw_pixel_blended_32(Uint8 *pixels, Uint8 *colors, float br,
                      SDL_PixelFormat *format)
{
    Uint8 pixel32[4];

    SDL_GetRGBA(get_pixel_32(pixels, format), format, &pixel32[0], &pixel32[1],
                &pixel32[2], &pixel32[3]);

    *(Uint32 *)pixel32 =
        SDL_MapRGBA(format, (Uint8)(br * colors[0] + (1 - br) * pixel32[0]),
                    (Uint8)(br * colors[1] + (1 - br) * pixel32[1]),
                    (Uint8)(br * colors[2] + (1 - br) * pixel32[2]),
                    (Uint8)(br * colors[3] + (1 - br) * pixel32[3]));

    set_pixel_32(pixels, format, *(Uint32 *)pixel32);
}

#define DRAWPIX32(pixels, colorptr, br, blend)                                \
    {                                                                         \
        if (blend)                                                            \
            draw_pixel_blended_32(pixels, colorptr, br, surf->format);        \
        else {                                                                \
            set_pixel_32(pixels, surf->format,                                \
                         SDL_MapRGBA(surf->format, (Uint8)(br * colorptr[0]), \
                                     (Uint8)(br * colorptr[1]),               \
                                     (Uint8)(br * colorptr[2]),               \
                                     (Uint8)(br * colorptr[3])));             \
        }                                                                     \
    }

/* Adapted from http://freespace.virgin.net/hugo.elias/graphics/x_wuline.htm */
static void
draw_aaline(SDL_Surface *surf, Uint32 color, float from_x, float from_y, float to_x,
           float to_y, int blend)
{
    float slope, dx, dy;
    float xgap, ygap, pt_x, pt_y, xf, yf;
    float brightness1, brightness2;
    float swaptmp;
    int x, y, ifrom_x, ito_x, ifrom_y, ito_y;
    int pixx, pixy;
    Uint8 colorptr[4];
    SDL_Rect *rect = &surf->clip_rect;
    int max_x = rect->x + rect->w - 1;
    int max_y = rect->y + rect->h - 1;

    Uint8 *pixel;
    Uint8 *surf_pmap = (Uint8 *)surf->pixels;
    SDL_GetRGBA(color, surf->format, &colorptr[0], &colorptr[1], &colorptr[2],
                &colorptr[3]);
    if (!blend)
        colorptr[3] = 255;

    pixx = surf->format->BytesPerPixel;
    pixy = surf->pitch;

    dx = to_x - from_x;
    dy = to_y - from_y;

    if (dx == 0 && dy == 0) {
        /* Single point. Due to the nature of the aaline clipping, this
         * is less exact than the normal line. */
        set_at(surf, (int)truncf(from_x), (int)truncf(from_y), color);
        return;
    }

    if (fabs(dx) > fabs(dy)) {
        /* Lines tending to be more horizontal (run > rise) handled here. */
        if (from_x > to_x) {
            SWAP(from_x, to_x, swaptmp)
            SWAP(from_y, to_y, swaptmp)
            dx = -dx;
            dy = -dy;
        }
        slope = dy / dx;

        // 1. Draw start of the segment
        /* This makes more sense than truncf(from_x + 0.5f) */
        pt_x = truncf(from_x) + 0.5f;
        pt_y = from_y + slope * (pt_x - from_x);
        xgap = INVFRAC_FLT(from_x);
        ifrom_x = (int)pt_x;
        ifrom_y = (int)pt_y;
        yf = pt_y + slope;
        brightness1 = INVFRAC_FLT(pt_y) * xgap;

        pixel = surf_pmap + pixx * ifrom_x + pixy * ifrom_y;
        DRAWPIX32(pixel, colorptr, brightness1, blend)

        /* Skip if ifrom_y+1 is not on the surface. */
        if (ifrom_y < max_y) {
            brightness2 = FRAC_FLT(pt_y) * xgap;
            pixel += pixy;
            DRAWPIX32(pixel, colorptr, brightness2, blend)
        }

        // 2. Draw end of the segment
        pt_x = truncf(to_x) + 0.5f;
        pt_y = to_y + slope * (pt_x - to_x);
        xgap = INVFRAC_FLT(to_x);
        ito_x = (int)pt_x;
        ito_y = (int)pt_y;
        brightness1 = INVFRAC_FLT(pt_y) * xgap;

        pixel = surf_pmap + pixx * ito_x + pixy * ito_y;
        DRAWPIX32(pixel, colorptr, brightness1, blend)

        /* Skip if ito_y+1 is not on the surface. */
        if (ito_y < max_y) {
            brightness2 = FRAC_FLT(pt_y) * xgap;
            pixel += pixy;
            DRAWPIX32(pixel, colorptr, brightness2, blend)
        }

        // 3. loop for other points
        for (x = ifrom_x + 1; x < ito_x; ++x) {
            brightness1 = INVFRAC_FLT(yf);
            y = (int)yf;

            pixel = surf_pmap + pixx * x + pixy * y;
            DRAWPIX32(pixel, colorptr, brightness1, blend)

            /* Skip if y+1 is not on the surface. */
            if (y < max_y) {
                brightness2 = FRAC_FLT(yf);
                pixel += pixy;
                DRAWPIX32(pixel, colorptr, brightness2, blend)
            }
            yf += slope;
        }
    }
    else {
        /* Lines tending to be more vertical (rise >= run) handled here. */
        if (from_y > to_y) {
            SWAP(from_x, to_x, swaptmp)
            SWAP(from_y, to_y, swaptmp)
            dx = -dx;
            dy = -dy;
        }
        slope = dx / dy;

        // 1. Draw start of the segment
        /* This makes more sense than truncf(from_x + 0.5f) */
        pt_y = truncf(from_y) + 0.5f;
        pt_x = from_x + slope * (pt_y - from_y);
        ygap = INVFRAC_FLT(from_y);
        ifrom_y = (int)pt_y;
        ifrom_x = (int)pt_x;
        xf = pt_x + slope;
        brightness1 = INVFRAC_FLT(pt_x) * ygap;

        pixel = surf_pmap + pixx * ifrom_x + pixy * ifrom_y;
        DRAWPIX32(pixel, colorptr, brightness1, blend)

        /* Skip if ifrom_x+1 is not on the surface. */
        if (ifrom_x < max_x) {
            brightness2 = FRAC_FLT(pt_x) * ygap;
            pixel += pixx;
            DRAWPIX32(pixel, colorptr, brightness2, blend)
        }

        // 2. Draw end of the segment
        pt_y = truncf(to_y) + 0.5f;
        pt_x = to_x + slope * (pt_y - to_y);
        ygap = INVFRAC_FLT(to_y);
        ito_y = (int)pt_y;
        ito_x = (int)pt_x;
        brightness1 = INVFRAC_FLT(pt_x) * ygap;

        pixel = surf_pmap + pixx * ito_x + pixy * ito_y;
        DRAWPIX32(pixel, colorptr, brightness1, blend)

        /* Skip if ito_x+1 is not on the surface. */
        if (ito_x < max_x) {
            brightness2 = FRAC_FLT(pt_x) * ygap;
            pixel += pixx;
            DRAWPIX32(pixel, colorptr, brightness2, blend)
        }

        // 3. loop for other points
        for (y = ifrom_y + 1; y < ito_y; ++y) {
            x = (int)xf;
            brightness1 = INVFRAC_FLT(xf);

            pixel = surf_pmap + pixx * x + pixy * y;
            DRAWPIX32(pixel, colorptr, brightness1, blend)

            /* Skip if x+1 is not on the surface. */
            if (x < max_x) {
                brightness2 = FRAC_FLT(xf);
                pixel += pixx;
                DRAWPIX32(pixel, colorptr, brightness2, blend)
            }
            xf += slope;
        }
    }
}

/*here's my sdl'ized version of bresenham*/
static void
drawline(SDL_Surface *surf, Uint32 color, int x1, int y1, int x2, int y2)
{
    int deltax, deltay, signx, signy;
    int pixx, pixy;
    int x = 0, y = 0;
    int swaptmp;
    Uint8 *pixel;
    Uint8 *colorptr;

    deltax = x2 - x1;
    deltay = y2 - y1;
    signx = (deltax < 0) ? -1 : 1;
    signy = (deltay < 0) ? -1 : 1;
    deltax = signx * deltax + 1;
    deltay = signy * deltay + 1;

    pixx = surf->format->BytesPerPixel;
    pixy = surf->pitch;
    pixel = ((Uint8 *)surf->pixels) + pixx * x1 + pixy * y1;

    pixx *= signx;
    pixy *= signy;
    if (deltax < deltay) /*swap axis if rise > run*/
    {
        SWAP(deltax, deltay, swaptmp)
        SWAP(pixx, pixy, swaptmp)
    }

    switch (surf->format->BytesPerPixel) {
        case 1:
            for (; x < deltax; x++, pixel += pixx) {
                *pixel = (Uint8)color;
                y += deltay;
                if (y >= deltax) {
                    y -= deltax;
                    pixel += pixy;
                }
            }
            break;
        case 2:
            for (; x < deltax; x++, pixel += pixx) {
                *(Uint16 *)pixel = (Uint16)color;
                y += deltay;
                if (y >= deltax) {
                    y -= deltax;
                    pixel += pixy;
                }
            }
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                color <<= 8;
            colorptr = (Uint8 *)&color;
            for (; x < deltax; x++, pixel += pixx) {
                pixel[0] = colorptr[0];
                pixel[1] = colorptr[1];
                pixel[2] = colorptr[2];
                y += deltay;
                if (y >= deltax) {
                    y -= deltax;
                    pixel += pixy;
                }
            }
            break;
        default: /*case 4*/
            for (; x < deltax; x++, pixel += pixx) {
                *(Uint32 *)pixel = (Uint32)color;
                y += deltay;
                if (y >= deltax) {
                    y -= deltax;
                    pixel += pixy;
                }
            }
            break;
    }
}

static void
drawhorzline(SDL_Surface *surf, Uint32 color, int x1, int y1, int x2)
{
    Uint8 *pixel, *end;
    Uint8 *colorptr;

    if (x1 == x2) {
        set_at(surf, x1, y1, color);
        return;
    }

    pixel = ((Uint8 *)surf->pixels) + surf->pitch * y1;
    if (x1 < x2) {
        end = pixel + x2 * surf->format->BytesPerPixel;
        pixel += x1 * surf->format->BytesPerPixel;
    }
    else {
        end = pixel + x1 * surf->format->BytesPerPixel;
        pixel += x2 * surf->format->BytesPerPixel;
    }
    switch (surf->format->BytesPerPixel) {
        case 1:
            for (; pixel <= end; ++pixel) {
                *pixel = (Uint8)color;
            }
            break;
        case 2:
            for (; pixel <= end; pixel += 2) {
                *(Uint16 *)pixel = (Uint16)color;
            }
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                color <<= 8;
            colorptr = (Uint8 *)&color;
            for (; pixel <= end; pixel += 3) {
                pixel[0] = colorptr[0];
                pixel[1] = colorptr[1];
                pixel[2] = colorptr[2];
            }
            break;
        default: /*case 4*/
            for (; pixel <= end; pixel += 4) {
                *(Uint32 *)pixel = color;
            }
            break;
    }
}

static void
drawhorzlineclip(SDL_Surface *surf, Uint32 color, int x1, int y1, int x2)
{
    if (y1 < surf->clip_rect.y || y1 >= surf->clip_rect.y + surf->clip_rect.h)
        return;

    if (x2 < x1) {
        int temp = x1;
        x1 = x2;
        x2 = temp;
    }

    x1 = MAX(x1, surf->clip_rect.x);
    x2 = MIN(x2, surf->clip_rect.x + surf->clip_rect.w - 1);

    if (x2 < surf->clip_rect.x || x1 >= surf->clip_rect.x + surf->clip_rect.w)
        return;

    drawhorzline(surf, color, x1, y1, x2);
}

static void
drawvertline(SDL_Surface *surf, Uint32 color, int x1, int y1, int y2)
{
    Uint8 *pixel, *end;
    Uint8 *colorptr;
    Uint32 pitch = surf->pitch;

    if (y1 == y2) {
        set_at(surf, x1, y1, color);
        return;
    }

    pixel = ((Uint8 *)surf->pixels) + x1 * surf->format->BytesPerPixel;
    if (y1 < y2) {
        end = pixel + surf->pitch * y2;
        pixel += surf->pitch * y1;
    }
    else {
        end = pixel + surf->pitch * y1;
        pixel += surf->pitch * y2;
    }

    switch (surf->format->BytesPerPixel) {
        case 1:
            for (; pixel <= end; pixel += pitch) {
                *pixel = (Uint8)color;
            }
            break;
        case 2:
            for (; pixel <= end; pixel += pitch) {
                *(Uint16 *)pixel = (Uint16)color;
            }
            break;
        case 3:
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                color <<= 8;
            colorptr = (Uint8 *)&color;
            for (; pixel <= end; pixel += pitch) {
                pixel[0] = colorptr[0];
                pixel[1] = colorptr[1];
                pixel[2] = colorptr[2];
            }
            break;
        default: /*case 4*/
            for (; pixel <= end; pixel += pitch) {
                *(Uint32 *)pixel = color;
            }
            break;
    }
}

static void
drawvertlineclip(SDL_Surface *surf, Uint32 color, int x1, int y1, int y2)
{
    if (x1 < surf->clip_rect.x || x1 >= surf->clip_rect.x + surf->clip_rect.w)
        return;

    if (y2 < y1) {
        int temp = y1;
        y1 = y2;
        y2 = temp;
    }

    y1 = MAX(y1, surf->clip_rect.y);
    y2 = MIN(y2, surf->clip_rect.y + surf->clip_rect.h - 1);

    if (y2 < surf->clip_rect.y || y1 >= surf->clip_rect.y + surf->clip_rect.h)
        return;

    drawvertline(surf, color, x1, y1, y2);
}

static void
draw_arc(SDL_Surface *dst, int x, int y, int radius1, int radius2,
         double angle_start, double angle_stop, Uint32 color)
{
    double aStep;  // Angle Step (rad)
    double a;      // Current Angle (rad)
    int x_last, x_next, y_last, y_next;

    // Angle step in rad
    if (radius1 < radius2) {
        if (radius1 < 1.0e-4) {
            aStep = 1.0;
        }
        else {
            aStep = asin(2.0 / radius1);
        }
    }
    else {
        if (radius2 < 1.0e-4) {
            aStep = 1.0;
        }
        else {
            aStep = asin(2.0 / radius2);
        }
    }

    if (aStep < 0.05) {
        aStep = 0.05;
    }

    x_last = (int)(x + cos(angle_start) * radius1);
    y_last = (int)(y - sin(angle_start) * radius2);
    for (a = angle_start + aStep; a <= angle_stop; a += aStep) {
        int points[4];
        x_next = (int)(x + cos(a) * radius1);
        y_next = (int)(y - sin(a) * radius2);
        points[0] = x_last;
        points[1] = y_last;
        points[2] = x_next;
        points[3] = y_next;
        clip_and_draw_line(dst, &dst->clip_rect, color, points);
        x_last = x_next;
        y_last = y_next;
    }
}

static void
draw_ellipse(SDL_Surface *dst, int x, int y, int width, int height, int solid,
             Uint32 color)
{
    int ix, iy;
    int h, i, j, k;
    int oh, oi, oj, ok;
    int xoff = (width & 1) ^ 1;
    int yoff = (height & 1) ^ 1;
    int rx = (width >> 1);
    int ry = (height >> 1);

    /* Special case: draw a single pixel */
    if (rx == 0 && ry == 0) {
        set_at(dst, x, y, color);
        return;
    }

    /* Special case: draw a vertical line */
    if (rx == 0) {
        drawvertlineclip(dst, color, x, (Sint16)(y - ry),
                         (Sint16)(y + ry + (height & 1)));
        return;
    }

    /* Special case: draw a horizontal line */
    if (ry == 0) {
        drawhorzlineclip(dst, color, (Sint16)(x - rx), y,
                         (Sint16)(x + rx + (width & 1)));
        return;
    }

    /* Adjust ry for the rest of the ellipses (non-special cases). */
    ry += (solid & 1) - yoff;

    /* Init vars */
    oh = oi = oj = ok = 0xFFFF;

    /* Draw */
    if (rx >= ry) {
        ix = 0;
        iy = rx * 64;

        do {
            h = (ix + 8) >> 6;
            i = (iy + 8) >> 6;
            j = (h * ry) / rx;
            k = (i * ry) / rx;
            if (((ok != k) && (oj != k) && (k < ry)) || !solid) {
                if (solid) {
                    drawhorzlineclip(dst, color, x - h, y - k - yoff,
                                     x + h - xoff);
                    drawhorzlineclip(dst, color, x - h, y + k, x + h - xoff);
                }
                else {
                    set_at(dst, x - h, y - k - yoff, color);
                    set_at(dst, x + h - xoff, y - k - yoff, color);
                    set_at(dst, x - h, y + k, color);
                    set_at(dst, x + h - xoff, y + k, color);
                }
                ok = k;
            }
            if (((oj != j) && (ok != j) && (k != j)) || !solid) {
                if (solid) {
                    drawhorzlineclip(dst, color, x - i, y + j, x + i - xoff);
                    drawhorzlineclip(dst, color, x - i, y - j - yoff,
                                     x + i - xoff);
                }
                else {
                    set_at(dst, x - i, y + j, color);
                    set_at(dst, x + i - xoff, y + j, color);
                    set_at(dst, x - i, y - j - yoff, color);
                    set_at(dst, x + i - xoff, y - j - yoff, color);
                }
                oj = j;
            }
            ix = ix + iy / rx;
            iy = iy - ix / rx;

        } while (i > h);
    }
    else {
        ix = 0;
        iy = ry * 64;

        do {
            h = (ix + 8) >> 6;
            i = (iy + 8) >> 6;
            j = (h * rx) / ry;
            k = (i * rx) / ry;

            if (((oi != i) && (oh != i) && (i < ry)) || !solid) {
                if (solid) {
                    drawhorzlineclip(dst, color, x - j, y + i, x + j - xoff);
                    drawhorzlineclip(dst, color, x - j, y - i - yoff,
                                     x + j - xoff);
                }
                else {
                    set_at(dst, x - j, y + i, color);
                    set_at(dst, x + j - xoff, y + i, color);
                    set_at(dst, x - j, y - i - yoff, color);
                    set_at(dst, x + j - xoff, y - i - yoff, color);
                }
                oi = i;
            }
            if (((oh != h) && (oi != h) && (i != h)) || !solid) {
                if (solid) {
                    drawhorzlineclip(dst, color, x - k, y + h, x + k - xoff);
                    drawhorzlineclip(dst, color, x - k, y - h - yoff,
                                     x + k - xoff);
                }
                else {
                    set_at(dst, x - k, y + h, color);
                    set_at(dst, x + k - xoff, y + h, color);
                    set_at(dst, x - k, y - h - yoff, color);
                    set_at(dst, x + k - xoff, y - h - yoff, color);
                }
                oh = h;
            }

            ix = ix + iy / ry;
            iy = iy - ix / ry;

        } while (i > h);
    }
}

static int
compare_int(const void *a, const void *b)
{
    return (*(const int *)a) - (*(const int *)b);
}

static void
draw_fillpoly(SDL_Surface *dst, int *point_x, int *point_y,
              Py_ssize_t num_points, Uint32 color)
{
    /* point_x : x coordinates of the points
     * point-y : the y coordinates of the points
     * num_points : the number of points
     */
    Py_ssize_t i, i_previous;  // i_previous is the index of the point before i
    int y, miny, maxy;
    int x1, y1;
    int x2, y2;
    /* x_intersect are the x-coordinates of intersections of the polygon
     * with some horizontal line */
    int *x_intersect = PyMem_New(int, num_points);
    if (x_intersect == NULL) {
        PyErr_NoMemory();
        return;
    }

    /* Determine Y maxima */
    miny = point_y[0];
    maxy = point_y[0];
    for (i = 1; (i < num_points); i++) {
        miny = MIN(miny, point_y[i]);
        maxy = MAX(maxy, point_y[i]);
    }

    if (miny == maxy) {
        /* Special case: polygon only 1 pixel high. */

        /* Determine X bounds */
        int minx = point_x[0];
        int maxx = point_x[0];
        for (i = 1; (i < num_points); i++) {
            minx = MIN(minx, point_x[i]);
            maxx = MAX(maxx, point_x[i]);
        }
        drawhorzlineclip(dst, color, minx, miny, maxx);
        PyMem_Free(x_intersect);
        return;
    }

    /* Draw, scanning y
     * ----------------
     * The algorithm uses a horizontal line (y) that moves from top to the
     * bottom of the polygon:
     *
     * 1. search intersections with the border lines
     * 2. sort intersections (x_intersect)
     * 3. each two x-coordinates in x_intersect are then inside the polygon
     *    (drawhorzlineclip for a pair of two such points)
     */
    for (y = miny; (y <= maxy); y++) {
        // n_intersections is the number of intersections with the polygon
        int n_intersections = 0;
        for (i = 0; (i < num_points); i++) {
            i_previous = ((i) ? (i - 1) : (num_points - 1));

            y1 = point_y[i_previous];
            y2 = point_y[i];
            if (y1 < y2) {
                x1 = point_x[i_previous];
                x2 = point_x[i];
            }
            else if (y1 > y2) {
                y2 = point_y[i_previous];
                y1 = point_y[i];
                x2 = point_x[i_previous];
                x1 = point_x[i];
            }
            else {  // y1 == y2 : has to be handled as special case (below)
                continue;
            }
            if (((y >= y1) && (y < y2)) || ((y == maxy) && (y2 == maxy))) {
                // add intersection if y crosses the edge (excluding the lower
                // end), or when we are on the lowest line (maxy)
                x_intersect[n_intersections++] =
                    (y - y1) * (x2 - x1) / (y2 - y1) + x1;
            }
        }
        qsort(x_intersect, n_intersections, sizeof(int), compare_int);

        for (i = 0; (i < n_intersections); i += 2) {
            drawhorzlineclip(dst, color, x_intersect[i], y,
                             x_intersect[i + 1]);
        }
    }

    /* Finally, a special case is not handled by above algorithm:
     *
     * For two border points with same height miny < y < maxy,
     * sometimes the line between them is not colored:
     * this happens when the line will be a lower border line of the polygon
     * (eg we are inside the polygon with a smaller y, and outside with a
     * bigger y),
     * So we loop for border lines that are horizontal.
     */
    for (i = 0; (i < num_points); i++) {
        i_previous = ((i) ? (i - 1) : (num_points - 1));
        y = point_y[i];

        if ((miny < y) && (point_y[i_previous] == y) && (y < maxy)) {
            drawhorzlineclip(dst, color, point_x[i], y, point_x[i_previous]);
        }
    }
    PyMem_Free(x_intersect);
}

static PyMethodDef _draw_methods[] = {
    {"aaline", (PyCFunction)aaline, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWAALINE},
    {"line", (PyCFunction)line, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWLINE},
    {"aalines", (PyCFunction)aalines, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWAALINES},
    {"lines", (PyCFunction)lines, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWLINES},
    {"ellipse", (PyCFunction)ellipse, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWELLIPSE},
    {"arc", (PyCFunction)arc, METH_VARARGS | METH_KEYWORDS, DOC_PYGAMEDRAWARC},
    {"circle", (PyCFunction)circle, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWCIRCLE},
    {"polygon", (PyCFunction)polygon, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWPOLYGON},
    {"rect", (PyCFunction)rect, METH_VARARGS | METH_KEYWORDS,
     DOC_PYGAMEDRAWRECT},

    {NULL, NULL, 0, NULL}};

MODINIT_DEFINE(draw)
{
#if PY3
    static struct PyModuleDef _module = {PyModuleDef_HEAD_INIT,
                                         "draw",
                                         DOC_PYGAMEDRAW,
                                         -1,
                                         _draw_methods,
                                         NULL,
                                         NULL,
                                         NULL,
                                         NULL};
#endif

    /* imported needed apis; Do this first so if there is an error
       the module is not loaded.
    */
    import_pygame_base();
    if (PyErr_Occurred()) {
        MODINIT_ERROR;
    }
    import_pygame_color();
    if (PyErr_Occurred()) {
        MODINIT_ERROR;
    }
    import_pygame_rect();
    if (PyErr_Occurred()) {
        MODINIT_ERROR;
    }
    import_pygame_surface();
    if (PyErr_Occurred()) {
        MODINIT_ERROR;
    }

/* create the module */
#if PY3
    return PyModule_Create(&_module);
#else
    Py_InitModule3(MODPREFIX "draw", _draw_methods, DOC_PYGAMEDRAW);
#endif
}
