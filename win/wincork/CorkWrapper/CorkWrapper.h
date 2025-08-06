#pragma once

#ifdef _WIN32
#define CORK_API __declspec(dllexport)
#else
#define CORK_API
#endif

// 枚举与结构体不能在 extern "C" 块中声明
enum BooleanOp
{
    Union = 0,
    Intersection = 1,
    Difference = 2
};

struct Vector3f
{
    float x, y, z;
};

#ifdef __cplusplus
extern "C" {
#endif

    CORK_API bool PerformBooleanOperation(
        const Vector3f* verticesA, int vertexCountA,
        const int* indicesA, int indexCountA,
        const Vector3f* verticesB, int vertexCountB,
        const int* indicesB, int indexCountB,
        BooleanOp operation,
        Vector3f** outVertices, int* outVertexCount,
        int** outIndices, int* outIndexCount
    );

    CORK_API void FreeMeshResult(Vector3f* vertices, int* indices);

#ifdef __cplusplus
}
#endif
