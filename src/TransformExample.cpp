
#include "raylib.h"
#include "raymath.h"
#include <transform/GameTransform.h>
#include <iostream>

using namespace GameEngine;

int main(int argc, char* argv[])
{
    // Initialization
    //--------------------------------------------------------------------------------------
    const int screenWidth = 800;
    const int screenHeight = 450;

    InitWindow(screenWidth, screenHeight, "Transform Example");

    // Set camera.
    Camera3D camera = { 0 };
    camera.position = (Vector3){ 0.0f, 10.0f, 10.0f };  // Camera position
    camera.target = (Vector3){ 0.0f, 0.0f, 0.0f };      // Camera looking at point
    camera.up = (Vector3){ 0.0f, 1.0f, 0.0f };          // Camera up vector (rotation towards target)
    camera.fovy = 45.0f;                                // Camera field-of-view Y
    camera.projection = CAMERA_PERSPECTIVE;             // Camera mode type
    SetCameraMode(camera, CAMERA_FREE); // Set a free camera mode

    // WORLD.
    GameTransform worldTransform(
        { 0.0, 0.0, 0.0 }, // Position
        {{ 0.0, 1.0, 0.0 }, 0.0}, // Rotation
        { 1.0, 1.0, 1.0 }  // Scale
    );
    Texture2D texture = LoadTexture("resources/Brick_0.png");

    // CUBE.
    // Transform.
    GameTransform cubeTransform(
        { 1.0, 1.0, 1.0 }, // Position
        {{ 1.0, 1.0, 1.0 }, 0.0 }, // Rotation
        { 2.0, 2.0, 2.0 }  // Scale
    );
    cubeTransform.SetParent(&worldTransform);
    // Model.
    Mesh cubeMesh = GenMeshCube(1.0f, 1.0f, 1.0f);
    Model cubeModel = LoadModelFromMesh(cubeMesh);
    cubeModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;
    
    // SPHERE.
    // Transform.
    GameTransform sphereTransform(
        { 0.0, 1.0, 1.0 },
        {{ 1.0, 1.0, 1.0 }, 0.0},
        { 0.5, 0.5, 0.5 }
    );
    sphereTransform.SetParent(&cubeTransform);
    // Model.
    Mesh sphereMesh = GenMeshSphere(1.0f, 10, 10);
    Model sphereModel = LoadModelFromMesh(sphereMesh);
    sphereModel.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = texture;

    float spin = 0.0f;

    SetTargetFPS(60);

    // Main game loop
    while (!WindowShouldClose())    // Detect window close button or ESC key
    {
        // Update
        //----------------------------------------------------------------------------------
        UpdateCamera(&camera);              // Update camera
        // Get rotation.
        // worldTransform.SetLocalRotation({ {0.0, 1.0, 0.0}, spin * 0.5f });
        cubeTransform.SetLocalRotation({ {0.0, 1.0, 0.0}, spin });
        // sphereTransform.SetLocalRotation({ {0.0, 1.0, 0.0}, spin });
        RotationAxisAngle cubeRotation = cubeTransform.GetWorldRotation();
        Vector3 cubePosition = cubeTransform.GetWorldPosition();
        Vector3 cubeScale = cubeTransform.GetWorldScale();
        RotationAxisAngle sphereRotation = sphereTransform.GetWorldRotation();
        Vector3 spherePosition = sphereTransform.GetWorldPosition();
        Vector3 sphereScale = sphereTransform.GetWorldScale();
        
        //----------------------------------------------------------------------------------

        // Draw
        //----------------------------------------------------------------------------------
        BeginDrawing();

            ClearBackground(RAYWHITE);

            BeginMode3D(camera);

                int ypos = 50;
                
                DrawModelEx(cubeModel, { cubePosition.x, cubePosition.y, cubePosition.z }, cubeRotation.axis, cubeRotation.angle, cubeTransform.GetWorldScale(), WHITE);

                DrawModelEx(sphereModel, { spherePosition.x, spherePosition.y, spherePosition.z }, sphereRotation.axis, sphereRotation.angle, sphereTransform.GetWorldScale(), WHITE);

                spin += 1.0f;

                DrawGrid(10, 1.0f);

            EndMode3D();

            DrawText(TextFormat("Cube Pos: %3.2f %3.2f %3.2f",
                                cubePosition.x,
                                cubePosition.y,
                                cubePosition.z), 10, ypos + 15, 10, BLACK);
            DrawText(TextFormat("Cube Rot: %3.2f %3.2f %3.2f %3.2f",
                                cubeRotation.axis.x,
                                cubeRotation.axis.y,
                                cubeRotation.axis.z,
                                cubeRotation.angle), 10, ypos + 30, 10, BLACK);
            DrawText(TextFormat("Sphere Pos: %3.2f %3.2f %3.2f",
                                spherePosition.x,
                                spherePosition.y,
                                spherePosition.z), 10, ypos + 45, 10, BLACK);
            DrawText(TextFormat("Sphere Rot: %3.2f %3.2f %3.2f %3.2f",
                                sphereRotation.axis.x,
                                sphereRotation.axis.y,
                                sphereRotation.axis.z,
                                sphereRotation.angle), 10, ypos + 60, 10, BLACK);

            DrawFPS(10, 10);

        EndDrawing();
        //----------------------------------------------------------------------------------
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    CloseWindow();        // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}