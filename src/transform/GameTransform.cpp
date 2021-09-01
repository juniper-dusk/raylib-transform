/*******************************************************************************************
*
*   GameTransform.cpp
*   Implementation of a GameTransform component. Gives an object a position, rotation,
*   and scale in world and local space.
*   
*   Partially inspired by http://graphics.cs.cmu.edu/courses/15-466-f17/notes/hierarchy.html
*
*   LICENSE: GPLv3
*
*   Copyright (c) 2021 Juniper Dusk (@juniper-dusk)
*
*******************************************************************************************/

#include "GameTransform.h"
#include "raymath.h"

namespace GameEngine
{

GameTransform::GameTransform()
{
    // Zero out data, exists at (0, 0, 0) world space.
    const Vector3 origin = {0, 0, 0};
    SetLocalPosition(origin);
    SetLocalRotation(origin);
    SetLocalScale(origin);
    // Root node.
    parent = nullptr;
}

GameTransform::GameTransform(
    Vector3 localPosition,
    Vector3 localEulerRotation,
    Vector3 localScale)
{
    SetLocalPosition(localPosition);
    SetLocalRotation(localEulerRotation);
    SetLocalScale(localScale);
    // Root node.
    parent = nullptr;
}

GameTransform::~GameTransform()
{
    // Remove dangling pointers from children and parent.
    for (GameTransform* child: children)
    {
        child->SetParent(nullptr);
    }
    if (parent)
    {
        this->SetParent(nullptr);
    }
}

Vector3 GameTransform::GetLocalPosition() const
{
    return position;
}

void GameTransform::SetLocalPosition(Vector3 localPosition)
{
    position = localPosition;
}

Vector3 GameTransform::GetWorldPosition() const
{
    return Vector3Transform(position, GetLocalToWorldMatrix());
}

void GameTransform::SetWorldPosition(Vector3 worldPosition)
{
    position = worldPosition;
}

Vector3 GameTransform::GetLocalRotation() const
{
    return QuaternionToEuler(rotation);
}

void GameTransform::SetLocalRotation(Vector3 localEulerRotation)
{
    rotation = QuaternionFromEuler(localEulerRotation.x, localEulerRotation.y, localEulerRotation.z);
}

Vector3 GameTransform::GetWorldRotation() const
{
    Quaternion worldRotation = QuaternionTransform(rotation, GetLocalToWorldMatrix());
    return QuaternionToEuler(rotation);
}

void GameTransform::SetWorldRotation(Vector3 worldEulerRotation)
{
    Quaternion worldRotation = QuaternionFromEuler(worldEulerRotation.x, worldEulerRotation.y, worldEulerRotation.z);
    rotation = QuaternionTransform(worldRotation, GetWorldToLocalMatrix());
}

Vector3 GameTransform::GetLocalScale() const
{
    return scale;
}

void GameTransform::SetLocalScale(Vector3 localScale)
{
    scale = localScale;
}

Vector3 GameTransform::GetWorldScale() const
{
    return Vector3Transform(scale, GetLocalToWorldMatrix());
}

void GameTransform::SetWorldScale(Vector3 worldScale)
{
    scale = Vector3Transform(worldScale, GetWorldToLocalMatrix());
}

Matrix GameTransform::GetLocalToWorldMatrix() const
{
    if (parent)
    {
        // Recursively multiply the current transformation with the parent.
        return MatrixMultiply(parent->GetLocalToWorldMatrix(), GetLocalMatrix());
    }
    else
    {
        // Base case: root node.
        return GetLocalMatrix();
    }
}

Matrix GameTransform::GetWorldToLocalMatrix() const
{
    // To go in the opposite direction, simply invert the matrix.
    return MatrixInvert(GetLocalToWorldMatrix());
}

Matrix GameTransform::GetLocalMatrix() const
{
    // Get matrices for translation from local to parent.
    Matrix translationMatrix = MatrixTranslate(position.x, position.y, position.z);
    Matrix rotationMatrix = QuaternionToMatrix(rotation);
    Matrix scaleMatrix = MatrixScale(scale.x, scale.y, scale.z);

    // Order matters: scale, rotation, transformation.
    Matrix product = MatrixMultiply(scaleMatrix, rotationMatrix);
    product = MatrixMultiply(product, translationMatrix);

    return product;
}

void GameTransform::SetParent(GameTransform* newParent, unsigned int childIndex)
{
    if (parent)
    {
        // Remove pointer to current node from parent.
        parent->children.remove(this);
    }
    // Update pointer.
    parent = newParent;
    if (parent)
    {
        // Insert pointer to current node at given index in parent's children.
        auto iterator = children.begin();
        std::advance(iterator, 5);
        parent->children.insert(iterator, this);
    }
}

}