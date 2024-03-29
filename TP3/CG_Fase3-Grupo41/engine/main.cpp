﻿#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#endif
#define _USE_MATH_DEFINES
#include <math.h>
#include <vector>
#include <fstream>
#include <iostream>
#include "../tinyxml.h"
#include "../include/xml_parse.h"
using namespace std;
using namespace myXML;

float camX, camY, camZ;
float alpha = 0.0f, beta = 0.0f, radius = 5.0f;

xml_parse* xmlParse = new xml_parse();
char* filename;

struct vertex {
    float x, y, z;
};

vector<vertex> allvertices;
vector<vertex> vertices;
vector< TiXmlElement*> groupStack;

vector<char*> filesRead;
vector<const char*> models;
vector<vector<vertex> > v_matrix;

vector<float> planetsPos;
vector<float> planetsAngle;

GLuint *buffers;

vector<int> sizes;

int countCurves;
int countRotate;
int current_index = 0;

// Translate Catmull
static float t=0;
float pos[4];
float deriv[4];

float xi[4];
float zi[4] = {0,0,0};
float y[4] = {0,1,0};
float y_prev[3] = {0,1,0};

int curr_time_trans;
int t_prev_trans = 0;

int curr_time_rot;
int t_prev_rot = 0;

float time_multiplier = 1;

int interval;

bool showCurve = true;

void spherical2Cartesian() {
    camX = radius * cos(beta) * sin(alpha);
    camY = radius * sin(beta);
    camZ = radius * cos(beta) * cos(alpha);
}

void buildRotMatrix(float *x, float *y, float *z, float *m) {
    m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
    m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
    m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}

void cross(float *a, float *b, float *res) {
    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
}


void normalize(float *a) {

    float l = sqrt(a[0]*a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] = a[0]/l;
    a[1] = a[1]/l;
    a[2] = a[2]/l;
}


void processKeys(unsigned char c, int xx, int yy) {

// put code to process regular keys in here
    switch(c){
        case 'l':
            time_multiplier *= 2;
            break;
        case 'r':
            time_multiplier = time_multiplier/2;
            break;
        case 's':
            if(!showCurve) showCurve = true;
            else{
                showCurve = false;
            }
    }
}


void processSpecialKeys(int key, int xx, int yy){
    switch(key){
        case GLUT_KEY_RIGHT:
            alpha -= 0.1f;
            break;
        case GLUT_KEY_LEFT:
            alpha += 0.1f;
            break;
        case GLUT_KEY_UP:
            beta += 0.1f;
            if (beta > 1.5f)
                beta = 1.5f;
            break;
        case GLUT_KEY_DOWN:
            beta -= 0.1f;
            if (beta < -1.5f)
                beta = -1.5f;
            break;
        case GLUT_KEY_PAGE_DOWN:
            radius -= 10.0f;
            if(radius < 0.1f)
                radius = 1.0f;
            break;
        case GLUT_KEY_PAGE_UP:
            radius += 10.0f;
            break;

    }
    spherical2Cartesian();
    glutPostRedisplay();
}


void changeSize(int w, int h) {

    // Prevent a divide by zero, when window is too short
    // (you cant make a window with zero width).
    if (h == 0)
        h = 1;

    // compute window's aspect ratio
    float ratio = w * 1.0 / h;

    // Set the projection matrix as current
    glMatrixMode(GL_PROJECTION);
    // Load Identity Matrix
    glLoadIdentity();

    // Set the viewport to be the entire window
    glViewport(0, 0, w, h);

    // Set perspective
    gluPerspective(xmlParse->fov, ratio, xmlParse->near, xmlParse->far);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}


void tokenize(std::string const& str, const char delim,
              std::vector<std::string>& out)
{
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos)
    {
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}


// parse de ficheiros .3d
void parser(const char* filename) {

    string line;
    ifstream myfile(filename);

    const char delimiter = ';';
    std::vector<std::string> out;

    if (myfile.is_open())
    {
        while (getline(myfile, line))
        {
            tokenize(line, delimiter, out); //ler linha do ficheiro
        }
        myfile.close();
    }

    else std::cout << "Unable to open file";


    for (std::size_t i = 0; i < out.size(); i += 3) {
        vertex v1 = {
                std::stof(out[i]),
                std::stof(out[i + 1]),
                std::stof(out[i + 2])
        };
        vertices.push_back(v1);
    }
    vector<float> tmp;
    for(int i = 0; i < vertices.size(); i++){
        tmp.push_back(vertices[i].x);
        tmp.push_back(vertices[i].y);
        tmp.push_back(vertices[i].z);
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffers[filesRead.size()-1]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, tmp.size()*sizeof(float), tmp.data(), GL_STATIC_DRAW);

    sizes.push_back(tmp.size());

    vertices.clear();
}


void drawPrimitive(int fileIndex){
    glColor3f(1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[fileIndex]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_TRIANGLES, 0, sizes.at(fileIndex));
}


void multMatrixVector(float *m, float *v, float *res) {

    for (int j = 0; j < 4; ++j) {
        res[j] = 0;
        for (int k = 0; k < 4; ++k) {
            res[j] += v[k] * m[j * 4 + k];
        }
    }
}


void getCatmullRomPoint(float t, float *p0, float *p1, float *p2, float *p3, float *pos, float *deriv) {

    // catmull-rom matrix
    float m[4][4] = {	{-0.5f,  1.5f, -1.5f,  0.5f},
                         { 1.0f, -2.5f,  2.0f, -0.5f},
                         {-0.5f,  0.0f,  0.5f,  0.0f},
                         { 0.0f,  1.0f,  0.0f,  0.0f}};

    // Compute A = M * P
    float a[4];
    float tVector[4] = {pow(t, 3.0f), pow(t, 2.0f), t, 1};
    float tVectorL[4] = {3*pow(t, 2.0f), 2*t, 1, 0};

    for (int i = 0; i < 3; i++){ //for each component
        float ponto[4] = {p0[i], p1[i], p2[i], p3[i]};

        multMatrixVector((float *) m, ponto, a);

        // Compute pos = T * A

        pos[i] = tVector[0] * a[0] + tVector[1] * a[1] + tVector[2] * a[2] + tVector[3] * a[3];

        // compute deriv = T' * A

        deriv[i] = tVectorL[0] * a[0] + tVectorL[1] * a[1] + tVectorL[2] * a[2];
    }

    // ...
}


// given  global t, returns the point in the curve
void getGlobalCatmullRomPoint(float gt, float *pos, float *deriv) {

    int n_points = (int) xmlParse->nrPointsPerCurve.at(countCurves);
    // Points that make up the loop for catmull-rom interpolation

    float t = gt * n_points; // this is the real global t
    int index = floor(t);  // which segment
    t = t - index; // where within  the segment

    // indices store the points
    int indices[n_points];

    indices[0] = (index+n_points-1)%n_points;
    indices[1] = (indices[0]+1)%n_points;
    indices[2] = (indices[1]+1)%n_points;
    indices[3] = (indices[2]+1)%n_points;

   //for(int i = 0; i < xmlParse->controlPoints.size(); i++){
        //rintf("Ponto %d: %f %f %f\n", i, xmlParse->controlPoints.at(i)[0], xmlParse->controlPoints.at(i)[1], xmlParse->controlPoints.at(i)[2]);
    //}

    float* p0 = xmlParse->controlPoints.at(current_index + indices[0]);
    float* p1 = xmlParse->controlPoints.at(current_index + indices[1]);
    float* p2 = xmlParse->controlPoints.at(current_index + indices[2]);
    float* p3 = xmlParse->controlPoints.at(current_index + indices[3]);

    getCatmullRomPoint(t, p0, p1, p2, p3, pos, deriv);
}

void renderCatmullRomCurve() {
    // draw curve using line segments with GL_LINE_LOOP
    float pos[4];
    float deriv[4];


    vector<float> x;

    for (int i = 0; i < 100; ++i) {
        getGlobalCatmullRomPoint(i / 100.0f, pos, deriv);
        x.push_back(pos[0]);
        x.push_back(pos[1]);
        x.push_back(pos[2]);
    }

    glBindBuffer(GL_ARRAY_BUFFER, buffers[models.size()]);
    glBufferData(GL_ARRAY_BUFFER, x.size()*sizeof(float), x.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[models.size()]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_LINE_LOOP, 0, x.size());

}


/*
    1 -> translate
    2 -> rotate
    3 -> scale
    4 -> pushMatrix
    5 -> popMatrix
    6-> model
    7-> translate catmull
    8-> rotate catmull
*/

void draw() {
    int modelInd = 0;
    countCurves = 0;
    countRotate = 0;
    current_index = 0;
    bool exists;
    int n_points;


    for(int i=0 ; i<xmlParse->transformations.size() ; i++) {

        switch ((int) xmlParse->transformations[i]) {

            case 1:
                glTranslatef(xmlParse->transformations[i + 1], xmlParse->transformations[i + 2],
                             xmlParse->transformations[i + 3]);
                i += 3;
                break;

            case 2:
                glRotatef(xmlParse->transformations[i + 1], xmlParse->transformations[i + 2],
                          xmlParse->transformations[i + 3], xmlParse->transformations[i + 4]);
                i += 4;
                break;

            case 3:
                glScalef(xmlParse->transformations[i + 1], xmlParse->transformations[i + 2],
                         xmlParse->transformations[i + 3]);
                i += 3;
                break;

            case 4:
                glPushMatrix();
                break;

            case 5:
                glPopMatrix();
                break;

            case 6:
                exists = false;
                for (int j = 0; j < filesRead.size(); j++) {
                    if (strcmp(xmlParse->models[modelInd], filesRead[j]) == 0) {
                        drawPrimitive(j);
                        exists = true;
                    }
                }
                if (!exists) {

                    filesRead.push_back((char *) xmlParse->models[modelInd]);
                    parser(xmlParse->models[modelInd]);
                    drawPrimitive(filesRead.size() - 1);
                }
                modelInd++;
                break;

            case 7:
                if(showCurve) renderCatmullRomCurve();
                getGlobalCatmullRomPoint(planetsPos.at(countCurves), pos, deriv);

                n_points = (int) xmlParse->nrPointsPerCurve.at(countCurves);
                current_index += n_points;

                if (strcmp(xmlParse->translateAligns[countCurves], "True") == 0) {
                    for (int i = 0; i < 3; i++) {
                        xi[i] = deriv[i];
                    }
                    normalize(xi);
                    normalize(y);
                    normalize(y_prev);
                    normalize(zi);

                    cross(xi, y_prev, zi);
                    cross(zi, xi, y);

                    float rotMatrix[4][4];

                    buildRotMatrix(xi, y, zi, (float *) rotMatrix);
                    glTranslatef(pos[0], pos[1], pos[2]);
                    glMultMatrixf((float *) rotMatrix);

                    for (int i = 0; i < 3; i++) {
                        y_prev[i] = y[i];
                    }

                    planetsPos[countCurves] += (float) interval / (float) (xmlParse->translateTimes[countCurves] * 1000 * time_multiplier);
                    t_prev_trans = curr_time_trans;
                } else {
                    glTranslatef(pos[0], pos[1], pos[2]);
                }
                countCurves++;
                break;

            case 8:
                planetsAngle[countRotate] += (360.0f / ((float) xmlParse->rotateTimes[countRotate] * 1000)) * (float) interval;

                t_prev_rot = curr_time_rot;
                glRotatef(planetsAngle[countRotate], 0, 1, 0);
                countRotate++;
                break;
        }
    }
}

void renderScene(void) {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(camX, camY, camZ,
              0.0, 0.0, 0.0,
              0.0f, 1.0f, 0.0f);

    glBegin(GL_LINES);
    // X axis in red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f( 100.0f, 0.0f, 0.0f);
    // Y Axis in Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f,-100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);
    // Z Axis in Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f,-100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();

    draw();

    // End of frame
    glutSwapBuffers();
}

void diferent_models(){
    for(int i = 0; i < xmlParse->models.size(); i++){
        bool exists = false;
        for(int j = 0; j < models.size() && !exists; j++){
            if(strcmp(models.at(j), xmlParse->models.at(i)) == 0) exists = true;
        }
        if(!exists){
            models.push_back(xmlParse->models.at(i));
        }
    }
}

void setToZeroAllPlanetsPos(){
    for(int i=0; i < xmlParse->translateTimes.size(); i++){
        planetsPos.push_back(0);
    }
}

void setToZeroAllPlanetsAngle(){
    for(int i=0; i < xmlParse->translateTimes.size(); i++){
        planetsAngle.push_back(0);
    }
}

int main(int argc, char** argv) {

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG@DI-UM");
    filename = argv[1];
    //readXML();
    xmlParse->readXML(filename);
    diferent_models();

    GLuint aux[models.size()+1];
    buffers = aux;
    setToZeroAllPlanetsPos();
    setToZeroAllPlanetsAngle();

    curr_time_trans = glutGet(GLUT_ELAPSED_TIME);
    interval = curr_time_trans - t_prev_trans;

    // init GLEW
#ifndef __APPLE__
    glewInit();
#endif

    // Required callback registry
    glutSpecialFunc(processSpecialKeys);
    glutKeyboardFunc(processKeys);
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);
    glutIdleFunc(renderScene);

    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnableClientState(GL_VERTEX_ARRAY);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // enter GLUT's main cycle
    glGenBuffers(models.size()+1, buffers);
    glutMainLoop();

    return 1;
}