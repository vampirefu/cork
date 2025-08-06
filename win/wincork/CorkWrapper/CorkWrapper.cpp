#include "pch.h"
#include <cstdlib>  // for malloc/free
#include <cstring>  // for memcpy
#include "CorkWrapper.h"
#include "cork.h" // Cork 主头文件

bool PerformBooleanOperation(
    const Vector3f* verticesA, int vertexCountA,
    const int* indicesA, int indexCountA,
    const Vector3f* verticesB, int vertexCountB,
    const int* indicesB, int indexCountB,
    BooleanOp operation,
    Vector3f** outVertices, int* outVertexCount,
    int** outIndices, int* outIndexCount)
{
    // === 构造 CorkTriMesh A ===
    CorkTriMesh meshA;
    meshA.n_vertices = vertexCountA;
    meshA.vertices = (float*)malloc(sizeof(float) * vertexCountA * 3);
    for (int i = 0; i < vertexCountA; ++i) {
        meshA.vertices[i * 3 + 0] = verticesA[i].x;
        meshA.vertices[i * 3 + 1] = verticesA[i].y;
        meshA.vertices[i * 3 + 2] = verticesA[i].z;
    }

    meshA.n_triangles = indexCountA / 3;
    meshA.triangles = (unsigned int*)malloc(sizeof(unsigned int) * indexCountA);
    for (int i = 0; i < indexCountA; ++i) {
        meshA.triangles[i] = (unsigned int)indicesA[i];
    }

    // === 构造 CorkTriMesh B ===
    CorkTriMesh meshB;
    meshB.n_vertices = vertexCountB;
    meshB.vertices = (float*)malloc(sizeof(float) * vertexCountB * 3);
    for (int i = 0; i < vertexCountB; ++i) {
        meshB.vertices[i * 3 + 0] = verticesB[i].x;
        meshB.vertices[i * 3 + 1] = verticesB[i].y;
        meshB.vertices[i * 3 + 2] = verticesB[i].z;
    }

    meshB.n_triangles = indexCountB / 3;
    meshB.triangles = (unsigned int*)malloc(sizeof(unsigned int) * indexCountB);
    for (int i = 0; i < indexCountB; ++i) {
        meshB.triangles[i] = (unsigned int)indicesB[i];
    }

    // === 执行布尔操作 ===
    CorkTriMesh result;
    switch (operation)
    {
    case Union: computeUnion(meshA, meshB, &result); break;
    case Intersection: computeIntersection(meshA, meshB, &result); break;
    case Difference: computeDifference(meshA, meshB, &result); break;
    default:
        free(meshA.vertices); free(meshA.triangles);
        free(meshB.vertices); free(meshB.triangles);
        return false;
    }

    // === 释放输入 Mesh ===
    free(meshA.vertices); free(meshA.triangles);
    free(meshB.vertices); free(meshB.triangles);

    // === 拷贝结果 ===
    *outVertexCount = (int)result.n_vertices;
    *outVertices = new Vector3f[*outVertexCount];
    for (int i = 0; i < *outVertexCount; ++i)
    {
        (*outVertices)[i].x = result.vertices[i * 3 + 0];
        (*outVertices)[i].y = result.vertices[i * 3 + 1];
        (*outVertices)[i].z = result.vertices[i * 3 + 2];
    }

    *outIndexCount = (int)result.n_triangles * 3;
    *outIndices = new int[*outIndexCount];
    for (int i = 0; i < result.n_triangles * 3; ++i)
    {
        (*outIndices)[i] = (int)result.triangles[i];
    }

    // === 释放 cork 结果 ===
    freeCorkTriMesh(&result);

    return true;
}

void FreeMeshResult(Vector3f* vertices, int* indices)
{
    delete[] vertices;
    delete[] indices;
}


