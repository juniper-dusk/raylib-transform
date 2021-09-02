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

namespace GameEngine
{

class GameTransform
{
public:
    // INITIALIZATION.
    // Default constructor.
    GameTransform();
    // Verbose constructor in local space.
    GameTransform(Vector3 localPosition, Vector3 localEulerRotation, Vector3 localScale);
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
    Vector3 GetLocalRotation() const;
    void SetLocalRotation(Vector3 localEulerRotation);
    // World.
    Vector3 GetWorldRotation() const;
    // Quaternion rotation.
    Quaternion GetWorldQuaternion() const;
    Quaternion GetLocalQuaternion() const;

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
    // (X, Y, Z) scalar amounts.
    Vector3 scale;

    // TODO: Store individual matrices, only compose when needed together, not individually.
    // Maybe even use Quaternions along the hierarchy, instead of converting in between.

    // Matrix that transforms a point from local to world space.
    Matrix GetLocalMatrix() const;
};

}