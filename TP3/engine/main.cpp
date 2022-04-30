#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string>

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

// Translate Catmull
static float t=0;
float pos[4];
float deriv[4];

void spherical2Cartesian() {

    camX = radius * cos(beta) * sin(alpha);
    camY = radius * sin(beta);
    camZ = radius * cos(beta) * cos(alpha);
}


void processKeys(unsigned char c, int xx, int yy) {

// put code to process regular keys in here


}


void processSpecialKeys(int key, int xx, int yy){
    switch(key){
        case GLUT_KEY_RIGHT:
            alpha -= 0.1;
            break;
        case GLUT_KEY_LEFT:
            alpha += 0.1;
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
            radius -= 0.1f;
            if(radius < 0.1f)
                radius = 1.0f;
            break;
        case GLUT_KEY_PAGE_UP:
            radius += 1.0f;
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

    if (myfile.is_open()){
        while (getline(myfile, line)){
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
}


void drawPrimitive(){
    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (std::size_t i = 0; i < vertices.size(); i += 3) {
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
        glVertex3f(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
        glVertex3f(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
    }
    glEnd();
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

    int n_points = xmlParse->controlPoints.size();
    // Points that make up the loop for catmull-rom interpolation

    float t = gt * n_points; // this is the real global t
    int index = floor(t);  // which segment
    t = t - index; // where within  the segment

    // indices store the points
    int indices[n_points];

    indices[0] = (index +n_points-1)%n_points;
    indices[1] = (indices[0]+1)%n_points;
    indices[2] = (indices[1]+1)%n_points;
    indices[3] = (indices[2]+1)%n_points;

    float p0[3] = {xmlParse->controlPoints.at(indices[0]), xmlParse->controlPoints.at(indices[0]+1), xmlParse->controlPoints.at(indices[0]+2)};
    float p1[3] = {xmlParse->controlPoints.at(indices[1]), xmlParse->controlPoints.at(indices[1]+1), xmlParse->controlPoints.at(indices[1]+2)};
    float p2[3] = {xmlParse->controlPoints.at(indices[2]), xmlParse->controlPoints.at(indices[2]+1), xmlParse->controlPoints.at(indices[2]+2)};
    float p3[3] = {xmlParse->controlPoints.at(indices[3]), xmlParse->controlPoints.at(indices[3]+1), xmlParse->controlPoints.at(indices[3]+2)};

    getCatmullRomPoint(t, p0, p1, p2, p3, pos, deriv);
}

void renderCatmullRomCurve() {

    // draw curve using line segments with GL_LINE_LOOP
    float pos[4];
    float deriv[4];

    glBegin(GL_LINE_LOOP);

    for (int i = 0; i < 50; ++i) {
        getGlobalCatmullRomPoint(i / 50.0, pos, deriv);
        glVertex3f(pos[0], pos[1], pos[2]);
    }
    glEnd();
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

    for(int i=0 ; i<xmlParse->transformations.size() ; i++){

        switch((int)xmlParse->transformations[i]) {

            case 1:
                glTranslatef(xmlParse->transformations[i + 1], xmlParse->transformations[i + 2], xmlParse->transformations[i + 3]);
                i += 3;
                //printf("Translação\n");
                break;

            case 2:
                glRotatef(xmlParse->transformations[i + 1], xmlParse->transformations[i + 2], xmlParse->transformations[i + 3],xmlParse->transformations[i + 4]);
                i += 4;
                //("Rotação\n");
                break;

            case 3:
                glScalef(xmlParse->transformations[i + 1], xmlParse->transformations[i + 2], xmlParse->transformations[i + 3]);
                i += 3;
                //("Escala\n");
                break;

            case 4:
                glPushMatrix();
                //printf("Push\n");
                break;

            case 5:
                glPopMatrix();
                //printf("Pop\n");
                break;

            case 6:
                parser(xmlParse->models[modelInd]);
                modelInd++;
                drawPrimitive();
                vertices.clear();
                break;

            case 7:
                renderCatmullRomCurve();
                getGlobalCatmullRomPoint(t,pos,deriv);
                glTranslatef(pos[0],pos[1],pos[2]);
        }
    }
}

void renderScene(void) {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();

    gluLookAt(camX, camY, camZ,
              xmlParse->x2, xmlParse->y2, xmlParse->z2,
              xmlParse->x3, xmlParse->y3, xmlParse->z3);

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

    // Required callback registry
    glutSpecialFunc(processSpecialKeys);
    glutKeyboardFunc(processKeys);
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);

    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // enter GLUT's main cycle
    glutMainLoop();

    return 1;
}