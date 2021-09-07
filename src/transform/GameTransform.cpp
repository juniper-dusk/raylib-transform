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
#include <stdexcept>

namespace GameEngine
{

const float EPSILON = 0.001;

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
    // Extract translation.
    return ExtractTranslation(ltwMat);
}

RotationAxisAngle GameTransform::GetLocalRotation() const
{
    Vector3 rotationAxis = {0, 0, 0};
    float rotationAngle = 0;
    QuaternionToAxisAngle(rotation, &rotationAxis, &rotationAngle);
    return { rotationAxis, rotationAngle * RAD2DEG };
}

void GameTransform::SetLocalRotation(RotationAxisAngle rotation)
{
    this->rotation = QuaternionFromAxisAngle(rotation.axis, rotation.angle * DEG2RAD);
}

RotationAxisAngle GameTransform::GetWorldRotation() const
{
    // Get transformation matrix.
    Matrix ltwMat = GetLocalToWorldMatrix();
    Matrix transformationMatrix = ltwMat;

    // std::cout << "World matrix: " << std::endl;
    // std::cout << transformationMatrix.m0 << " " << transformationMatrix.m4 << " " << transformationMatrix.m8 << " " << transformationMatrix.m12 << std::endl;
    // std::cout << transformationMatrix.m1 << " " << transformationMatrix.m5 << " " << transformationMatrix.m9 << " " << transformationMatrix.m13 << std::endl;
    // std::cout << transformationMatrix.m2 << " " << transformationMatrix.m6 << " " << transformationMatrix.m10 << " " << transformationMatrix.m14 << std::endl;
    // std::cout << transformationMatrix.m3 << " " << transformationMatrix.m7 << " " << transformationMatrix.m11 << " " << transformationMatrix.m15 << std::endl;

    Matrix rotationMatrix = ExtractRotation(ltwMat);

    // Check to see if rotation is non-zero.
    float matrixAngle = acos((rotationMatrix.m0 + rotationMatrix.m5 + rotationMatrix.m10 - 1) / 2);
    // If rotation is zero, do not proceed to quaternion conversion.
    if (matrixAngle <= EPSILON && matrixAngle >= -EPSILON)
    {
        return { {0, 0, 0}, 0 };
    }

    // Get quaternion from matrix.
    Quaternion rotationQuat = QuaternionFromMatrix(rotationMatrix);
    if (isnan(rotationQuat.w) || isnan(rotationQuat.x) || isnan(rotationQuat.y) || isnan(rotationQuat.z))
    {
        throw std::runtime_error("Invalid quaternion created from rotation matrix!");
    }
    Vector3 rotationAxis = {0.0, 0.0, 0.0};
    float rotationAngle = 0.0;
    QuaternionToAxisAngle(rotationQuat, &rotationAxis, &rotationAngle);

    return { rotationAxis, rotationAngle * RAD2DEG };
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
    return ExtractScale(ltwMat);
}

Matrix GameTransform::GetLocalToWorldMatrix() const
{
    if (parent)
    {
        // Get parent matrix.
        return MatrixMultiply(MakeLocalToParent(), parent->GetLocalToWorldMatrix());
    }
    else
    {
        // Base case: root node.
        return MakeLocalToParent();
    }
}

Matrix GameTransform::GetWorldToLocalMatrix() const
{
    return MatrixInvert(GetLocalToWorldMatrix());
}

Matrix GameTransform::MakeLocalToParent() const
{
    // Get matrices for transformation from local to parent.
    Matrix scaleMatrix = MatrixScale(scale.x, scale.y, scale.z);
    // Get parent scale.
    Vector3 parentScale = {1, 1, 1};
    if ( parent )
    {
        parentScale = parent->GetLocalScale();
    }
    Matrix rotationMatrix = QuaternionToMatrix(rotation);
    Matrix translationMatrix = MatrixTranslate(position.x, position.y, position.z);

    Matrix transformationMatrix = MatrixMultiply(scaleMatrix, rotationMatrix);
    transformationMatrix = MatrixMultiply(transformationMatrix, translationMatrix);
    
    // // Order matters: scale -> rotation -> translation.
    // Matrix transformationMatrix = {
    //     rotationMatrix.m0 * scale.x, rotationMatrix.m4 * scale.y, rotationMatrix.m8 * scale.z,  position.x,
    //     rotationMatrix.m1 * scale.x, rotationMatrix.m5 * scale.y, rotationMatrix.m9 * scale.z,  position.y,
    //     rotationMatrix.m2 * scale.x, rotationMatrix.m6 * scale.y, rotationMatrix.m10 * scale.z, position.z,
    //     0.0f,                        0.0f,                        0.0f,                         1.0f
    // };

    // std::cout << "Local to parent matrix: " << std::endl;
    // std::cout << transformationMatrix.m0 << " " << transformationMatrix.m4 << " " << transformationMatrix.m8 << " " << transformationMatrix.m12 << std::endl;
    // std::cout << transformationMatrix.m1 << " " << transformationMatrix.m5 << " " << transformationMatrix.m9 << " " << transformationMatrix.m13 << std::endl;
    // std::cout << transformationMatrix.m2 << " " << transformationMatrix.m6 << " " << transformationMatrix.m10 << " " << transformationMatrix.m14 << std::endl;
    // std::cout << transformationMatrix.m3 << " " << transformationMatrix.m7 << " " << transformationMatrix.m11 << " " << transformationMatrix.m15 << std::endl;

    return transformationMatrix;
}

Matrix GameTransform::MakeParentToLocal() const
{
    return MatrixInvert(MakeLocalToParent());
}

Vector3 GameTransform::ExtractTranslation(Matrix transform)
{
    return { transform.m12, transform.m13, transform.m14 };
}

Matrix GameTransform::ExtractRotation(Matrix transform)
{
    // Extract scale.
    Vector3 scale = ExtractScale(transform);
    // Extract rotation matrix.
    return {
        transform.m0 / scale.x, transform.m4 / scale.y, transform.m8 / scale.z,  0.0f,
        transform.m1 / scale.x, transform.m5 / scale.y, transform.m9 / scale.z,  0.0f,
        transform.m2 / scale.x, transform.m6 / scale.y, transform.m10 / scale.z, 0.0f,
        0.0f,                0.0f,                0.0f,                 1.0f
    };
}

Vector3 GameTransform::ExtractScale(Matrix transform)
{
    return { 
        Vector3Length({ transform.m0, transform.m1, transform.m2 }), // Scale X
        Vector3Length({ transform.m4, transform.m5, transform.m6 }), // Scale Y
        Vector3Length({ transform.m8, transform.m9, transform.m10 }) // Scale Z
    };
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