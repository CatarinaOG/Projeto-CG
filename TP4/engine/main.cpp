#ifdef __APPLE__
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

float camPos[3] = {0,0,0};
float look[3] = {1,0,0};
float up[3] = {0,1,0};
float lookVec[3];
float yVec[3];

float* r = (float*) malloc(sizeof(float)*3);


double alpha /*= 0.0f*/, beta /*= 0.0f*/, radius /*= 5.0f*/;

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




void cross(float *a, float *b, float *res) {
    res[0] = a[1]*b[2] - a[2]*b[1];
    res[1] = a[2]*b[0] - a[0]*b[2];
    res[2] = a[0]*b[1] - a[1]*b[0];
}

void calculateVec(float p0[3], float p1[3], float res[3]) {
    res[0] = p0[0] - p1[0];
    res[1] = p0[1] - p1[1];
    res[2] = p0[2] - p1[2];
}

void spherical2Cartesian() {
    camPos[0] = radius * cos(beta) * sin(alpha);
    camPos[1] = radius * sin(beta);
    camPos[2] = radius * cos(beta) * cos(alpha);
}

void buildRotMatrix(float *x, float *y, float *z, float *m) {
    m[0] = x[0]; m[1] = x[1]; m[2] = x[2]; m[3] = 0;
    m[4] = y[0]; m[5] = y[1]; m[6] = y[2]; m[7] = 0;
    m[8] = z[0]; m[9] = z[1]; m[10] = z[2]; m[11] = 0;
    m[12] = 0; m[13] = 0; m[14] = 0; m[15] = 1;
}


void normalize(float * a) {

    float l = sqrt(a[0]*a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] = a[0]/l;
    a[1] = a[1]/l;
    a[2] = a[2]/l;
}


void cameraOrientarionNewAxis(){
    calculateVec(look, camPos, lookVec);
    normalize(lookVec);
    cross(lookVec, up, r);
    normalize(r);
    cross(r, lookVec, yVec);
    normalize(yVec);
    printf("look: %f %f %f\n",look[0],look[1],look[2]);
    printf("camPos: %f %f %f\n",camPos[0],camPos[1],camPos[2]);
    printf("lookVec: %f %f %f\n",lookVec[0],lookVec[1],lookVec[2]);
    printf("r: %f %f %f\n",r[0],r[1],r[2]);
    printf("yVec: %f %f %f\n",yVec[0],yVec[1],yVec[2]);

}

void processKeys(unsigned char c, int xx, int yy) {

    int stride = 1;
// put code to process regular keys in here
    switch(c){
        case 'l':
            time_multiplier *= 2;
            break;
        case 'r':
            time_multiplier = time_multiplier/2;
            break;
        case 'o':
            if(!showCurve) showCurve = true;
            else{
                showCurve = false;
            }
            break;
        case 'd':

            camPos[0] += r[0]*stride;
            camPos[1] += r[1]*stride;
            camPos[2] += r[2]*stride;

            look[0] += r[0]*stride;
            look[1] += r[1]*stride;
            look[2] += r[2]*stride;
            break;

        case 'a':
            camPos[0] -= r[0]*stride;
            camPos[1] -= r[1]*stride;
            camPos[2] -= r[2]*stride;

            look[0] -= r[0]*stride;
            look[1] -= r[1]*stride;
            look[2] -= r[2]*stride;
            break;

        case 'w':
            camPos[0] += lookVec[0]*stride;
            camPos[1] += lookVec[1]*stride;
            camPos[2] += lookVec[2]*stride;

            look[0] += lookVec[0]*stride;
            look[1] += lookVec[1]*stride;
            look[2] += lookVec[2]*stride;
            break;

        case 's':
            camPos[0] -= lookVec[0]*stride;
            camPos[1] -= lookVec[1]*stride;
            camPos[2] -= lookVec[2]*stride;

            look[0] -= lookVec[0]*stride;
            look[1] -= lookVec[1]*stride;
            look[2] -= lookVec[2]*stride;
            break;

        case 'e':
            camPos[0] += yVec[0]*stride;
            camPos[1] += yVec[1]*stride;
            camPos[2] += yVec[2]*stride;

            look[0] += yVec[0]*stride;
            look[1] += yVec[1]*stride;
            look[2] += yVec[2]*stride;
            break;

        case 'q':
            camPos[0] -= yVec[0]*stride;
            camPos[1] -= yVec[1]*stride;
            camPos[2] -= yVec[2]*stride;

            look[0] -= yVec[0]*stride;
            look[1] -= yVec[1]*stride;
            look[2] -= yVec[2]*stride;
            break;
    }

    glutPostRedisplay();
}


void processSpecialKeys(int key, int xx, int yy){
    switch(key){
        case GLUT_KEY_RIGHT:
            alpha -= 0.001f;
            look[0] = cos(beta)* sin(alpha);
            look[1] = sin(beta);
            look[2] = cos(beta) * cos(alpha);
            cameraOrientarionNewAxis();
            break;
        case GLUT_KEY_LEFT:
            alpha += 0.001f;
            look[0] = cos(beta)* sin(alpha);
            look[1] = sin(beta);
            look[2] = cos(beta) * cos(alpha);
            cameraOrientarionNewAxis();

            break;
        case GLUT_KEY_UP:
            beta += 0.001f;
            if (beta > 1.5f)
                beta = 1.5f;
            look[0] = cos(beta)* sin(alpha);
            look[1] = sin(beta);
            look[2] = cos(beta) * cos(alpha);
            cameraOrientarionNewAxis();
            break;

        case GLUT_KEY_DOWN:
            beta -= 0.001f;
            if (beta < -1.5f)
                beta = -1.5f;
            look[0] = cos(beta)* sin(alpha);
            look[1] = sin(beta);
            look[2] = cos(beta) * cos(alpha);
            cameraOrientarionNewAxis();
            break;
    }
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
    vector<float> tmp;
    vector<float> texels;
    for (std::size_t i = 0; i < out.size(); i += 15) {


        for(int ti = 0 ; ti < 6  ; ti++)
            texels.push_back(std::stof(out[ti]));
        for(int vi = 0; vi < 9 ; vi++)
            tmp.push_back(std::stof(out[vi]));

    }





    /* Bind buffer com o pontos */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2*(filesRead.size()-1)]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, tmp.size()*sizeof(float), tmp.data(), GL_STATIC_DRAW);

    /* bind buffer com coordendas de texturas */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[2*(filesRead.size()-1)+1]);
    glBufferData(GL_ARRAY_BUFFER, texels.size()*sizeof(float), texels.data(), GL_STATIC_DRAW);



    sizes.push_back(tmp.size());

    vertices.clear();
}


void drawPrimitive(int fileIndex){
    glColor3f(1.0f, 1.0f, 1.0f);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[2*fileIndex]);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER,buffers[2*fileIndex+1]);
    glTexCoordPointer(2,GL_FLOAT,0,0);
    glDrawArrays(GL_TRIANGLES, 0, sizes.at(2*fileIndex));

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

    gluLookAt(camPos[0], camPos[1], camPos[2],
              look[0], look[1], look[2],
              up[0], up[1], up[2]);

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

    GLdouble aux_p[3];

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG@DI-UM");
    filename = argv[1];
    //readXML();
    xmlParse->readXML(filename);

    look[0] = xmlParse->x2;
    look[1] = xmlParse->y2;
    look[2] = xmlParse->z2;

    camPos[0] = xmlParse->x1;
    camPos[1] = xmlParse->t1;
    camPos[2] = xmlParse->z1;

    cameraOrientarionNewAxis();

    aux_p[0] = camPos[0] + lookVec[0];
    aux_p[1] = camPos[1] + lookVec[1];
    aux_p[2] = camPos[2] + lookVec[2];

    beta = asin(aux_p[1] - camPos[1]);

    float tmp = (aux_p[2] - camPos[2]) / cos(beta);
    
    if (tmp < -1 || tmp > 1)
        if(tmp < -1)
            alpha = acos(-1);
        else
            alpha = acos(1);
    else
        alpha = acos((aux_p[2] - camPos[2]) / cos(beta));



    //beta = beta * 180.0 / M_PI;
    //alpha = alpha * 180.0 / M_PI;


    diferent_models();

    GLuint aux[2*(models.size()+1)];
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