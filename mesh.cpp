// mesh.cpp
// By Ross Light, 2012.
// Released as public domain. Don't be evil. :)

#include "mesh.h"

#include <map>
#include <sstream>

namespace {

template<class T>
void fillBuffer(GLuint buf, std::vector<T> data)
{
    glBindBuffer(GL_ARRAY_BUFFER, buf);
    glBufferData(GL_ARRAY_BUFFER, sizeof(T)*data.size(), &data[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

} // namespace

void Model::load(std::istream& is)
{
    myMesh* currMesh = NULL;

    mMeshes.empty();
    mVertices.empty();
    mUVs.empty();
    mNormals.empty();

    while (is.good()) {
        std::string line;
        std::getline(is, line);
        if (line.empty() || line[0] == '#') {
            continue;
        }
        std::istringstream lineStream(line);

        std::string cmd;
        lineStream >> cmd;
        if (cmd == "o") {
            std::string name;
            lineStream >> std::ws;
            std::getline(lineStream, name);
            myMesh mesh(*this, name);
            mMeshes.push_back(mesh);
            currMesh = &(*--mMeshes.end());
        } else if (cmd == "v") {
            float x, y, z;
            lineStream >> x >> y >> z;
            mVertices.push_back(glm::vec3(x, y, z));
        } else if (cmd == "vt") {
            float u, v;
            lineStream >> u >> v;
            mUVs.push_back(glm::vec2(u, v));
        } else if (cmd == "vn") {
            float x, y, z;
            lineStream >> x >> y >> z;
            mNormals.push_back(glm::vec3(x, y, z));
        } else if (cmd == "f") {
            if (currMesh == NULL) {
                myMesh mesh(*this, "");
                mMeshes.push_back(mesh);
                currMesh = &(*--mMeshes.end());
            }

            Face face;
            lineStream >> face.V[0] >> face.V[1] >> face.V[2];
            currMesh->mFaces.push_back(face);

            if (lineStream.good()) {
                lineStream >> face.V[1];
                currMesh->mFaces.push_back(face);
            }
        }
    }
}

myMesh::myMesh(const Model& model, const std::string& name)
    : mModel(&model)
    , mName(name)
{
}

typedef std::map<Face::Vertex, GLuint> VertexIndexMap;

int myMesh::makeVBO(GLuint* indexHandle, GLuint* posHandle, GLuint* uvHandle, GLuint* normalHandle) const
{
    VertexIndexMap indexMap;
    std::vector<GLuint> index;
    std::vector<GLfloat> pos;
    std::vector<GLfloat> uv;
    std::vector<GLfloat> normal;
    GLuint n = 0;

    for (std::vector<Face>::const_iterator iter = mFaces.begin(); iter != mFaces.end(); ++iter) {
        const Face& face = *iter;
        for (int i = 0; i < 3; ++i) {
            const Face::Vertex& v = face.V[i];
            VertexIndexMap::const_iterator indexIter;

            if (indexHandle != NULL && (indexIter = indexMap.find(v)) != indexMap.end()) {
                index.push_back(indexIter->second);
            } else {
                pos.push_back(mModel->mVertices[v.mVertex-1].x);
                pos.push_back(mModel->mVertices[v.mVertex-1].y);
                pos.push_back(mModel->mVertices[v.mVertex-1].z);
                
                /*pos.push_back(mModel->mVertices[v.mVertex-1].x+mModel->mNormals[v.mNormal-1].x);
                pos.push_back(mModel->mVertices[v.mVertex-1].y+mModel->mNormals[v.mNormal-1].y);
                pos.push_back(mModel->mVertices[v.mVertex-1].z+mModel->mNormals[v.mNormal-1].z);
                */
                if (v.mUV != 0) {
                    uv.push_back(mModel->mUVs[v.mUV-1].x);
                    uv.push_back(mModel->mUVs[v.mUV-1].y);
                } else {
                    uv.push_back(0.0f);
                    uv.push_back(0.0f);
                }
                if (v.mNormal != 0) {
                    normal.push_back(mModel->mNormals[v.mNormal-1].x);
                    normal.push_back(mModel->mNormals[v.mNormal-1].y);
                    normal.push_back(mModel->mNormals[v.mNormal-1].z);
                } else {
                    normal.push_back(0.0f);
                    normal.push_back(0.0f);
                    normal.push_back(0.0f);
                }

                if (indexHandle != NULL) {
                    index.push_back(n);
                    indexMap[v] = n;
                    n++;
                }
            }
        }
    }

    if (indexHandle != NULL) {
        glGenBuffers(1, indexHandle);
        fillBuffer(*indexHandle, index);
    }
    if (posHandle != NULL) {
        glGenBuffers(1, posHandle);
        fillBuffer(*posHandle, pos);
    }
    if (uvHandle != NULL) {
        glGenBuffers(1, uvHandle);
        fillBuffer(*uvHandle, uv);
    }
    if (normalHandle != NULL) {
        glGenBuffers(1, normalHandle);
        fillBuffer(*normalHandle, normal);
    }

    return mFaces.size() * 3;
}

std::istream& operator>>(std::istream& is, Face::Vertex& vert)
{
    vert.mVertex = 0;
    vert.mUV = 0;
    vert.mNormal = 0;

    is >> vert.mVertex;
    if (is.peek() != '/') {
        return is;
    }
    is.ignore();

    if (is.peek() != '/') {
        is >> vert.mUV;
    }
    if (is.peek() != '/') {
        return is;
    }
    is.ignore();

    is >> vert.mNormal;

    return is;
}

bool operator<(const Face::Vertex& a, const Face::Vertex& b)
{
    if (a.mVertex != b.mVertex) {
        return a.mVertex < b.mVertex;
    }
    if (a.mUV != b.mUV) {
        return a.mUV < b.mUV;
    }
    return a.mNormal < b.mNormal;
}

bool operator==(const Face::Vertex& a, const Face::Vertex& b)
{
    return a.mVertex == b.mVertex &&
        a.mUV == b.mUV &&
        a.mNormal == b.mNormal;
}

bool operator!=(const Face::Vertex& a, const Face::Vertex& b)
{
    return a.mVertex != b.mVertex ||
        a.mUV != b.mUV ||
        a.mNormal != b.mNormal;
}
