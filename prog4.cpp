//for CSC 471 - texture mapping lab
//ZJ WOOD

#ifdef __APPLE__
#include "GLUT/glut.h"
#include <OPENGL/gl.h>
#endif
#ifdef __unix__
#include <GL/glut.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include "GLSL_helper.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp" //perspective, trans etc
#include "glm/gtc/type_ptr.hpp" //value_ptr
#include "MStackHelp.h"
#include "GeometryCreator.h"
#include "mesh.h"

#include <iostream>
#include <math.h>
#include <assert.h>
#include <vector>
#include <fstream>

using namespace std;
using namespace glm;

/*data structure for the image used for  texture mapping */
typedef struct Image {
  unsigned long sizeX;
  unsigned long sizeY;
  char *data;
} Image;

Image *TextureImage;

typedef struct RGB {
  GLubyte r;
  GLubyte g; 
  GLubyte b;
} RGB;

RGB myimage[64][64];
RGB* g_pixel;

//forward declaration of image loading and texture set-up code
int ImageLoad(char *filename, Image *image);
GLvoid LoadTexture(char* image_file, int tex_id);

//mode to toggle drawing
int cube, manipulate;
float move;

//flag and ID to toggle on and off the shader
int ShadeProg, Shade2;

// mouse global variables
float start_x = 0;
float start_y = 0;
float start_z = 0;
float curr_x = 0;
float curr_y = 0;
float curr_z = 0;
float delta_x = 0; //mouse change
float delta_y = 0;
float alpha, beta;

//game stats
int fire = false; // laser active
vec3 laserMove = vec3(0, 0, 0); // laser position
float laserProgress = 0.0f; // step factor
vec3 laserLook = vec3(0, 0, 0); // look vector of laser
vec3 laserStart = vec3(0, 0, 0); // where the eye was
float myExplode[80]; // timer of animation for collision
float myMove = -100.0f; // goes from -100 to +100
int myMoveDec = false; // direction of mymove
int collides[80]; // collision detection on models
int count = 0; // hit count

static const int TieCount = 12; // must be less than 20
static const int CubeCount = 15; // must be less than 20

static const float g_maxZ = 50.0f;   // game boundaries
static const float g_minZ = -50.0f;

//Handles to the shader data
GLint h_uLight;
GLint h_uView;
GLint h_aColor;
GLint h_aNormal;
GLint h_options;

GLint h_uLaser;
GLint h_uTexUnit;
GLint h_uTexUnit2;
GLint h_aPosition;
GLint h_aTexCoord;
GLint h_uModelMatrix;
GLuint uNormalMatrix;
GLint h_uViewMatrix;
GLint h_uProjMatrix;
GLuint GrndBuffObj, GIndxBuffObj, GTexBuffObj, GroundNormalBuffObj, TexBuffObj;
int g_CiboLen, g_GiboLen;
static float  g_width, g_height;
unsigned int const StepSize = 10;
float g_angle = 0;
float g_trans = 0;

glm::vec3 up = vec3(0, 1, 0);
glm::vec3 look = vec3(0, 0, 0);
glm::vec3 eye = vec3(0.0);
glm::vec3 gaze = vec3(0, 0, 0);
glm::vec3 strafe = vec3(0, 0, 0);

float light_x = -10;
float light_y = 50; //light location
float light_z = 50;
float myRot = 0.0f;
float shapeRot = -70.0f; //ship yaw
float shipRot = 0.0f; //ship pitch

//obj loader
int vertexCount;
GLuint ibo;
GLuint positions, uvs, normals;

//declare a matrix stack
RenderingHelper ModelTrans;

static const float RADS_TO_DEGS = 180.0 / 4*atan((float)1.0); // Pi=3.14

static const float g_groundY = -1.51;      // y coordinate of the ground
static const float g_groundSize = 600.0;   // half the ground length

/* projection matrix */
void SetProjectionMatrix() {
  glm::mat4 Projection = glm::perspective(80.0f, (float)g_width/g_height, 0.1f, 100.f);
  safe_glUniformMatrix4fv(h_uProjMatrix, glm::value_ptr(Projection));
}

/* camera controls - do not change */
void SetView() {
  glm::mat4 view = glm::lookAt(eye, look, up);
  safe_glUniformMatrix4fv(h_uViewMatrix, glm::value_ptr(view));
  glUniform4f(h_uView, eye.x, eye.y, eye.z, 0);
  light_x = eye.x;
  light_y = eye.y;
  light_z = eye.z;
  glUniform3f(h_uLight, light_x, light_y, light_z);
}

/* set the model transform to the identity */
void SetModel() {
  safe_glUniformMatrix4fv(h_uModelMatrix, glm::value_ptr(ModelTrans.modelViewMatrix));
  safe_glUniformMatrix4fv(uNormalMatrix, glm::value_ptr(glm::transpose(glm::inverse(ModelTrans.modelViewMatrix))));
}

static void initGround() {

  // A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
    float GrndPos[] = {
    -g_groundSize, g_groundY, -g_groundSize,
    -g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY,  g_groundSize,
     g_groundSize, g_groundY, -g_groundSize
    };

    float GrndNorm[] = {
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0,
     0, 1, 0
    };

    static GLfloat GroundTex[] = {
      0, 1, // back 
      0, 0,
      1, 0,
      1, 1,
    }; 

    unsigned short idx[] = {0, 1, 2, 0, 2, 3};


    g_GiboLen = 6;
    glGenBuffers(1, &GrndBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

    glGenBuffers(1, &GroundNormalBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GroundNormalBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

    glGenBuffers(1, &GIndxBuffObj);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);

    glGenBuffers(1, &GTexBuffObj);
    glBindBuffer(GL_ARRAY_BUFFER, GTexBuffObj);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GroundTex), GroundTex, GL_STATIC_DRAW);
}

static void initTex() {
  static GLfloat CubeTex[] = {
    1, 0, // back 
    1, 1,
    0, 1,
    0, 0,
    0, 0, //right 
    0, 1,
    1, 1,
    1, 0,
    0, 0, //front 
    0, 1,
    1, 1,
    1, 0,
    0, 0, // left 
    0, 1,
    1, 1,
    1, 0,
    0, 0, // top 
    0, 1,
    1, 1,
    1, 0,
    0, 0, // bottom 
    0, 1,
    1, 1,
    1, 0
  };

  glGenBuffers(1, &TexBuffObj);
  glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
  glBufferData(GL_ARRAY_BUFFER, sizeof(CubeTex), CubeTex, GL_STATIC_DRAW);
}

void InitGeom() {
  initGround();
  initTex();
}

/*function to help load the shaders (both vertex and fragment */
int InstallShader(const GLchar *vShaderName, const GLchar *fShaderName) {
        GLuint VS; //handles to shader object
        GLuint FS; //handles to frag shader object
        GLint vCompiled, fCompiled, linked; //status of shader

        VS = glCreateShader(GL_VERTEX_SHADER);
        FS = glCreateShader(GL_FRAGMENT_SHADER);

        //load the source
        glShaderSource(VS, 1, &vShaderName, NULL);
        glShaderSource(FS, 1, &fShaderName, NULL);

        //compile shader and print log
        glCompileShader(VS);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetShaderiv(VS, GL_COMPILE_STATUS, &vCompiled);
        printShaderInfoLog(VS);

        //compile shader and print log
        glCompileShader(FS);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetShaderiv(FS, GL_COMPILE_STATUS, &fCompiled);
        printShaderInfoLog(FS);

        if (!vCompiled || !fCompiled) {
                printf("Error compiling either shader %s or %s", vShaderName, fShaderName);
                return 0;
        }

        //create a program object and attach the compiled shader
        ShadeProg = glCreateProgram();
        glAttachShader(ShadeProg, VS);
        glAttachShader(ShadeProg, FS);

        glLinkProgram(ShadeProg);
        /* check shader status requires helper functions */
        printOpenGLError();
        glGetProgramiv(ShadeProg, GL_LINK_STATUS, &linked);
        printProgramInfoLog(ShadeProg);

        glUseProgram(ShadeProg);

        /* get handles to attribute data */
       h_aPosition = safe_glGetAttribLocation(ShadeProg, "aPosition");
       h_aTexCoord = safe_glGetAttribLocation(ShadeProg,  "aTexCoord");
       h_uTexUnit = safe_glGetUniformLocation(ShadeProg, "uTexUnit");
       h_uProjMatrix = safe_glGetUniformLocation(ShadeProg, "uProjMatrix");
       h_uViewMatrix = safe_glGetUniformLocation(ShadeProg, "uViewMatrix");
       h_uModelMatrix = safe_glGetUniformLocation(ShadeProg, "uModelMatrix");
        h_aNormal = safe_glGetAttribLocation(ShadeProg, "aNormal"); 
        h_uView = safe_glGetUniformLocation(ShadeProg, "uView");
      h_uLight = safe_glGetUniformLocation(ShadeProg, "uLight");
      h_options = safe_glGetUniformLocation(ShadeProg, "options");
    uNormalMatrix   = safe_glGetUniformLocation(ShadeProg, "uNormalMatrix");
    h_uLaser   = safe_glGetUniformLocation(ShadeProg, "uLaser");
    h_aColor   = safe_glGetUniformLocation(ShadeProg, "aColor");
       printf("sucessfully installed shader %d\n", ShadeProg);
       return 1;

}

/* Some OpenGL initialization */
void Initialize ()                  // Any GL Init Code
{
    // Start Of User Initialization
    glClearColor(0.2f, 0.9f, 0.7f, 1.0f);
    // Black Background
    glClearDepth (1.0f);    // Depth Buffer Setup
    glDepthFunc (GL_LEQUAL);    // The Type Of Depth Testing
    glEnable (GL_DEPTH_TEST);// Enable Depth Testing
    /* some matrix stack init */
    ModelTrans.useModelViewMatrix();
    ModelTrans.loadIdentity();
    /* texture specific settings */
    glEnable(GL_TEXTURE_2D);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    // open an OBJ file
    //std::ifstream modelFile("mechanic.obj");
    //std::ifstream modelFile("ati_walker.obj");
    std::ifstream modelFile("SnowSpeeder.obj");
    // parse the meshes from the file
    Model model;
    model.load(modelFile);
    // create VBOs from the first mesh in the file.
    // Any of the parameters can be NULL and the corresponding VBO won't be created.
    vertexCount = model.meshes()[0].makeVBO(&ibo, &positions, &uvs, &normals);
    // If you make IBO NULL, then the vertices will not be de-duped (you can use
    // the simpler glDrawArrays, but it will probably be more video memory).
    //vertexCount = model.meshes()[0].makeVBO(NULL, &positions, &uvs, &normals);
}

/* Main display function */
void Draw (void)
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (cube == 0) {
    //Start our shader
    glUseProgram(ShadeProg);

    //Set up matrix transforms
    SetProjectionMatrix();
    SetView();

    ModelTrans.loadIdentity();
    SetModel();

    manipulate = false;
    glUniform3f(h_options, fire, myExplode[0], manipulate);
    glUniform3f(h_aColor, 0.3f, 0.3f, 0.3f);

    //obj (speeder)
    glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, positions);
    glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, normals);
    glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    ModelTrans.pushMatrix();
    ModelTrans.translate(vec3(look.x, look.y-0.3f, look.z));
    ModelTrans.scale(0.002f);
    ModelTrans.rotate(shapeRot, vec3(0, 1, 0));
    ModelTrans.rotate(shipRot, vec3(1, 0, 0));
    SetModel();

    // If you used the IBO (generally a good idea):
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glDrawElements(GL_TRIANGLES, vertexCount, GL_UNSIGNED_INT, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    // else:
    //glDrawArrays(GL_TRIANGLES, 0, vertexCount);
    ModelTrans.popMatrix();

    safe_glDisableVertexAttribArray(h_aNormal);
    safe_glDisableVertexAttribArray(h_aPosition);

  //laser
  if(fire){

    Mesh* m = GeometryCreator::CreateCylinder(1.f, 1.f, 1.5f, 8, 8);

    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, m->PositionHandle);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, m->NormalHandle);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->IndexHandle);

    glUniform3f(h_aColor, 0.35f, 1.0f, 0.35f);

    ModelTrans.pushMatrix();
    ModelTrans.translate(laserMove);
    ModelTrans.scale(0.3f);
    ModelTrans.rotate(shapeRot, vec3(0, 1, 0));
    ModelTrans.rotate(shipRot, vec3(1, 0, 0));
    SetModel();

    glDrawElements(GL_TRIANGLES, m->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

    ModelTrans.popMatrix();
  

    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aNormal);
  }

    //walls
    //base
    Mesh* m = GeometryCreator::CreateCylinder(1.5f, 0.5f, 2.0f, 16, 5);

    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, m->PositionHandle);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, m->NormalHandle);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->IndexHandle);

    glUniform3f(h_aColor, 0.35f, 0.35f, 0.35f);

    float tran_x = 0.0f;
    float tran_z = g_maxZ;
    for(int i = 0; i < 30; i++){
      ModelTrans.pushMatrix();
        ModelTrans.translate(vec3(tran_x, -1.0f, tran_z));
        ModelTrans.scale(4.0f);
        ModelTrans.rotate(-90.0f, vec3(1, 0, 0));
        SetModel();
        tran_x += 25.0f;
        if(i == 15){
          tran_x = 0.0f;
          tran_z = g_minZ;
        }
    
      glDrawElements(GL_TRIANGLES, m->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

      ModelTrans.popMatrix();
    }

    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aNormal);

    //wall
    m = GeometryCreator::CreateCube(vec3(0.5f, 2.f, 4.f));

    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, m->PositionHandle);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, m->NormalHandle);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->IndexHandle);

    glUniform3f(h_aColor, 0.35f, 0.35f, 0.35f);

    tran_x = 0.0f;
    tran_z = g_maxZ;
    for(int i = 0; i < 30; i++){
      ModelTrans.pushMatrix();
        ModelTrans.translate(vec3(tran_x, 6.0f, tran_z));
        ModelTrans.scale(7.0f);
        ModelTrans.rotate(-90.0f, vec3(0, 1, 0));
        SetModel();
        tran_x += 25.0f;
        if(i == 15){
          tran_x = 0.0f;
          tran_z = g_minZ;
        }
      glDrawElements(GL_TRIANGLES, m->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

      ModelTrans.popMatrix();
    }

    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aNormal);

    // tie fighter
    m = GeometryCreator::CreateSphere(glm::vec3(1.5f));
    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, m->PositionHandle);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, m->NormalHandle);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->IndexHandle);
    glUniform3f(h_aColor, 0.2f, 0.2f, 0.3f);
    tran_x = 70.0f;
    float tran_y = 15.0f;
    tran_z = 15.0f;
    tran_x -= myRot * 0.1f; //fly patterns
    float delta_z = myMove*0.1f;
    if(delta_z < 5.0f && !myMoveDec){ // dont move
      delta_z = -5.0f;
    }
    else if(delta_z > -5.0f && myMoveDec){
      delta_z = 0.0f;
    }
    if(delta_z >= 5.0f && !myMoveDec){ // start to move
      delta_z -= 10.0f;
    }
    else if(delta_z <= -5.0f && myMoveDec){
      delta_z += 5.0f;
    }
  for(int i = 0; i < TieCount; i++){
    ModelTrans.pushMatrix();
    if(laserMove.y <= 18.5f && laserMove.y >= 12.0f
      && laserMove.x <= tran_x + 2.0f && laserMove.x >= tran_x - 2.0f
      && laserMove.z <= tran_z+delta_z + 2.0f && laserMove.z >= tran_z+delta_z - 2.0f){ //did laser hit it?
      manipulate = true;
      if(collides[i+40] == false){ // new animation
        myExplode[i+40] = 0.0f;
        //count++;
        //printf("Count: %d\n", count);
      } //start animation
      collides[i+40] = true;
      glUniform3f(h_aColor, 0.7f, 0.3f, 0.3f);
    }
    if(myExplode[i+40] >= 359.0f){ collides[i+40] = false; //reset
      glUniform3f(h_aColor, 0.2f, 0.2f, 0.3f);
    }
    if(collides[i+40]){ manipulate = true; // was hit
      glUniform3f(h_aColor, 0.7f, 0.3f, 0.3f);
      tran_y -= 0.2f * myExplode[i+40];
    }
    glUniform3f(h_options, fire, myExplode[i+40], manipulate);
    ModelTrans.translate(vec3(tran_x, tran_y, tran_z+delta_z));
    ModelTrans.scale(1.05f);
    SetModel();  
    glDrawElements(GL_TRIANGLES, m->IndexBufferLength, GL_UNSIGNED_SHORT, 0);
    ModelTrans.popMatrix();
    tran_y = 15.0f;
    manipulate = false;
    glUniform3f(h_options, fire, myExplode[i+40], manipulate);
    glUniform3f(h_aColor, 0.2f, 0.2f, 0.3f);
    if(i % 2 == 0){
      tran_x += 30.0f;
      tran_z = 15.0f;
    }
    else{
      tran_z -= 30.0f;
      tran_x += 10.0f;
    }
  }
    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aNormal);

    m = GeometryCreator::CreateDisc(0.1f, 1.f, 0.f, 8, 0);
    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, m->PositionHandle);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);
    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, m->NormalHandle);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->IndexHandle);
    tran_x = 70.0f;
    tran_y = 15.0f;
    tran_z = 15.0f;
    tran_x -= myRot * 0.1f; // fly
  for(int i = 0; i < TieCount*2; i++){
    ModelTrans.pushMatrix();
    if(laserMove.y <= 18.5f && laserMove.y >= 12.0f
      && laserMove.x <= tran_x + 2.0f && laserMove.x >= tran_x - 2.0f
      && laserMove.z <= tran_z+delta_z + 2.0f && laserMove.z >= tran_z+delta_z - 2.0f){ //did laser hit it?
      manipulate = true;
      if(collides[(i/2)+40] == false){
        myExplode[(i/2)+40] = 0.0f;
        //count++;
        //printf("Count: %d\n", count);
      }
      collides[(i/2)+40] = true;
      glUniform3f(h_aColor, 0.7f, 0.3f, 0.3f);
    }
    if(myExplode[(i/2)+40] >= 359.0f){ collides[(i/2)+40] = false;
      glUniform3f(h_aColor, 0.2f, 0.2f, 0.3f);
    }
    if(collides[(i/2)+40]){ manipulate = true;
      glUniform3f(h_aColor, 0.7f, 0.3f, 0.3f);
      tran_y -= 0.2f * myExplode[(i/2)+40]; 
    }
    glUniform3f(h_options, fire, myExplode[(i/2)+40], manipulate);
    ModelTrans.translate(vec3(tran_x, tran_y, tran_z+delta_z));
    ModelTrans.scale(3.0f);
    if(i % 2 == 0){ ModelTrans.rotate(myMove*0.5f, vec3(0, 1, 0));
      ModelTrans.translate(vec3(0, 0, 0.5f)); }
    else { ModelTrans.rotate(myMove*0.5f, vec3(0, 1, 0));
      ModelTrans.translate(vec3(0, 0, -0.5f)); }
    SetModel();  
    glDrawElements(GL_TRIANGLES, m->IndexBufferLength, GL_UNSIGNED_SHORT, 0);
    ModelTrans.popMatrix();
    tran_y = 15.0f;
    manipulate = false;
    glUniform3f(h_options, fire, myExplode[(i/2)+40], manipulate);
    glUniform3f(h_aColor, 0.2f, 0.2f, 0.3f);
    if((i-1) % 4 == 0){
      tran_x += 30.0f;
      tran_z = 15.0f;
    }
    else if ((i-1) % 2 == 0){
      tran_z -= 30.0f;
      tran_x += 10.0f;
    }
  }
    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aNormal);
    safe_glDisableVertexAttribArray(h_aTexCoord);

    //end tie fighters

    //set up the texture unit
    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 1);
    glUniform3f(h_aColor, 1.5f, 1.5f, 1.5f);

    //ground
    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, GTexBuffObj);
    safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);  

    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, GroundNormalBuffObj);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
    SetModel();
    glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aTexCoord);
    safe_glDisableVertexAttribArray(h_aNormal);

    //dancing cubes
    m = GeometryCreator::CreateCube();

    safe_glEnableVertexAttribArray(h_aPosition);
    glBindBuffer(GL_ARRAY_BUFFER, m->PositionHandle);
    safe_glVertexAttribPointer(h_aPosition, 3, GL_FLOAT, GL_FALSE, 0, 0);

    safe_glEnableVertexAttribArray(h_aNormal);
    glBindBuffer(GL_ARRAY_BUFFER, m->NormalHandle);
    safe_glVertexAttribPointer(h_aNormal, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindTexture(GL_TEXTURE_2D, 2);
    safe_glEnableVertexAttribArray(h_aTexCoord);
    glBindBuffer(GL_ARRAY_BUFFER, TexBuffObj);
    safe_glVertexAttribPointer(h_aTexCoord, 2, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m->IndexHandle);

    tran_x = 40.0f;
    tran_z = 10.0f;
    for(int i = 0; i < CubeCount*2; i++){
      ModelTrans.pushMatrix();
      tran_y = 1.0f;
      if (i % 2 == 0.0f){
        if(laserMove.y <= 8.5f && laserMove.y >= -1.5f
          && laserMove.x <= tran_x + 2.51f && laserMove.x >= tran_x - 2.51f
          && laserMove.z <= tran_z + 2.51f && laserMove.z >= tran_z - 2.51f){ //did laser hit it?
          manipulate = true;
          if(collides[i] == false){ // start animations
            myExplode[i] = 0.0f;
            //glUniform3f(h_uLaser, laserLook.x+laserMove.x, laserLook.y+laserMove.y, laserLook.z+laserMove.z);
          }
          collides[i] = true;
        }
        if(myExplode[i] >= 359.0f){ collides[i] = false; }
        if(collides[i]){ manipulate = true;
          tran_y -= 0.1f * myExplode[i];  }
        glUniform3f(h_options, fire, myExplode[i], manipulate);

        ModelTrans.translate(vec3(tran_x, tran_y, tran_z));
        ModelTrans.scale(5.0f);
      }
      else{
        tran_y = 6.0f;

      if(laserMove.y <= 8.5f && laserMove.y >= -1.5f
        && laserMove.x <= tran_x + 2.51f && laserMove.x >= tran_x - 2.51f
        && laserMove.z <= tran_z + 2.51f && laserMove.z >= tran_z - 2.51f){ //did laser hit it?
        manipulate = true;
        if(collides[i] == false){ // got a new hit, start animations
          myExplode[i] = 0.0f;
          //count++;
          //printf("Count: %d\n", count);
          //glUniform3f(h_uLaser, laserLook.x+laserMove.x, laserLook.y+laserMove.y, laserLook.z+laserMove.z);
        }
        collides[i] = true;
      }
      if(myExplode[i] >= 359.0f){ collides[i] = false; }
      if(collides[i]){ manipulate = true;
      tran_y -= 0.1f * myExplode[i];  }
      glUniform3f(h_options, fire, myExplode[i], manipulate);

        ModelTrans.translate(vec3(tran_x, tran_y, tran_z));
        ModelTrans.scale(5.0f);
        if(!collides[i]){ ModelTrans.rotate(myRot, vec3(0, 1, 0)); }
        else { ModelTrans.rotate(myExplode[i]*-0.7f+myRot, vec3(0, 1, 0)); } // dancing part

        tran_z -= 10.0f;
        if (i % 3 == 0.0f){
          tran_z = 10.0f;
          tran_x += 60.0f;
        }
      }

      SetModel();
    
      glDrawElements(GL_TRIANGLES, m->IndexBufferLength, GL_UNSIGNED_SHORT, 0);

      ModelTrans.popMatrix();
    manipulate = false;
    glUniform3f(h_options, fire, myExplode[0], manipulate);
    }

    safe_glDisableVertexAttribArray(h_aPosition);
    safe_glDisableVertexAttribArray(h_aNormal);
    safe_glDisableVertexAttribArray(h_aTexCoord);

    //Disable the shader
    glUseProgram(0);
    glDisable(GL_TEXTURE_2D);
  } 
    glutSwapBuffers();

}

//translates mouse clicks into usable coordinates
//sub task is to create vectors for rotation
void glutToPixel(int x, int y, int isStart){
  float new_x = ((float)x)*2.0/(g_width-1.0);
  new_x -= 1.0;
  float new_y = ((float)y)*(-2.0)/(g_height-1.0);
  new_y += 1.0;

  if(isStart){
    start_x = new_x;
    start_y = new_y;
  }
  else{
    curr_x = new_x;
    curr_y = new_y;
  }
}

//the mouse button callback - fill in with logic for creating strokes for the program
void mouse(int button, int state, int x, int y) {
  if (button == GLUT_LEFT_BUTTON) {
     if (state == GLUT_DOWN) {
      glutToPixel(x, y, true);
    }
  }
}

//the mouse move callback
void mouseMove(int x, int y) {
  glutToPixel(x, y, false);
  delta_x = curr_x - start_x;
  delta_y = curr_y - start_y;
  alpha += 1.57*delta_y/2;
  beta += 3.14*delta_x/2;
  if (alpha > 70.0*3.14/180.0){  //boundaries on looking up/down
    alpha = 70.0*3.14/180.0;
  }
  else if (alpha < -70.0*3.14/180.0){
    alpha = -70.0*3.14/180.0;
  }
  look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
  look += eye;
  shapeRot -= delta_x/2 * 180.0f;
  shipRot += delta_y/2 * 90.0f;
  if (shipRot > 70.0){  //boundaries on ship looking up/down
    shipRot = 70.0;
  }
  else if (shipRot < -70.0){
    shipRot = -70.0;
  }
  glutPostRedisplay();
  //update mouse position variables
  start_x = curr_x;
  start_y = curr_y;
}

void keyboard(unsigned char key, int x, int y ){
  switch( key ) {
    /* WASD keyes effect view/camera transform */
    case 'w':
      gaze = look - eye;
      eye += gaze;
      if(eye.y < 0){
        eye.y = 0;
      }
      look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
      look += eye;
      break;
    case 's':
      gaze = look - eye;
      eye -= gaze;
      if(eye.y < 0){
        eye.y = 0;
      }
      look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
      look += eye;
      break;
    case 'a':
      //cross product between gaze and up
      gaze = look - eye;
      strafe = glm::cross(gaze, up);
      eye -= strafe;
      look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
      look += eye;
      break;
    case 'd':
      gaze = look - eye;
      strafe = glm::cross(gaze, up);
      eye += strafe;
      look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
      look += eye;
      break;
  case 'f': // shoot laser
    fire = 1;
    laserStart = eye;
    laserLook = look;
    laserProgress = 0.0f;
    break;
  case 'g': // toggle autofly
    if(move) move = 0.0f;
    else move = 1.0f;
    break;
  case 'r': // go up
    eye.y += 1.0f;
    look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
    look += eye;
    break;
  case 'v': // go down
    eye.y -= 1.0f;
    if(eye.y < 0){
      eye.y = 0;
    }
    look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
    look += eye;
    break;
  case 'h': // reset
    eye = vec3(0.0f, eye.y, 0.0f);
    look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
    look += eye;
    myRot = 0.0f;
    break;
 case 'q': case 'Q' :
    exit( EXIT_SUCCESS );
    break;
  }
  glutPostRedisplay();
}

/* Reshape */
void ReshapeGL (int width, int height)
{
    g_width = (float)width;
    g_height = (float)height;
    glViewport (0, 0, (GLsizei)(width), (GLsizei)(height));

}
void Timer(int param)
{
    myRot += StepSize * 0.1f;
    if (myRot >= 2080.0f){ // handles cube rotation and tie fighter cycling
      myRot = 0.0f;
    }
    if (myMove >= 100.0f){
      myMoveDec = true;
    }
    if (myMove <= -100.0f){
      myMoveDec = false;
    }
    if(!myMoveDec){ myMove += StepSize * 0.2f; }
    else { myMove -= StepSize * 0.2f; }
    for(int i = 0; i < 80; i++){
      myExplode[i] += StepSize * 0.1f;
      if (myExplode[i] >= 360.0f){
        myExplode[i] = 0.0f;
      }
    }
    if(move){
      gaze = look - eye;
      if(eye.y < 0){
        eye.y = 0;
      }
      //condition for wall collision ( > <, formula mx + b)
      if(eye.z < g_maxZ - 2 && eye.z > g_minZ + 2){
        eye += 0.3f*gaze;
      }
      else if (eye.z > g_minZ + 2){
        eye.z -= 0.2f;
      }
      else{
        eye.z += 0.2f;
      }
      look = vec3(cos(alpha)*cos(beta), sin(alpha), cos(alpha)*cos(90-beta));
      look += eye;
    }
    if(fire){
      laserProgress += StepSize * 2.0f;
      laserMove = (vec3(laserLook.x+0.1f*laserProgress*(laserLook.x-laserStart.x),
        laserLook.y-0.3f+0.1f*laserProgress*(laserLook.y-laserStart.y),
        laserLook.z+0.1f*laserProgress*(laserLook.z-laserStart.z)));
      if(laserProgress > 1100.0f){ fire = false; laserProgress = 0.0f; }
    }
    glutTimerFunc(StepSize, Timer, 1);
    glutPostRedisplay();
}

int main(int argc, char** argv) {
  
  //initialize the window
  glutInit(&argc, argv);
  glutInitWindowPosition( 20, 20 );
  glutInitWindowSize(1200, 800);
  glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH);
  glutCreateWindow("My First texture maps");
  
  //set up the opengl call backs
  glutDisplayFunc(Draw);
  glutReshapeFunc(ReshapeGL);
  glutKeyboardFunc(keyboard);
  glutMouseFunc( mouse );
  glutMotionFunc( mouseMove );
  glutTimerFunc(StepSize, Timer, 1);
  
  cube = 0;

  Initialize();
  //load in the other image textures
  LoadTexture((char *)"earth1.bmp", 0);
  LoadTexture((char *)"cloud5.bmp", 1);
  LoadTexture((char *)"Imperial.bmp", 2);
  //test the openGL version
  getGLversion();
  //install the shader
  if (!InstallShader(textFileRead((char *)"Prog4_vert.glsl"), textFileRead((char *)"Prog4_frag.glsl"))) {
        printf("Error installing shader!\n");
        return 0;
  }
  InitGeom();
 
  glutMainLoop();
  
}

//routines to load in a bmp files - must be 2^nx2^m and a 24bit bmp
GLvoid LoadTexture(char* image_file, int texID) { 
  
  TextureImage = (Image *) malloc(sizeof(Image));
  if (TextureImage == NULL) {
    printf("Error allocating space for image");
    exit(1);
  }
  cout << "trying to load " << image_file << endl;
  if (!ImageLoad(image_file, TextureImage)) {
    exit(1);
  }  
  /*  2d texture, level of detail 0 (normal), 3 components (red, green, blue),            */
  /*  x size from image, y size from image,                                              */    
  /*  border 0 (normal), rgb color data, unsigned byte data, data  */ 
  glBindTexture(GL_TEXTURE_2D, texID);

  // select modulate to mix texture with color for shading
  glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );

  // when texture area is small, bilinear filter the closest mipmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST );
  // when texture area is large, bilinear filter the first mipmap
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

  // if wrap is true, the texture wraps over at the edges (repeat)
  //       ... false, the texture ends at the edges (clamp)
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP );
  glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP );

  // build our texture mipmaps
  gluBuild2DMipmaps( GL_TEXTURE_2D, 3, TextureImage->sizeX, TextureImage->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data );
  /*glTexImage2D(GL_TEXTURE_2D, 0, 3,
    TextureImage->sizeX, TextureImage->sizeY,
    0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage->data);*/
  //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); /*  cheap scaling when image bigger than texture */    
  //glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); /*  cheap scaling when image smalled than texture*/
}


/* BMP file loader loads a 24-bit bmp file only */

/*
* getint and getshort are help functions to load the bitmap byte by byte
*/
static unsigned int getint(FILE *fp) {
  int c, c1, c2, c3;
  
  /*  get 4 bytes */ 
  c = getc(fp);  
  c1 = getc(fp);  
  c2 = getc(fp);  
  c3 = getc(fp);
  
  return ((unsigned int) c) +   
    (((unsigned int) c1) << 8) + 
    (((unsigned int) c2) << 16) +
    (((unsigned int) c3) << 24);
}

static unsigned int getshort(FILE *fp){
  int c, c1;
  
  /* get 2 bytes*/
  c = getc(fp);  
  c1 = getc(fp);
  
  return ((unsigned int) c) + (((unsigned int) c1) << 8);
}

/*  quick and dirty bitmap loader...for 24 bit bitmaps with 1 plane only.  */

int ImageLoad(char *filename, Image *image) {
  FILE *file;
  unsigned long size;                 /*  size of the image in bytes. */
  unsigned long i;                    /*  standard counter. */
  unsigned short int planes;          /*  number of planes in image (must be 1)  */
  unsigned short int bpp;             /*  number of bits per pixel (must be 24) */
  char temp;                          /*  used to convert bgr to rgb color. */
  
  /*  make sure the file is there. */
  if ((file = fopen(filename, "rb"))==NULL) {
    printf("File Not Found : %s\n",filename);
    return 0;
  }
  
  /*  seek through the bmp header, up to the width height: */
  fseek(file, 18, SEEK_CUR);
  
  /*  No 100% errorchecking anymore!!! */
  
  /*  read the width */    image->sizeX = getint (file);
  
  /*  read the height */ 
  image->sizeY = getint (file);
  
  /*  calculate the size (assuming 24 bits or 3 bytes per pixel). */
  size = image->sizeX * image->sizeY * 3;
  
  /*  read the planes */    
  planes = getshort(file);
  if (planes != 1) {
    printf("Planes from %s is not 1: %u\n", filename, planes);
    return 0;
  }
  
  /*  read the bpp */    
  bpp = getshort(file);
  if (bpp != 24) {
    printf("Bpp from %s is not 24: %u\n", filename, bpp);
    return 0;
  }
  
  /*  seek past the rest of the bitmap header. */
  fseek(file, 24, SEEK_CUR);
  
  /*  read the data.  */
  image->data = (char *) malloc(size);
  if (image->data == NULL) {
    printf("Error allocating memory for color-corrected image data");
    return 0; 
  }
  
  if ((i = fread(image->data, size, 1, file)) != 1) {
    printf("Error reading image data from %s.\n", filename);
    return 0;
  }
  
  for (i=0;i<size;i+=3) { /*  reverse all of the colors. (bgr -> rgb) */
    temp = image->data[i];
    image->data[i] = image->data[i+2];
    image->data[i+2] = temp;
  }
  
  fclose(file); /* Close the file and release the filedes */
  
  /*  we're done. */
  return 1;
}