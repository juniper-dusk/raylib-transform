/*******************************************************************************************
*
*   GameTransform.h
*   Definition of a GameTransform component. Gives an object a position, rotation,
*   and scale in world and local space. Rotation is get/set by euler angles, but uses
*   quaternions under the hood to avoid gimbal lock and other problems.
*
*   Partially inspired by http://graphics.cs.cmu.edu/courses/15-466-f17/notes/hierarchy.html
*
*   LICENSE: GPLv3
*
*   Copyright (c) 2021 Juniper Dusk (@juniper-dusk)
*
*******************************************************************************************/

#include "raylib.h"
#include <list>
#include <memory>
#include <utility>

namespace GameEngine
{

//typedef std::pair<Vector3, float> RotationAxisAngle;
typedef struct RotationAxisAngle
{
    Vector3 axis;
    float   angle;
} RotationAxisAngle;

class GameTransform
{
public:
    // INITIALIZATION.
    // Default constructor.
    GameTransform();
    // Verbose constructor in local space.
    GameTransform(Vector3 localPosition, RotationAxisAngle rotation, Vector3 localScale);
    // Disallow copies.
    GameTransform(const GameTransform& copy) = delete;
    // Default destructor.
    virtual ~GameTransform();

    // POSITION PROPERTY.
    // Local.
    Vector3 GetLocalPosition() const;
    void SetLocalPosition(Vector3 localPosition);
    // World.
    Vector3 GetWorldPosition() const;

    // ROTATION PROPERTY.
    // Local.
    RotationAxisAngle GetLocalRotation() const;
    void SetLocalRotation(RotationAxisAngle rotation);
    // World.
    RotationAxisAngle GetWorldRotation() const;

    // SCALE PROPERTY.
    // Local.
    Vector3 GetLocalScale() const;
    void SetLocalScale(Vector3 localScale);
    // World.
    Vector3 GetWorldScale() const;

    // SPACE TRANSFORMATIONS.
    // Local to world space.
    Matrix GetLocalToWorldMatrix() const;
    // World to local space.
    Matrix GetWorldToLocalMatrix() const;

    // HIERARCHY OPERATIONS.
    void SetParent(GameTransform* newParent, unsigned int childIndex = 0);

protected:
    // Parent transform.
    GameTransform* parent;
    // Child transforms.
    std::list<GameTransform*> children;

    // (X, Y, Z) coordinates of position.
    Vector3 position;
    // (W, X, Y, Z) quaternion describing rotation.
    Quaternion rotation;
    // Store the axis/angle separately. Conversion is not well defined.
    RotationAxisAngle rotationAxisAngle;
    // (X, Y, Z) scalar amounts.
    Vector3 scale;

    // Matrix that transforms a point from local to world space.
    Matrix MakeLocalToParent() const;
    Matrix MakeParentToLocal() const;
};

}