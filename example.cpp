#include <fstream>

int vertexCount;
GLuint ibo;
GLuint positions, uvs, normals;

void Initialize()
{
    // ...
    
    // open an OBJ file
    std::ifstream modelFile("model.obj");
    // parse the meshes from the file
    Model model;
    model.load(modelFile);
    // create VBOs from the first mesh in the file.
    // Any of the parameters can be NULL and the corresponding VBO won't be created.
    vertexCount = model.meshes()[0].makeVBO(&ibo, &positions, &uvs, &normals);
    // If you make IBO NULL, then the vertices will not be de-duped (you can use
    // the simpler glDrawArrays, but it will probably be more video memory).
    vertexCount = model.meshes()[0].makeVBO(NULL, &positions, &uvs, &normals);

    // You can even get fancy and check against mesh names:
    for (MeshVector::const_iterator iter = model.meshes().begin(); iter != model.meshes().end(); ++iter) {
        if (iter->name() == "Ground") {
            iter->makeVBO(/* ... */);
        }
    }

    // ...
}

void Draw()
{
    // ...

    glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(h_aUV);
    glBindBuffer(GL_ARRAY_BUFFER, uvs);
    glVertexAttribPointer(h_aUV, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, normals);
    glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // If you used the IBO (generally a good idea):
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // else:
    glDrawArrays(GL_TRIANGLES, 0, vertexCount);

    // ...
}
