#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <GL/glew.h>
#include <GL/glut.h>
#include <IL/il.h>
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

float camX = 0, camY = 0, camZ = 30;
double alpha = 0.0f, beta = 0.0f, radius = 30.0f;
float angle_step = 0.05f;

xml_parse* xmlParse = new xml_parse();
char* filename;

struct vertex {
    float x, y, z;
};

vector<vertex> allvertices;
vector<vertex> vertices;
vector< TiXmlElement*> groupStack;

vector<char*> filesRead;
vector<char*> texturesRead;
vector<const char*> models;
vector<const char*> textures;
vector<vector<vertex> > v_matrix;

vector<float> planetsPos;
vector<float> planetsAngle;

GLuint *buffers;    /* Buffer das coordenadas dos vertices */
GLuint *texBuffer;  /* Buffer das coordenadas das texturas */
GLuint *normBuffer;

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

GLuint* textIds;

/* variÃ¡veis globais texturas */


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

}

void processKeys(unsigned char c, int xx, int yy) {

    int stride = 1;

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
        case 'u':
            radius += 10;
            break;
        case 'j':
            radius -= 10;
            break;
    }
    spherical2Cartesian();
    glutPostRedisplay();
}


void processSpecialKeys(int key, int xx, int yy){
    switch(key){
        case GLUT_KEY_RIGHT:
            alpha -= angle_step;
            //look[0] = cos(beta)* sin(alpha);
            //look[1] = sin(beta);
            //look[2] = cos(beta) * cos(alpha);
            //cameraOrientarionNewAxis();
            break;
        case GLUT_KEY_LEFT:
            alpha += angle_step;
            //look[0] = cos(beta)* sin(alpha);
            //look[1] = sin(beta);
            //look[2] = cos(beta) * cos(alpha);
            //cameraOrientarionNewAxis();

            break;
        case GLUT_KEY_UP:
            beta += angle_step;
            if (beta > 1.5f)
                beta = 1.5f;
            //look[0] = cos(beta)* sin(alpha);
            //look[1] = sin(beta);
            //look[2] = cos(beta) * cos(alpha);
            //cameraOrientarionNewAxis();
            break;

        case GLUT_KEY_DOWN:
            beta -= angle_step;
            if (beta < -1.5f)
                beta = -1.5f;
            //look[0] = cos(beta)* sin(alpha);
            //look[1] = sin(beta);
            //look[2] = cos(beta) * cos(alpha);
            //cameraOrientarionNewAxis();
            break;
    }
    spherical2Cartesian();
    glutPostRedisplay();
}


void changeSize(int w, int h) {


    if (h == 0) h = 1;                                                                      // Prevent a divide by zero, when window is too short
    float ratio = w * 1.0 / h;                                                              // compute window's aspect ratio

    glMatrixMode(GL_PROJECTION);                                                        // Set the projection matrix as current
    glLoadIdentity();                                                                        // Load Identity Matrix
    glViewport(0, 0, w, h);                                                     // Set the viewport to be the entire window
    gluPerspective(xmlParse->fov, ratio, xmlParse->near, xmlParse->far);   // Set perspective
    glMatrixMode(GL_MODELVIEW);                                                         // Return to the model view matrix mode
}

/*------------ Divisão da informação de cada linha ---------*/
void tokenize(std::string const& str, const char delim,std::vector<std::string>& out){
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos){
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}


/*-----------------Carregamento das luzes------------------ */
void loadLights() {

    int n_light = 0;
    /*------------Ponctual Light---------*/

    //printf("%d,%d,%d", xmlParse->lightsPoint.size(), xmlParse->lightsDirectional.size(), xmlParse->lightsSpotlight.size());

    for (int i = 0; i < xmlParse->lightsPoint.size(); i++) {
        float pos[4];

        pos[0] = xmlParse->lightsPoint[i]->posX;
        pos[1] = xmlParse->lightsPoint[i]->posY;
        pos[2] = xmlParse->lightsPoint[i]->posZ;
        pos[3] = 1;
        glLightfv(GL_LIGHT0 + n_light, GL_POSITION, pos);
       // printf("\n\n%f | %f | %f\n\n", pos[0], pos[1], pos[2]);
        n_light++;
    }

    /*------------Directional Lights----*/

    for (int i = 0; i < xmlParse->lightsDirectional.size(); i++) {
        float dir[4];
        dir[0] = xmlParse->lightsDirectional[i]->dirX;
        dir[1] = xmlParse->lightsDirectional[i]->dirY;
        dir[2] = xmlParse->lightsDirectional[i]->dirZ;
        dir[3] = 0;
        glLightfv(GL_LIGHT0 + n_light, GL_SPOT_DIRECTION, dir);
        n_light++;
    }

    /*------------Spotlight Lights----*/

    for (int i = 0; i < xmlParse->lightsSpotlight.size(); i++) {
        float pos[4];
        pos[0] = xmlParse->lightsSpotlight[i]->posX;
        pos[1] = xmlParse->lightsSpotlight[i]->posY;
        pos[2] = xmlParse->lightsSpotlight[i]->posZ;
        pos[3] = 0;

        float dir[4];
        dir[0] = xmlParse->lightsSpotlight[i]->dirX;
        dir[1] = xmlParse->lightsSpotlight[i]->dirX;;
        dir[2] = xmlParse->lightsSpotlight[i]->dirZ;
        dir[3] = 1;

        float cutoff = xmlParse->lightsSpotlight[i]->cutoff;

        glLightfv(GL_LIGHT0 + n_light, GL_POSITION, pos);
        glLightfv(GL_LIGHT0 + n_light, GL_SPOT_DIRECTION, dir);
        glLightfv(GL_LIGHT0 + n_light, GL_SPOT_CUTOFF, &cutoff);
        n_light++;
    }
}

/*-----------------Carregamento das cores------------------ */
void loadColors(int textInd){

    //printf("%d\n", fileIndex);

    /* Ambiente */
    float ambient_color[3];
    ambient_color[0] = xmlParse->modelsTexAndColors[textInd]->ambient[0]/255.0f;
    ambient_color[1] = xmlParse->modelsTexAndColors[textInd]->ambient[1]/255.0f;
    ambient_color[2] = xmlParse->modelsTexAndColors[textInd]->ambient[2]/255.0f;
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient_color);

    /* Difusa */
    float diffuse_color[3];
    diffuse_color[0] = xmlParse->modelsTexAndColors[textInd]->diffuse[0]/255.0f;
    diffuse_color[1] = xmlParse->modelsTexAndColors[textInd]->diffuse[1]/255.0f;
    diffuse_color[2] = xmlParse->modelsTexAndColors[textInd]->diffuse[2]/255.0f;
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse_color);
    //printf("%f | %f | %f\n", diffuse_color[0], diffuse_color[1], diffuse_color[2]);

    /* Especular */
    float specular_color[3];
    specular_color[0] = xmlParse->modelsTexAndColors[textInd]->specular[0]/255.0f;
    specular_color[1] = xmlParse->modelsTexAndColors[textInd]->specular[1]/255.0f;
    specular_color[2] = xmlParse->modelsTexAndColors[textInd]->specular[2]/255.0f;
    glMaterialfv(GL_FRONT, GL_SPECULAR,specular_color);

    /* Emissiva */
    float emissive_color[3];
    emissive_color[0] = xmlParse->modelsTexAndColors[textInd]->emissive[0]/255.0f;
    emissive_color[1] = xmlParse->modelsTexAndColors[textInd]->emissive[1]/255.0f;
    emissive_color[2] = xmlParse->modelsTexAndColors[textInd]->emissive[2]/255.0f;
    glMaterialfv(GL_FRONT, GL_EMISSION,emissive_color);


    /* Shininess */
    //printf("Shininess%f\n", xmlParse->modelsTexAndColors[fileIndex]->shininess);
    glMaterialf(GL_FRONT, GL_SHININESS, xmlParse->modelsTexAndColors[textInd]->shininess);
}

/*--------------------Retorna id da Textura--------------------*/
int loadTex(std::string s){
    unsigned int t, tw, th, texID;
    unsigned char* texData;
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    ilGenImages(1, &t);
    ilBindImage(t);
    ilLoadImage((ILstring) s.c_str());
    tw = ilGetInteger(IL_IMAGE_WIDTH);
    th = ilGetInteger(IL_IMAGE_HEIGHT);
    ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
    texData = ilGetData();
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,tw,th,0,GL_RGBA,GL_UNSIGNED_BYTE,texData);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);
    return texID;
}

/*----------------------Parser do Ficheiros .3d --------------------*/
void parser(const char* filename, int modelInd) {

    string line;
    ifstream myfile(filename);

    const char delimiter = ';';

    vector<std::string> out;
    vector<float> tmp;
    vector<float> texels;
    vector<float> normals;

    /* Vai em cada linha colocar em out a info separado pelo delimitador  */
    if (myfile.is_open()){
        while (getline(myfile, line))
            tokenize(line, delimiter, out);

        myfile.close();
    }
    else cout << "Unable to open file " << filename << "\n";


    /* Guarda as 6 coordenadas de textura
     * ,9 coordenadas dos 3 vértices e
     * 9 coordenadas das normais de cada vértice associadas a um triângulo */

    for (size_t i = 0; i < out.size(); ) {
        for(int ti = 0 ; ti < 6  ; ti++)
            texels.push_back(std::stof(out[i + ti]));
        i += 6;
        for(int ni = 0; ni < 9; ni++)
            normals.push_back(std::stof(out[i + ni]));
        i+=9;
        for(int vi = 0; vi < 9 ; vi++)
            tmp.push_back(std::stof(out[i + vi]));
        i+=9;
    }

    /* Bind buffer com as coordenadas dos vértices */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[filesRead.size()-1]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glBufferData(GL_ARRAY_BUFFER, tmp.size()*sizeof(float), tmp.data(), GL_STATIC_DRAW);

    if(xmlParse->modelsTexAndColors[modelInd]->texFile != NULL) {
        /* Guarda o id do modelo atual */
        textIds[modelInd] = loadTex(xmlParse->modelsTexAndColors[modelInd]->texFile);

        /* Bind buffer com coordendas de texturas */
        glBindBuffer(GL_ARRAY_BUFFER, texBuffer[filesRead.size() - 1]);
        glTexCoordPointer(2, GL_FLOAT, 0, 0);
        glBufferData(GL_ARRAY_BUFFER, texels.size() * sizeof(float), texels.data(), GL_STATIC_DRAW);
    }

    /* Bind buffer com normais dos vértices */
    glBindBuffer(GL_ARRAY_BUFFER, normBuffer[filesRead.size()-1]);
    glBufferData(GL_ARRAY_BUFFER, normals.size()*sizeof(float), normals.data(), GL_STATIC_DRAW);

    /* Número de coordenadas de vertices e texturas de cada modelo */ //--------------------------------------Cuidado para modelos sem texturas
    sizes.push_back(tmp.size());
    sizes.push_back(texels.size());
    sizes.push_back(normals.size());

    vertices.clear();
}


void drawPrimitive(int fileIndex, int textIndex){

    loadColors(textIndex);

    /* Carregar textura */
    if (xmlParse->modelsTexAndColors[textIndex]->texFile != NULL)
        glBindTexture(GL_TEXTURE_2D, textIds[textIndex]);

    /* Carregar vertices */
    glBindBuffer(GL_ARRAY_BUFFER, buffers[fileIndex]);
    glVertexPointer(3, GL_FLOAT, 0, 0);

    /* Carregar normais dos vértices */
    glBindBuffer(GL_ARRAY_BUFFER, normBuffer[fileIndex]);
    glNormalPointer(GL_FLOAT, 0, 0);

    /* Carregar coordenadas de textura */
    glBindBuffer(GL_ARRAY_BUFFER, texBuffer[fileIndex]);
    glTexCoordPointer(2,GL_FLOAT,0,0);

    /* Desenhar os triângulos */
    glDrawArrays(GL_TRIANGLES, 0, (sizes.at(3*fileIndex))/3);


}


/*-----------------Multiplicação de Matrix por Vetor-----------------*/
void multMatrixVector(float *m, float *v, float *res) {

    for (int j = 0; j < 4; ++j) {
        res[j] = 0;
        for (int k = 0; k < 4; ++k)
            res[j] += v[k] * m[j * 4 + k];
    }
}

/*-----------------Obter pontos da curva num dado t-----------------*/
void getCatmullRomPoint(float t, float *p0, float *p1, float *p2, float *p3, float *pos, float *deriv) {

    /* catmull-rom matrix */
    float m[4][4] = {	{-0.5f,  1.5f, -1.5f,  0.5f},
                         { 1.0f, -2.5f,  2.0f, -0.5f},
                         {-0.5f,  0.0f,  0.5f,  0.0f},
                         { 0.0f,  1.0f,  0.0f,  0.0f}};

    /* Compute A = M * P */
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
}


/*-----------------Dado um t global obtêm-se os pontos da curva -----------------*/
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

    float* p0 = xmlParse->controlPoints.at(current_index + indices[0]);
    float* p1 = xmlParse->controlPoints.at(current_index + indices[1]);
    float* p2 = xmlParse->controlPoints.at(current_index + indices[2]);
    float* p3 = xmlParse->controlPoints.at(current_index + indices[3]);

    getCatmullRomPoint(t, p0, p1, p2, p3, pos, deriv);
}

/*-----------------Desenhar curva de Catmull-----------------*/
void renderCatmullRomCurve() {

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
    glBufferData(GL_ARRAY_BUFFER, x.size() * sizeof(float), x.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, buffers[models.size()]);
    glVertexPointer(3, GL_FLOAT, 0, 0);
    glDrawArrays(GL_LINE_LOOP, 0, x.size());
}

/* Simbologia das operações:

    1 -> translate
    2 -> rotate
    3 -> scale
    4 -> pushMatrix
    5 -> popMatrix
    6-> model
    7-> translate catmull
    8-> rotate catmull
*/

/*-----------------Desenha e Faz as transformações de primitivas-----------------*/
void draw() {
    int modelInd = 0;
    countCurves = 0;
    countRotate = 0;
    current_index = 0;
    bool existsFile;
    bool existsTexture;

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

                existsFile = false;
                existsTexture = false;
                int indFile;
                int indText;


                for (int j = 0; j < filesRead.size(); j++) {
                    if (strcmp(xmlParse->models[modelInd], filesRead[j]) == 0) {
                        indFile = j;
                        printf("indFile: %d | file: %s\n",j,filesRead[j]);

                        existsFile = true;
                    }
                }

                for (int j = 0; j < texturesRead.size(); j++) {
                    if (strcmp(xmlParse->modelsTexAndColors[modelInd]->texFile, texturesRead[j]) == 0) {
                        indText = j;
                        printf("indTx: %d | texFile: %s\n",j,texturesRead[j]);
                        existsTexture = true;
                    }
                }

                if (!existsFile && !existsTexture) {
                    filesRead.push_back((char *) xmlParse->models[modelInd]);
                    texturesRead.push_back((char *) xmlParse->modelsTexAndColors[modelInd]->texFile);
                    parser(xmlParse->models[modelInd], modelInd);
                    drawPrimitive(filesRead.size() - 1,texturesRead.size()-1);
                }
                else if(existsFile && !existsTexture){
                    texturesRead.push_back((char *) xmlParse->modelsTexAndColors[modelInd]->texFile);
                    textIds[modelInd] = loadTex(xmlParse->modelsTexAndColors[modelInd]->texFile);
                    drawPrimitive(indFile,modelInd);

                }else if(!existsFile && existsTexture ){
                    filesRead.push_back((char *) xmlParse->models[modelInd]);
                    parser(xmlParse->models[modelInd], modelInd);
                    drawPrimitive(filesRead.size() - 1,indText);

                }else{
                    drawPrimitive(indFile,indText);
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

    

    glClearColor(0.0f,0.0f,0.0f,0.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    //printf("%f | %f | %f\n", xmlParse->lightsPoint[0]->posX, xmlParse->lightsPoint[0]->posY, xmlParse->lightsPoint[0]->posZ);

    glLoadIdentity();

    gluLookAt(camX, camY, camZ,
              look[0], look[1], look[2],
              up[0], up[1], up[2]);


    loadLights();
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

/*    glTranslatef(2,2,-2);
    glutSolidSphere(0.5, 10, 10);
    glTranslatef(-2,-2,2);

    glTranslatef(-2,2,2);
    glutSolidSphere(0.5, 10, 10);
    glTranslatef(2,-2,-2);*/

    float dark[] = { 0.2, 0.2, 0.2, 1.0 };
    float white[] = { 0.8, 0.8, 0.8, 1.0 };
    float red[] = { 0.8, 0.2, 0.2, 1.0 };
    float yellow[] = {1,1,0,0};

    draw();



/*
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, red);
    glMaterialfv(GL_FRONT, GL_SPECULAR, white);
    glMaterialf(GL_FRONT, GL_SHININESS, 128);

    glutSolidSphere(1, 500, 500);
*/

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

void diferent_textures(){
    for(int i = 0; i < xmlParse->modelsTexAndColors.size(); i++){
        bool exists = false;
        if(xmlParse->modelsTexAndColors[i]->texFile != NULL) {
            for (int j = 0; j < textures.size() && !exists; j++) {
                if (strcmp(textures.at(j), xmlParse->modelsTexAndColors[i]->texFile) == 0) exists = true;
            }
            if (!exists) {
                textures.push_back(xmlParse->modelsTexAndColors[i]->texFile);
            }
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

void setUpCamera(){
    GLdouble aux_p[3];

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
    camX = camPos[0];
    camY = camPos[1];
    camZ = camPos[2];
}

int main(int argc, char** argv) {

    float black[] = {0.0f,0.0f,0.0f,0.0f};
    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG@DI-UM");
    filename = argv[1];

    //Lê o ficheiros XML
    xmlParse->readXML(filename);

    //Coloca a camera de acordo com XML
    setUpCamera();

    diferent_models();
    diferent_textures();

    GLuint aux[models.size()];
    GLuint auxTex[models.size()];
    GLuint auxNorm[models.size()];

    buffers = aux;
    texBuffer = auxTex;
    normBuffer = auxNorm;

    textIds = (GLuint*) malloc(sizeof(GLuint) * (models.size()-1));

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

    ilInit();


    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_LIGHTING);
    glEnable(GL_NORMALIZE);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);
    //enable lights

    GLfloat dark[4] = {0.2, 0.2, 0.2, 1.0};
    GLfloat white[4] = {1.0, 1.0, 1.0, 1.0};

    int n_lights = xmlParse->lightsDirectional.size() + xmlParse->lightsPoint.size() + xmlParse->lightsSpotlight.size();
    for(int i = 0; i < n_lights; i++){
        glEnable(GL_LIGHT0 + i);
        glLightfv(GL_LIGHT0 + i, GL_AMBIENT, dark);
        glLightfv(GL_LIGHT0 + i, GL_DIFFUSE, white);
        glLightfv(GL_LIGHT0 + i, GL_SPECULAR, white);
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT,black);
    }

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_NORMAL_ARRAY);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    // enter GLUT's main cycle
    glGenBuffers(models.size(), buffers);
    glGenBuffers(models.size(), texBuffer);
    glGenBuffers(models.size(), normBuffer);
    glutMainLoop();

    return 1;
}
