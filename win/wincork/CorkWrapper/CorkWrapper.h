#pragma once

#ifdef CORKWRAPPER_EXPORTS
#define CORK_API __declspec(dllexport)
#else
#define CORK_API __declspec(dllimport)
#endif

extern "C"
{
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
}


