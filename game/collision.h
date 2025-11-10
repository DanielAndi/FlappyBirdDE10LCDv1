/**
 * @file collision.h
 * @brief Axis-aligned bounding box (AABB) collision detection system
 * 
 * This module provides collision detection functions for axis-aligned bounding boxes.
 * AABB collision is efficient and suitable for most 2D game collision detection needs.
 * 
 * @author Daniel Grijalva
 * @date 2025
 * @version 1.0
 * 
 * Coordinate System:
 * ------------------
 * The coordinate system uses screen coordinates:
 * - Origin (0, 0) is at the top-left corner
 * - X increases to the right
 * - Y increases downward
 * - Width and height are positive values
 */

#ifndef COLLISION_H
#define COLLISION_H

#include <stdbool.h>

/**
 * @brief Axis-aligned bounding box structure
 * 
 * Represents a rectangular collision box aligned with the coordinate axes.
 * The box is defined by its top-left corner position and dimensions.
 */
typedef struct {
    float x;      /**< X coordinate of top-left corner */
    float y;      /**< Y coordinate of top-left corner */
    float width;  /**< Width of the box */
    float height; /**< Height of the box */
} aabb_t;

/**
 * @brief Create an axis-aligned bounding box
 * 
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Width of the box (must be positive)
 * @param height Height of the box (must be positive)
 * @return Initialized AABB structure
 */
aabb_t collision_create(float x, float y, float width, float height);

/**
 * @brief Check if two AABBs intersect
 * 
 * Determines if two axis-aligned bounding boxes overlap.
 * 
 * @param a Pointer to first AABB
 * @param b Pointer to second AABB
 * @return true if the boxes intersect, false otherwise
 * 
 * @note Returns false if either pointer is NULL
 */
bool collision_aabb_intersect(const aabb_t *a, const aabb_t *b);

/**
 * @brief Check if a point is inside an AABB
 * 
 * Determines if a point (x, y) lies within the bounds of an AABB.
 * 
 * @param x X coordinate of the point
 * @param y Y coordinate of the point
 * @param box Pointer to AABB to test against
 * @return true if the point is inside the box, false otherwise
 * 
 * @note Returns false if box pointer is NULL
 * @note Points on the boundary are considered inside
 */
bool collision_point_in_aabb(float x, float y, const aabb_t *box);

#endif /* COLLISION_H */