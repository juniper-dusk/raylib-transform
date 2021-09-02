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

const float EPSILON = 0.005;

GameTransform::GameTransform()
{
    // Zero out data, exists at (0, 0, 0) world space.
    const Vector3 origin = {0, 0, 0};
    SetLocalPosition(origin);
    SetLocalRotation({ {0, 0, 0}, 0 });
    SetLocalScale(origin);
    // Root node.
    parent = nullptr;
}

GameTransform::GameTransform(
    Vector3 localPosition,
    RotationAxisAngle localRotation,
    Vector3 localScale)
{
    SetLocalPosition(localPosition);
    SetLocalRotation(localRotation);
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

RotationAxisAngle GameTransform::GetLocalRotation() const
{
    // First verify that quaternion is not a scalar (i.e. no rotation).
    if (sqrtf(1.0f - rotation.w*rotation.w) < EPSILON)
    {
        return { {0, 0, 0}, 0 };
    }
    Vector3 rotationAxis = {0, 0, 0};
    float rotationAngle = 0;
    QuaternionToAxisAngle(rotation, &rotationAxis, &rotationAngle);
    return { rotationAxis, rotationAngle };
}

void GameTransform::SetLocalRotation(RotationAxisAngle rotation)
{
    this->rotation = QuaternionFromAxisAngle(rotation.axis, rotation.angle);
}

RotationAxisAngle GameTransform::GetWorldRotation() const
{
    // Get transformation matrix.
    Matrix ltwMat = GetLocalToWorldMatrix();
    // Extract world scale.
    float scale_x = Vector3Length({ ltwMat.m0, ltwMat.m1, ltwMat.m2 });
    float scale_y = Vector3Length({ ltwMat.m4, ltwMat.m5, ltwMat.m6 });
    float scale_z = Vector3Length({ ltwMat.m8, ltwMat.m9, ltwMat.m10 });
    // Extract rotation matrix.
    Matrix rotation_matrix = {
        ltwMat.m0 / scale_x, ltwMat.m4 / scale_y, ltwMat.m8 / scale_z,  0.0f,
        ltwMat.m1 / scale_x, ltwMat.m5 / scale_y, ltwMat.m9 / scale_z,  0.0f,
        ltwMat.m2 / scale_x, ltwMat.m6 / scale_y, ltwMat.m10 / scale_z, 0.0f,
        0.0f,                0.0f,                0.0f,                 1.0f
    };

    Quaternion rotationQuat = QuaternionFromMatrix(rotation_matrix);
    if (sqrtf(1.0f - rotationQuat.w*rotationQuat.w) < EPSILON)
    {
        return { {0, 0, 0}, 0 };
    }
    Vector3 rotationAxis = {0.0, 0.0, 0.0};
    float rotationAngle = 0.0;
    QuaternionToAxisAngle(rotationQuat, &rotationAxis, &rotationAngle);

    return { rotationAxis, rotationAngle };
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
        return MatrixMultiply(parent->GetLocalToWorldMatrix(), MakeLocalToParent());
    }
    else
    {
        // Base case: root node.
        return MakeLocalToParent();
    }
}

// TODO: The matrix generation methods should not depend on transform state,
// so as to allow passing in various positions. For example, to set the world position here.
Matrix GameTransform::GetWorldToLocalMatrix() const
{
    if (parent)
    {
        // Recursively multiply the current transformation with the parent.
        return MatrixMultiply(parent->MakeParentToLocal(), GetWorldToLocalMatrix());
    }
    else
    {
        // Base case: root node.
        return MakeParentToLocal();
    }
    // To go in the opposite direction, simply invert the matrix.
    // return MatrixInvert(GetLocalToWorldMatrix());
}

Matrix GameTransform::MakeLocalToParent() const
{
    // Get matrices for transformation from local to parent.
    Matrix scaleMatrix = MatrixScale(scale.x, scale.y, scale.z);
    Matrix rotationMatrix = QuaternionToMatrix(rotation);
    Matrix translationMatrix = MatrixTranslate(position.x, position.y, position.z);
    
    // Order matters: scale * rotation + translation
    Matrix product = MatrixMultiply(scaleMatrix, rotationMatrix);
    Matrix transformationMatrix = MatrixMultiply(product, translationMatrix);

    // std::cout << "Local Matrix: " << std::endl;
    // std::cout << product.m0 << " " << product.m4 << " " << product.m8 << " " << product.m12 << std::endl;
    // std::cout << product.m1 << " " << product.m5 << " " << product.m9 << " " << product.m13 << std::endl;
    // std::cout << product.m2 << " " << product.m6 << " " << product.m10 << " " << product.m14 << std::endl;
    // std::cout << product.m3 << " " << product.m7 << " " << product.m11 << " " << product.m15 << std::endl;

    return transformationMatrix;
}

Matrix GameTransform::MakeParentToLocal() const
{
    // Get inverse scale, avoid division by zero errors.
    // Vector3 inverseScale = {
    //     (scale.x == 0.0f ? 0.0f : 1.0f / scale.x),
    //     (scale.y == 0.0f ? 0.0f : 1.0f / scale.y),
    //     (scale.z == 0.0f ? 0.0f : 1.0f / scale.z)
    // };
    // Matrix inverseScaleMatrix = MatrixScale(inverseScale.x, inverseScale.y, inverseScale.z);
    // // Get inverse rotation.
    // Quaternion inverseRotation = QuaternionInvert(rotation);
    // Matrix inverseRotationMatrix = QuaternionToMatrix(inverseRotation);
    // // Get inverse translation.
    // Matrix inverseTranslationMatrix = MatrixTranslate(position.x * -1, position.y * -1, position.z * -1);

    // Matrix product = MatrixMultiply(inverseScaleMatrix, inverseRotationMatrix);
    // Matrix transformationMatrix = MatrixAdd(product, MatrixMultiply(product, inverseTranslationMatrix));

    // return transformationMatrix;

    return MatrixInvert(MakeLocalToParent());
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
        std::advance(iterator, childIndex);
        parent->children.insert(iterator, this);
    }
}

}