/*
 *  Lab exercise 2.0 
 *  CSc 474, Computer Graphics
 *  Chris Buckalew, modified from Jason L. McKesson, Ed Angel, and others
 *
 *  need at least 4 primitives as well four materals or colors
 *  Make a translation, Simple harmoic Motion, Pendulum Motion, and spirl motion - other motions if i want = 75%
 *  for 25 more i need to add a character to my animation, meaning put an expression
 *------------------------------------------------------------*/

#include <string>
#include <vector>
#include <stack>
#include <math.h>
#include <stdio.h>
//#include <GL/freeglut.h>
#include <GL/glut.h>
#include "../glm/glm.hpp"
#include "../glm/ext.hpp"
#include <fstream>
#include <sstream>
#include <exception>
#include <stdexcept>
#include <string.h>
#include <algorithm>
#include <string>
#include <vector>
#include "Sphere.cpp"
#include "util.cpp"

// prototypes and variables associated with the trackball viewer
void mouseCallback(int button, int state, int x, int y);
void mouseMotion(int x, int y);

bool trackballEnabled = true;
bool trackballMove = false;
bool trackingMouse = false;
bool redrawContinue = false;
bool zoomState = false;
bool shiftState = false;

int winWidth, winHeight;
float angle = 0.0, axis[3];

float lightXform[4][4] = {
   {1.0, 0.0, 0.0, 0.0},
   {0.0, 1.0, 0.0, 0.0},
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 1.0}
};

float objectXform[4][4] = {
   {1.0, 0.0, 0.0, 0.0},
   {0.0, 1.0, 0.0, 0.0},
   {0.0, 0.0, 1.0, 0.0},
   {0.0, 0.0, 0.0, 1.0}
};

float *objectXformPtr = (float *)objectXform;
float *lightXformPtr = (float *)lightXform;
float *trackballXform = (float *)objectXform;

// initial viewer position
static float modelTrans[] = {0.0f, 0.0f, -10.0f};

struct ProgramData
{
   // info for accessing the shaders
   GLuint theProgram;
   GLuint modelToWorldMatrixUnif;
   GLuint worldToCameraMatrixUnif;
   GLuint cameraToClipMatrixUnif;
   GLuint worldSpaceMoveMatrixUnif;
   GLuint lightIntensityUnif;
   GLuint ambientIntensityUnif;
   GLuint cameraSpaceLightPosUnif;
   GLuint shininessFactorUnif;
   GLuint diffuseColorUnif;
};

float nearClipPlane = 0.1f;
float farClipPlane = 1000.0f;

// buffers used to communicate with the GPU
GLuint vertexBufferObject;
GLuint indexBufferObject;
GLuint vao;

ProgramData PhongShade;

ProgramData LoadProgram(const std::string &strVertexShader, const std::string &strFragmentShader)
{
   std::vector<GLuint> shaderList;

   shaderList.push_back(LoadShader(GL_VERTEX_SHADER, strVertexShader));
   shaderList.push_back(LoadShader(GL_FRAGMENT_SHADER, strFragmentShader));

   ProgramData data;
   data.theProgram = CreateProgram(shaderList);

   // the uniforms needed for the shaders
   data.modelToWorldMatrixUnif = glGetUniformLocation(data.theProgram, "modelToWorldMatrix");
   data.worldToCameraMatrixUnif = glGetUniformLocation(data.theProgram, "worldToCameraMatrix");
   data.cameraToClipMatrixUnif = glGetUniformLocation(data.theProgram, "cameraToClipMatrix");
   data.worldSpaceMoveMatrixUnif = glGetUniformLocation(data.theProgram, "worldSpaceMoveMatrix");
   data.lightIntensityUnif = glGetUniformLocation(data.theProgram, "lightIntensity");
   data.ambientIntensityUnif = glGetUniformLocation(data.theProgram, "ambientIntensity");
   data.cameraSpaceLightPosUnif = glGetUniformLocation(data.theProgram, "cameraSpaceLightPos");
   data.shininessFactorUnif = glGetUniformLocation(data.theProgram, "shininessFactor");
   data.diffuseColorUnif = glGetUniformLocation(data.theProgram, "diffuseColor");

   return data;
}

void InitializeProgram()
{
   // load and compile shaders, link the program and return it
   PhongShade = LoadProgram("PCN.vert", "PhongLighting.frag");
}

// set up the sphere statics in case there's going to be a sphere
// can remove these lines if no sphere
bool Sphere::sphereInitialized = false;
GLuint Sphere::sphereVao = vao;
GLuint Sphere::sphereVertexBufferObject = vertexBufferObject;
GLuint Sphere::sphereIndexBufferObject = indexBufferObject;

// two variables you may need for retracing the path
float lastTimeP = 0.0f;
bool retrace = false;

// this function returns a transform matrix that is applied to an object to be moved.  The input is a time
// parameter and the transform is initially the identity
glm::mat4 moveObjLinearInterp(float timeP) {
   glm::mat4 moveMatrix;

   if (timeP < 0.0f) {
      // don't move anything
      return moveMatrix;
   }

   // time to toggle direction?
   if (timeP<lastTimeP && retrace==false) retrace = true;
   else if (timeP<lastTimeP && retrace==true) retrace = false;

   lastTimeP = timeP;
   if (retrace==true) timeP = 1.0f-timeP;

   moveMatrix = glm::translate(moveMatrix, glm::vec3(2*pow(2.718,(-1*(timeP-1)))* 2 *cos(timeP*3.14*2*5), 0.0f, 0.0f));

   return moveMatrix;
}
glm::mat4 moveObjLinearInterpOrg(float timeP) {
   glm::mat4 moveMatrix;

   if (timeP < 0.0f) {
      // don't move anything
      return moveMatrix;
   }

   // time to toggle direction?
   if (timeP<lastTimeP && retrace==false) retrace = true;
   else if (timeP<lastTimeP && retrace==true) retrace = false;

   lastTimeP = timeP;
   if (retrace==true) timeP = 1.0f-timeP;

   moveMatrix = glm::translate(moveMatrix, glm::vec3(-5,-5+ timeP*(5- -5), 0.0f));

   return moveMatrix;
}
glm::mat4 moveObjEyes(float timeP) {
   glm::mat4 moveMatrix;

   if (timeP < 0.0f) {
      // don't move anything
      return moveMatrix;
   }

   // time to toggle direction?
   if (timeP<lastTimeP && retrace==false) retrace = true;
   else if (timeP<lastTimeP && retrace==true) retrace = false;

   lastTimeP = timeP;
   if (retrace==true) timeP = 1.0f-timeP;

   moveMatrix = glm::translate(moveMatrix, glm::vec3(-5,-5+ timeP*(5- -5), 0.0f));
   moveMatrix = glm::scale(moveMatrix, glm::vec3(1.0f, 1.0f*timeP, 1.0f));

   return moveMatrix;
}
glm::mat4 moveObjMouth(float timeP) {
   glm::mat4 moveMatrix;

   if (timeP < 0.0f) {
      // don't move anything
      return moveMatrix;
   }

   // time to toggle direction?
   if (timeP<lastTimeP && retrace==false) retrace = true;
   else if (timeP<lastTimeP && retrace==true) retrace = false;

   lastTimeP = timeP;
   if (retrace==true) timeP = 1.0f-timeP;

   moveMatrix = glm::translate(moveMatrix, glm::vec3(-5,-5+ timeP*(5- -5), 0.0f));
   moveMatrix = glm::scale(moveMatrix, glm::vec3(1.05f, 1.05f*timeP, 1.05f)); // scale is different from the marker

   return moveMatrix;
}
glm::mat4 moveObjPend(float timeP) {
   glm::mat4 moveMatrix;

   if (timeP < 0.0f) {
      // don't move anything
      return moveMatrix;
   }

   // time to toggle direction?
   if (timeP<lastTimeP && retrace==false) retrace = true;
   else if (timeP<lastTimeP && retrace==true) retrace = false;

   lastTimeP = timeP;
   if (retrace==true) timeP = 1.0f-timeP;

   moveMatrix = glm::translate(moveMatrix, glm::vec3(3*cos(timeP*30), -1*sqrt(pow(4,2) - pow(3*cos(timeP*30),2)), 0.0f));

   return moveMatrix;
}

glm::mat4 moveObjSprial(float timeP) {
   glm::mat4 moveMatrix;

   if (timeP < 0.0f) {
      // don't move anything
      return moveMatrix;
   }

   // time to toggle direction?
   if (timeP<lastTimeP && retrace==false) retrace = true;
   else if (timeP<lastTimeP && retrace==true) retrace = false;

   lastTimeP = timeP;
   if (retrace==true) timeP = 1.0f-timeP;

   moveMatrix = glm::translate(moveMatrix, glm::vec3(2*timeP*cos(timeP*2*3.14*10), 2*timeP*sin(timeP *2*3.14* 10), 0.0f));

   return moveMatrix;
}

glm::vec3 linearColor(float timeP) {

   if (retrace==true) timeP = 1.0f-timeP;

   float interpR = 0.0f + timeP*(0.2f - 1.0f);
   float interpG = 0.0f + timeP*(1.0f - 0.2f);
   float interpB = 0.0f + timeP*(0.2f - 0.2f);

   return glm::vec3(interpR, interpG, interpB);
}
glm::vec3 eyeColor(float timeP) {

   if (retrace==true) timeP = 1.0f-timeP;

   float interpR = 0.2f + timeP*(0.2f - 1.0f);
   float interpG = 0.2f + timeP*(1.0f - 0.2f);
   float interpB = 8.8f + timeP*(0.2f - 0.2f);

   return glm::vec3(interpR, interpG, interpB);
}

glm::vec3 sqrColor(float timeP) {

   if (retrace==true) timeP = 1.0f-timeP;

   float interpR = 1.0f + (3*cos(timeP*30))*(0.2f - 1.0f);
   float interpG = 0.2f + (3*cos(timeP*30))*(1.0f - 0.2f);
   float interpB = 0.2f + (3*cos(timeP*30))*(0.2f - 0.2f);

   return glm::vec3(interpR, interpG, interpB);
}

glm::vec3 sineColor(float timeP) {

   if (retrace==true) timeP = 1.0f-timeP;

   float interpR = 0.2f + 3*sin(timeP * 30)*3*cos(timeP *30) *(0.2f - 0.2f);
   float interpG = 0.2f + 3*sin(timeP * 30)*3*cos(timeP *30) *(1.0f - 0.2f);
   float interpB = 1.0f + 3*sin(timeP * 30)*3*cos(timeP *30)*(0.2f - 1.0f);

   return glm::vec3(interpR, interpG, interpB);
}
//Called after the window and OpenGL are initialized. Called exactly once, before the main loop.
void init()
{
   glutMouseFunc(mouseCallback);
   glutMotionFunc(mouseMotion);
   InitializeProgram();

   glGenVertexArrays(1, &vao);
   glBindVertexArray(vao);

   glEnable(GL_CULL_FACE);
   glCullFace(GL_BACK);
   glFrontFace(GL_CW);

   glEnable(GL_DEPTH_TEST);
   glDepthMask(GL_TRUE);
   glDepthFunc(GL_LEQUAL);
   glDepthRange(0.0f, 1.0f);
   glEnable(GL_DEPTH_CLAMP);
}

// marker sphere to the left - the transform passed in is the identity in this case
void drawFirstSphere(glm::mat4 modelMatrix) 
{
   Sphere *sphere1 = new Sphere();
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjLinearInterp(-1.0))); // don't move it

   
   //modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
   // translate to the left 5 units
   modelMatrix = glm::translate(modelMatrix, glm::vec3(-5.0f, -0.25f, 0.0f));
   modelMatrix = glm::rotate(modelMatrix, 90.0f ,glm::vec3(-1,0,0));

   // scale it small - recall that transforms are applied in reverse coding order, so this scale occurs first
   modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));

   // now set the transform in the shader
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));

   // color
   glUniform4f(PhongShade.diffuseColorUnif, 1.0f, 0.2f, 0.2f, 1.0f); // low saturation red

   // couple of other reflectance parameters
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   // tells the shader to draw it
   glutSolidTeapot(2);
   //sphere1->DrawUnitSphere();
} 

// this is the top moving sphere
void drawTopSphere(glm::mat4 modelMatrix, glm::vec3 color, float timeParameter) 
{
   Sphere *sphereTop = new Sphere();

   // this time send the shader a non-identity transform to move in world space
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjPend(timeParameter)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 6.0f, 0.0f));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.5f, 1.0f)); // scale is different from the marker
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, color[0], color[1], color[2], 1.0f); // color is different from the marker
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   sphereTop->DrawUnitSphere();
}

void drawMiddleSphere(glm::mat4 modelMatrix, glm::vec3 color, float timeParameter) 
{
   Sphere *sphereMiddle = new Sphere();

   // this time send the shader a non-identity transform to move in world space
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjLinearInterp(timeParameter)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
   modelMatrix = glm::rotate(modelMatrix, 90.0f, glm::vec3(1,0,0));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.5f, 1.0f)); // scale is different from the marker
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, color[0], color[1], color[2], 1.0f); // color is different from the marker
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   sphereMiddle->DrawUnitSphere();
}
void eyeLeft(glm::mat4 modelMatrix, glm::vec3 color, float timeParameter) 
{
   Sphere *sphereMiddle = new Sphere();

   // this time send the shader a non-identity transform to move in world space
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjEyes(timeParameter)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.5f, 0.4f, 0.4f));
   modelMatrix = glm::rotate(modelMatrix, 90.0f, glm::vec3(1,0,0));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, .1f, 0.1f)); // scale is different from the marker
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, color[0], color[1], color[2], 1.0f); // color is different from the marker
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   sphereMiddle->DrawUnitSphere();
}
void eyeRight(glm::mat4 modelMatrix, glm::vec3 color, float timeParameter) 
{
   Sphere *sphereMiddle = new Sphere();

   // this time send the shader a non-identity transform to move in world space
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjEyes(timeParameter)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(0.5f, 0.4f, 0.4f));
   modelMatrix = glm::rotate(modelMatrix, 90.0f, glm::vec3(1,0,0));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1f, 0.1f, 0.1f)); // scale is different from the marker
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, color[0], color[1], color[2], 1.0f); // color is different from the marker
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   sphereMiddle->DrawUnitSphere();
}
void drawMouth(glm::mat4 modelMatrix, glm::vec3 color, float timeParameter) 
{
   Sphere *sphereMiddle = new Sphere();

   // this time send the shader a non-identity transform to move in world space
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjMouth(timeParameter)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -0.4f, 0.57f));
   modelMatrix = glm::rotate(modelMatrix, 190.0f, glm::vec3(1,0,0));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(0.05f, 0.05f, 0.05f)); // scale is different from the marker
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, color[0], color[1], color[2], 1.0f); // color is different from the marker
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   glutSolidTorus(2,4 , 100, 100);
}
void drawMiddleSphere2(glm::mat4 modelMatrix, glm::vec3 color, float timeParameter) 
{
   Sphere *sphereMiddle = new Sphere();

   // this time send the shader a non-identity transform to move in world space
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjLinearInterpOrg(timeParameter)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, 0.0f, 0.0f));
   modelMatrix = glm::rotate(modelMatrix, 90.0f, glm::vec3(1,0,0));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(1.0f, 0.5f, 1.0f)); // scale is different from the marker
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, color[0], color[1], color[2], 1.0f); // color is different from the marker
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   sphereMiddle->DrawUnitSphere();
}

void drawBottomSphere(glm::mat4 modelMatrix, glm::vec3 color, float timeParameter) 
{
   Sphere *sphereBottom = new Sphere();

   // this time send the shader a non-identity transform to move in world space
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjSprial(timeParameter)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f, -5.0f, 0.0f));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(0.2f, 0.2f, 0.2f)); // scale is different from the marker
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, color[0], color[1], color[2], 1.0f); // color is different from the marker
   glUniform4f(PhongShade.ambientIntensityUnif, 0.2f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 20.0f);

   glutSolidTorus(2,4 , 100, 100);
   
   //sphereBottom->DrawUnitSphere();
}

// the right marker sphere
void drawThirdSphere(glm::mat4 modelMatrix) 
{
   Sphere *sphere3 = new Sphere();
   glUniformMatrix4fv(PhongShade.worldSpaceMoveMatrixUnif, 1, GL_FALSE, glm::value_ptr(moveObjLinearInterp(-1.0)));

   modelMatrix = glm::translate(modelMatrix, glm::vec3(5.0f, 0.0f, 0.0f));
   modelMatrix = glm::scale(modelMatrix, glm::vec3(0.25f, 0.25f, 0.25f));
   glUniformMatrix4fv(PhongShade.modelToWorldMatrixUnif, 1, GL_FALSE, glm::value_ptr(modelMatrix));
   glUniform4f(PhongShade.diffuseColorUnif, 1.0f, 0.2f, 0.2f, 1.0f);
   glUniform4f(PhongShade.ambientIntensityUnif, 0.8f, 0.2f, 0.2f, 1.0f);
   glUniform1f(PhongShade.shininessFactorUnif, 50.0f);

   glutSolidCube(2);
   //sphere3->DrawUnitSphere();
}

//Called to update the display.
//You should call glutSwapBuffers after all of your rendering to display what you rendered.
//If you need continuous updates of the screen, call glutPostRedisplay() at the end of the function.
void display()
{
   const float epsilon = 0.001f; // necessary for the trackball viewer
   const float period = 10.0;  // repeat time in seconds of movement
   float fElapsedTime = glutGet(GLUT_ELAPSED_TIME) / 1000.0f; // time since programs started in seconds
   float timeParameter = (float) fmodf(fElapsedTime, period)/period; // parameterized time
   //float timeParameter = 0.0f; // no animation - also comment out the glutPostRedisplay() at the bottom of this function

   glClearColor(0.0f, 0.0f, 0.2f, 0.0f); // unshaded objects will show up on this almost-black background
   glClearDepth(1.0f);
   glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

   // light stuff
   glUniform3f(PhongShade.cameraSpaceLightPosUnif, 0.0, 0.0, 100.0);  // this is a headlight
   glUniform4f(PhongShade.lightIntensityUnif, 1.0f, 1.0f, 1.0f, 1.0f); // light color

   // compute trackball interface (world to camera) transform --------------------------------
   // don't change any of this (unless you can make it better!)
   glm::mat4 camMatrix;
   glm::mat4 cam2Matrix;
   camMatrix = glm::translate(camMatrix, glm::vec3(modelTrans[0], modelTrans[1], modelTrans[2]));
   // in the middle of a left-button drag
   if (trackballMove) {
      // check to make sure axis is not zero vector
      if (!(-epsilon < axis[0] && axis[0] < epsilon && -epsilon < axis[1] && axis[1] < epsilon && -epsilon < axis[2] && axis[2] < epsilon)) {

         cam2Matrix = glm::rotate(cam2Matrix, angle, glm::vec3(axis[0], axis[1], axis[2]));
         cam2Matrix = cam2Matrix * (glm::mat4(trackballXform[0], trackballXform[1], trackballXform[2], trackballXform[3], 
                                              trackballXform[4], trackballXform[5], trackballXform[6], trackballXform[7],
                                              trackballXform[8], trackballXform[9], trackballXform[10], trackballXform[11],
                                              trackballXform[12], trackballXform[13], trackballXform[14], trackballXform[15]));
         glm::mat4 tempM = cam2Matrix;
         // copy current transform back into trackball matrix
         for (int i=0; i<4; i++) {
            for (int j=0; j<4; j++) 
               trackballXform[i*4 + j] = tempM[i][j];
         }
      }
   }
   camMatrix = camMatrix * (glm::mat4(objectXformPtr[0], objectXformPtr[1], objectXformPtr[2], objectXformPtr[3], 
                                      objectXformPtr[4], objectXformPtr[5], objectXformPtr[6], objectXformPtr[7],
                                      objectXformPtr[8], objectXformPtr[9], objectXformPtr[10], objectXformPtr[11],
                                      objectXformPtr[12], objectXformPtr[13], objectXformPtr[14], objectXformPtr[15]));
   // end of world to camera transform -----------------------------------------------

   // pass it on to the shaders
   glUniformMatrix4fv(PhongShade.worldToCameraMatrixUnif, 1, GL_FALSE, glm::value_ptr(camMatrix));

   // draw objects - pass in the transform matrix to use
   glm::mat4 modelMatrix;  //starts with identity

   drawFirstSphere(modelMatrix); // modelMatrix is not modified by the draw functions
   drawTopSphere(modelMatrix, sqrColor(timeParameter), timeParameter);
   drawMiddleSphere(modelMatrix, linearColor(timeParameter), timeParameter);
   drawMiddleSphere2(modelMatrix, linearColor(timeParameter), timeParameter);
   eyeRight(modelMatrix, eyeColor(timeParameter), timeParameter);
   eyeLeft(modelMatrix, eyeColor(timeParameter), timeParameter);
   drawMouth(modelMatrix, linearColor(timeParameter), timeParameter);
   
   drawBottomSphere(modelMatrix, sineColor(timeParameter), timeParameter);
   drawThirdSphere(modelMatrix);

   glutSwapBuffers();
   glutPostRedisplay();
}

//Called whenever the window is resized. The new window size is given, in pixels.
//This is an opportunity to call glViewport or glScissor to keep up with the change in size.
void reshape (int w, int h)
{
   glm::mat4 persMatrix = glm::perspective(45.0f, (w / (float)h), nearClipPlane, farClipPlane);
   winWidth = w;
   winHeight = h;

   glUseProgram(PhongShade.theProgram);
   glUniformMatrix4fv(PhongShade.cameraToClipMatrixUnif, 1, GL_FALSE, glm::value_ptr(persMatrix));

   glViewport(0, 0, (GLsizei) w, (GLsizei) h);
   glutPostRedisplay();
}

// Trackball-like interface functions - no need to ever change any of this--------------------------------------------------
float lastPos[3] = {0.0, 0.0, 0.0};
int curx, cury;
int startX, startY;

void trackball_ptov(int x, int y, int width, int height, float v[3]) {
   float d, a;
   // project x, y onto a hemisphere centered within width, height
   v[0] = (2.0f*x - width) / width;
   v[1] = (height - 2.0f*y) / height;
   d = (float) sqrt(v[0]*v[0] + v[1]*v[1]);
   v[2] = (float) cos((3.14159f/2.0f) * ((d<1.0f)? d : 1.0f));
   a = 1.0f / (float) sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
   v[0] *= a;
   v[1] *= a;
   v[2] *= a;
}

void mouseMotion(int x, int y) {
   float curPos[3], dx, dy, dz;

   if (zoomState == false && shiftState == false) {

      trackball_ptov(x, y, winWidth, winHeight, curPos);

      dx = curPos[0] - lastPos[0];
      dy = curPos[1] - lastPos[1];
      dz = curPos[2] - lastPos[2];

      if (dx||dy||dz) {
         angle = 90.0f * sqrt(dx*dx + dy*dy + dz*dz);

         axis[0] = lastPos[1]*curPos[2] - lastPos[2]*curPos[1];
         axis[1] = lastPos[2]*curPos[0] - lastPos[0]*curPos[2];
         axis[2] = lastPos[0]*curPos[1] - lastPos[1]*curPos[0];

         lastPos[0] = curPos[0];
         lastPos[1] = curPos[1];
         lastPos[2] = curPos[2];
      }

   }
   else if (zoomState == true) {
      curPos[1] = (float) y;
      dy = curPos[1] - lastPos[1];

      if (dy) {
         modelTrans[2] += dy * 0.01f;
         lastPos[1] = curPos[1];
      }
   }
   else if (shiftState == true) {
      curPos[0] = (float) x;
      curPos[1] =(float)  y;
      dx = curPos[0] - lastPos[0];
      dy = curPos[1] - lastPos[1];

      if (dx) {
         modelTrans[0] += dx * 0.01f;
         lastPos[0] = curPos[0];
      }
      if (dy) {
         modelTrans[1] -= dy * 0.01f;
         lastPos[1] = curPos[1];
      }
   }
   glutPostRedisplay( );
}

void startMotion(long time, int button, int x, int y) {
   if (!trackballEnabled) return;

   trackingMouse = true;
   redrawContinue = false;
   startX = x; startY = y;
   curx = x; cury = y;
   trackball_ptov(x, y, winWidth, winHeight, lastPos);
   trackballMove = true;
}

void stopMotion(long time, int button, int x, int y) {
   if (!trackballEnabled) return;

   trackingMouse = false;

   if (startX != x || startY != y)
      redrawContinue = true;
   else {
      angle = 0.0f;
      redrawContinue = false;
      trackballMove = false;
   }
}

// Called when a mouse button is pressed or released
void mouseCallback(int button, int state, int x, int y) {

   switch (button) {
      case GLUT_LEFT_BUTTON:
         trackballXform = (float *)objectXform;
         break;
      case GLUT_RIGHT_BUTTON:
      case GLUT_MIDDLE_BUTTON:
         trackballXform = (float *)lightXform;
         break;
   }
   switch (state) {
      case GLUT_DOWN:
         if (button == GLUT_RIGHT_BUTTON) {
            zoomState = true;
            lastPos[1] = (float) y;
         }
         else if (button == GLUT_MIDDLE_BUTTON) {
            shiftState = true;
            lastPos[0] = (float) x;
            lastPos[1] = (float) y;
         }
         else startMotion(0, 1, x, y);
         break;
      case GLUT_UP:
         trackballXform = (float *)lightXform; // turns off mouse effects
         if (button == GLUT_RIGHT_BUTTON) {
            zoomState = false;
         }
         else if (button == GLUT_MIDDLE_BUTTON) {
            shiftState = false;
         }
         else stopMotion(0, 1, x, y);
         break;
   }
}

// end of trackball mouse functions--------------------------------------------------------------------------


//Called whenever a key on the keyboard was pressed.
//The key is given by the ''key'' parameter, which is in ASCII.
void keyboard(unsigned char key, int x, int y)
{
   switch (key) {
      case 27:
         return;
      case 'z': modelTrans[2] += 1.0f; break;
      case 'Z': modelTrans[2] -= 1.0f; break;
      case 'x': modelTrans[0] += 1.0f; break;
      case 'X': modelTrans[0] -= 1.0f; break;
      case 'y': modelTrans[1] += 1.0f; break;
      case 'Y': modelTrans[1] -= 1.0f; break;
      case 'h': 
         lightXformPtr[0] = objectXformPtr[0] = lightXformPtr[5] = objectXformPtr[5] =
         lightXformPtr[10] = objectXformPtr[10] = lightXformPtr[15] = objectXformPtr[15] = 1.0f;
         lightXformPtr[1] = objectXformPtr[1] = lightXformPtr[2] = objectXformPtr[2] = lightXformPtr[3] = objectXformPtr[3] =
         lightXformPtr[4] = objectXformPtr[4] = lightXformPtr[6] = objectXformPtr[6] = lightXformPtr[7] = objectXformPtr[7] =
         lightXformPtr[8] = objectXformPtr[8] = lightXformPtr[9] = objectXformPtr[9] = lightXformPtr[11] = objectXformPtr[11] =
         lightXformPtr[12] = objectXformPtr[12] = lightXformPtr[13] = objectXformPtr[13] = lightXformPtr[14] = objectXformPtr[14] = 0.0;
         modelTrans[0] = modelTrans[1] = 0.0; modelTrans[2] = -10.0;
         axis[0] = axis[1] = axis[2] = 0.0;
         angle = 0;
         break;
      case 'd':
         break;
   }
   glutPostRedisplay();
}

int main(int argc, char** argv)
{
   glutInit( &argc, argv );
   glutInitWindowPosition( 20, 20 );
   glutInitWindowSize( 500, 500 );
   glutInitDisplayMode( GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH );
   glutCreateWindow("Lab 2 - Interpolated Motion");

   //test the openGL version
   getGLversion();

   init();
   glutDisplayFunc(display); 
   glutReshapeFunc(reshape);
   glutKeyboardFunc(keyboard);
   glutMainLoop();
   return 0;
}
