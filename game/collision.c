/**
 * @file collision.c
 * @brief Implementation of collision detection functions
 * 
 * This file implements the collision detection functions declared in collision.h.
 * It provides efficient AABB-based collision detection for 2D game objects.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 */

#include "collision.h"

aabb_t collision_create(float x, float y, float width, float height) {
    aabb_t box = { x, y, width, height };
    return box;
}

bool collision_aabb_intersect(const aabb_t *a, const aabb_t *b) {
    if (!a || !b) {
        return false;
    }

    const bool overlap_x = (a->x < (b->x + b->width)) && ((a->x + a->width) > b->x);
    const bool overlap_y = (a->y < (b->y + b->height)) && ((a->y + a->height) > b->y);
    return overlap_x && overlap_y;
}

bool collision_point_in_aabb(float x, float y, const aabb_t *box) {
    if (!box) {
        return false;
    }

    const bool inside_x = (x >= box->x) && (x <= box->x + box->width);
    const bool inside_y = (y >= box->y) && (y <= box->y + box->height);
    return inside_x && inside_y;
}

