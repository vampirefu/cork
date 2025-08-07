#include "pch.h"
#include <cstdlib>  // for malloc/free
#include <cstring>  // for memcpy
#include "CorkWrapper.h"
#include "cork.h" // Cork 主头文件
#include <new>

bool PerformBooleanOperation(
    const Vector3f* verticesA, int vertexCountA,
    const int* indicesA, int indexCountA,
    const Vector3f* verticesB, int vertexCountB,
    const int* indicesB, int indexCountB,
    BooleanOp operation,
    Vector3f** outVertices, int* outVertexCount,
    int** outIndices, int* outIndexCount)
{
    // 参数验证
    if (!verticesA || !indicesA || !verticesB || !indicesB ||
        !outVertices || !outVertexCount || !outIndices || !outIndexCount ||
        vertexCountA <= 0 || vertexCountB <= 0 ||
        indexCountA <= 0 || indexCountB <= 0 ||
        indexCountA % 3 != 0 || indexCountB % 3 != 0) {
        return false;
    }

    // 初始化输出参数
    *outVertices = nullptr;
    *outVertexCount = 0;
    *outIndices = nullptr;
    *outIndexCount = 0;

    // === 构造 CorkTriMesh A ===
    CorkTriMesh meshA;
    meshA.n_vertices = vertexCountA;
    meshA.vertices = new float[vertexCountA * 3];
    if (!meshA.vertices) {
        return false;
    }

    for (int i = 0; i < vertexCountA; ++i) {
        meshA.vertices[i * 3 + 0] = verticesA[i].x;
        meshA.vertices[i * 3 + 1] = verticesA[i].y;
        meshA.vertices[i * 3 + 2] = verticesA[i].z;
    }

    meshA.n_triangles = indexCountA / 3;
    meshA.triangles = new unsigned int[indexCountA];
    if (!meshA.triangles) {
        delete[] meshA.vertices;
        return false;
    }

    // 验证索引范围
    for (int i = 0; i < indexCountA; ++i) {
        if (indicesA[i] < 0 || indicesA[i] >= vertexCountA) {
            delete[] meshA.vertices;
            delete[] meshA.triangles;
            return false;
        }
    }

    // 修正：反转三角形顶点顺序以适应右手坐标系
    for (int i = 0; i < indexCountA; i += 3) {
        meshA.triangles[i + 0] = (unsigned int)indicesA[i + 0];
        meshA.triangles[i + 1] = (unsigned int)indicesA[i + 2]; // 交换
        meshA.triangles[i + 2] = (unsigned int)indicesA[i + 1]; // 交换
    }

    // === 构造 CorkTriMesh B ===
    CorkTriMesh meshB;
    meshB.n_vertices = vertexCountB;
    meshB.vertices = new float[vertexCountB * 3];
    if (!meshB.vertices) {
        delete[] meshA.vertices;
        delete[] meshA.triangles;
        return false;
    }

    for (int i = 0; i < vertexCountB; ++i) {
        meshB.vertices[i * 3 + 0] = verticesB[i].x;
        meshB.vertices[i * 3 + 1] = verticesB[i].y;
        meshB.vertices[i * 3 + 2] = verticesB[i].z;
    }

    meshB.n_triangles = indexCountB / 3;
    meshB.triangles = new unsigned int[indexCountB];
    if (!meshB.triangles) {
        delete[] meshA.vertices;
        delete[] meshA.triangles;
        delete[] meshB.vertices;
        return false;
    }

    // 验证索引范围
    for (int i = 0; i < indexCountB; ++i) {
        if (indicesB[i] < 0 || indicesB[i] >= vertexCountB) {
            delete[] meshA.vertices;
            delete[] meshA.triangles;
            delete[] meshB.vertices;
            delete[] meshB.triangles;
            return false;
        }
    }

    // 修正：反转三角形顶点顺序以适应右手坐标系
    for (int i = 0; i < indexCountB; i += 3) {
        meshB.triangles[i + 0] = (unsigned int)indicesB[i + 0];
        meshB.triangles[i + 1] = (unsigned int)indicesB[i + 2]; // 交换
        meshB.triangles[i + 2] = (unsigned int)indicesB[i + 1]; // 交换
    }

    // 检查网格是否为实体
    if (!isSolid(meshA) || !isSolid(meshB)) {
        delete[] meshA.vertices; delete[] meshA.triangles;
        delete[] meshB.vertices; delete[] meshB.triangles;
        return false;
    }

    // === 执行布尔操作 ===
    CorkTriMesh result;
    // 初始化结果结构体
    result.n_vertices = 0;
    result.n_triangles = 0;
    result.vertices = nullptr;
    result.triangles = nullptr;

    try {
        switch (operation)
        {
        case Union:
            computeUnion(meshA, meshB, &result);
            break;
        case Intersection:
            computeIntersection(meshA, meshB, &result);
            break;
        case Difference:
            computeDifference(meshA, meshB, &result);
            break;
        default:
            delete[] meshA.vertices; delete[] meshA.triangles;
            delete[] meshB.vertices; delete[] meshB.triangles;
            return false;
        }
    }
    catch (...) {
        // Cork 操作失败
        delete[] meshA.vertices; delete[] meshA.triangles;
        delete[] meshB.vertices; delete[] meshB.triangles;
        if (result.vertices) freeCorkTriMesh(&result);
        return false;
    }

    // === 释放输入 Mesh ===
    delete[] meshA.vertices; delete[] meshA.triangles;
    delete[] meshB.vertices; delete[] meshB.triangles;

    // 检查结果是否有效
    if (result.n_vertices == 0 || result.n_triangles == 0 ||
        !result.vertices || !result.triangles) {
        if (result.vertices || result.triangles) {
            freeCorkTriMesh(&result);
        }
        return false;
    }

    // === 拷贝结果 ===
    *outVertexCount = (int)result.n_vertices;
    *outVertices = new(std::nothrow) Vector3f[*outVertexCount];
    if (!*outVertices) {
        freeCorkTriMesh(&result);
        return false;
    }

    for (int i = 0; i < *outVertexCount; ++i)
    {
        (*outVertices)[i].x = result.vertices[i * 3 + 0];
        (*outVertices)[i].y = result.vertices[i * 3 + 1];
        (*outVertices)[i].z = result.vertices[i * 3 + 2];
    }

    *outIndexCount = (int)result.n_triangles * 3;
    *outIndices = new(std::nothrow) int[*outIndexCount];
    if (!*outIndices) {
        delete[] * outVertices;
        *outVertices = nullptr;
        *outVertexCount = 0;
        freeCorkTriMesh(&result);
        return false;
    }

    // 修正：将结果的三角形顺序转换回Unity格式
    for (int i = 0; i < result.n_triangles * 3; i += 3) {
        (*outIndices)[i + 0] = (int)result.triangles[i + 0];
        (*outIndices)[i + 1] = (int)result.triangles[i + 2]; // 交换回来
        (*outIndices)[i + 2] = (int)result.triangles[i + 1]; // 交换回来
    }

    // === 释放 cork 结果 ===
    freeCorkTriMesh(&result);

    return true;
}

void FreeMeshResult(Vector3f* vertices, int* indices)
{
    if (vertices) delete[] vertices;
    if (indices) delete[] indices;
}