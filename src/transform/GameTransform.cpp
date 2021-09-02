/*******************************************************************************************
*
*   GameTransform.cpp
*   Implementation of a GameTransform component. Gives an object a position, rotation,
*   and scale in world and local space.
*   
*   Partially inspired by http://graphics.cs.cmu.edu/courses/15-466-f17/notes/hierarchy.html.
*   Matrix decomposition from https://math.stackexchange.com/questions/237369/.
*
*   LICENSE: GPLv3
*
*   Copyright (c) 2021 Juniper Dusk (@juniper-dusk)
*
*******************************************************************************************/

#include "GameTransform.h"
#include "raymath.h"
#include <iostream>

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
    // Get transformation matrix.
    Matrix ltwMat = GetLocalToWorldMatrix();
    // Extract world scale.
    float scale_x = Vector3Length({ ltwMat.m0, ltwMat.m1, ltwMat.m2 });
    float scale_y = Vector3Length({ ltwMat.m4, ltwMat.m5, ltwMat.m6 });
    float scale_z = Vector3Length({ ltwMat.m8, ltwMat.m9, ltwMat.m10 });
    // Extract world position.
    return { ltwMat.m12 * scale_x, ltwMat.m13 * scale_y, ltwMat.m14 * scale_z };
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
    return QuaternionToEuler(GetWorldQuaternion());
}

Quaternion GameTransform::GetLocalQuaternion() const
{
    return rotation;
}

Quaternion GameTransform::GetWorldQuaternion() const
{
    // Get transformation matrix.
    Matrix ltwMat = GetLocalToWorldMatrix();
    // Extract world scale.
    float scale_x = Vector3Length({ ltwMat.m0, ltwMat.m1, ltwMat.m2 });
    float scale_y = Vector3Length({ ltwMat.m4, ltwMat.m5, ltwMat.m6 });
    float scale_z = Vector3Length({ ltwMat.m8, ltwMat.m9, ltwMat.m10 });
    // Extract rotation matrix.
    Matrix rotation_matrix = {
        ltwMat.m0 / scale_x, ltwMat.m1 / scale_x, ltwMat.m2 / scale_x, 0.0f,
        ltwMat.m4 / scale_y, ltwMat.m5 / scale_y, ltwMat.m6 / scale_y, 0.0f,
        ltwMat.m8 / scale_z, ltwMat.m9 / scale_z, ltwMat.m10 / scale_z, 0.0f,
        0.0f, 0.0f, 0.0f, 0.1f
    };
    return QuaternionFromMatrix(rotation_matrix);
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
    // Get transformation matrix.
    Matrix ltwMat = GetLocalToWorldMatrix();
    // Extract world scale.
    return { 
        Vector3Length({ ltwMat.m0, ltwMat.m1, ltwMat.m2 }), // Scale X
        Vector3Length({ ltwMat.m4, ltwMat.m5, ltwMat.m6 }), // Scale Y
        Vector3Length({ ltwMat.m8, ltwMat.m9, ltwMat.m10 }) // Scale Z
    };
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

// TODO: The matrix generation methods should not depend on transform state,
// so as to allow passing in various positions. For example, to set the world position here.
Matrix GameTransform::GetWorldToLocalMatrix() const
{
    // To go in the opposite direction, simply invert the matrix.
    return MatrixInvert(GetLocalToWorldMatrix());
}

Matrix GameTransform::GetLocalMatrix() const
{
    // Get matrices for translation from local to parent.
    Matrix translationMatrix = MatrixTranslate(position.x, position.y, position.z);
    // Get rotation from Quaternion.
    Vector3 rotationAxis = {0.0f, 0.0f, 0.0f};
    float rotationAngle = 0.0f;
    QuaternionToAxisAngle(rotation, &rotationAxis, &rotationAngle);
    Matrix rotationMatrix = MatrixRotate(rotationAxis, rotationAngle);
    Matrix scaleMatrix = MatrixScale(scale.x, scale.y, scale.z);

    // Order matters: scale, rotation, transformation.
    Matrix product = MatrixMultiply(scaleMatrix, rotationMatrix);
    product = MatrixMultiply(product, translationMatrix);

    // std::cout << "Local Matrix: " << std::endl;
    // std::cout << product.m0 << product.m1 << product.m2 << product.m3 << std::endl;
    // std::cout << product.m4 << product.m5 << product.m6 << product.m7 << std::endl;
    // std::cout << product.m8 << product.m9 << product.m10 << product.m11 << std::endl;
    // std::cout << product.m12 << product.m13 << product.m14 << product.m15 << std::endl;

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