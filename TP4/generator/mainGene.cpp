#include <stdlib.h>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>

using namespace std;


char** arguments;
int argumentsLen;

struct vertex {
    float x, y, z;
};


int n_patches;
int n_control_points;

vector<std::string> patches_indices;
vector<std::string> control_points;
vector<float*> final_points;

float preCalculatedMatrixx[4][4];
float preCalculatedMatrixy[4][4];
float preCalculatedMatrixz[4][4];

vertex assignCoords(float radius, int currStack, int currSlice, float i_beta, float d_beta, float d_alpha) {
    vertex v = {
            radius * cos(i_beta - (currStack * d_beta)) * sin(currSlice * d_alpha),
            radius * sin(i_beta - (currStack * d_beta)),
            radius * cos(i_beta - (currStack * d_beta)) * cos(currSlice * d_alpha),
    };
    return v;
}

void normalize(float * a) {

    float l = sqrt(a[0]*a[0] + a[1] * a[1] + a[2] * a[2]);
    a[0] = a[0]/l;
    a[1] = a[1]/l;
    a[2] = a[2]/l;
}


void tokenize(std::string const& str, const char delim, std::vector<std::string>& out){
    size_t start;
    size_t end = 0;

    while ((start = str.find_first_not_of(delim, end)) != std::string::npos){
        end = str.find(delim, start);
        out.push_back(str.substr(start, end - start));
    }
}

void parsePatch(char* filename) {
    string line;
    ifstream myfile(filename);
    int i = 0;

    const char delimiter = ',';

    if (myfile.is_open()) {
        while (getline(myfile, line)) {
            if (i == 0) {
                n_patches = stoi(line);
            }
            else if (i > 0 && i < n_patches + 1) {
                tokenize(line, delimiter, patches_indices);
            }
            else if (i == n_patches + 1) {
                n_control_points = stoi(line);
            }
            else if (i > n_patches + 1 && i < n_patches + n_control_points + 2) {
                tokenize(line, delimiter, control_points);
            }
            i++;
        }
        myfile.close();
    }
    else printf("Unable to open file");
}

void printMatrix(float m[4][4]){
    for(int i = 0; i < 4; i++){
        for(int j = 0; j < 4; j++){
            printf("%f ", m[i][j]);
        }
        printf("\n");
    }
}

void getControlPointsMatrix(int currentPatch, int coord, float matrix[4][4]) {
    int aux = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            int index = stoi(patches_indices.at(16 * currentPatch + aux));
            matrix[i][j] = stof(control_points.at(3*index + coord));
            aux++;
        }
    }
    //printMatrix(matrix);
}

void multLineColumn(float l[4], float c[4][1], float *res){
    for(int i = 0; i < 4; i++){
        *res += l[i] * c[i][0];
    }
}

void multVectorMatrix(float v[4], float m[4][4], float res[4]) {
    for(int i = 0; i < 4; i++){
        res[i] = 0;

        for(int j = 0; j < 4; j++)
            res[i] += v[j] * m[j][i];
    }
}

void transposeMatrix(float m[4][4], float res[4][4]){
    for(int i = 0 ; i < 4 ; i++){
        for(int j = 0; j  < 4; j++){
            res[j][i] = m[i][j];
        }
    }
}

void multMatrixMatrix(float m[4][4], float v[4][4], float res[4][4]) {

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            res[i][j] = 0;

            for (int k = 0; k < 4; k++) {
                res[i][j] += m[i][k] * v[k][j];
            }
        }
    }
}

void preCalculate(int currentPatch) {

    float aux[4][4];
    float controlPoints[4][4];

    float m[4][4] = {{-1.0f,  3.0f, -3.0f,  1.0f},
                         { 3.0f, -6.0f,  3.0f, 0.0f},
                         {-3.0f,  3.0f,  0.0f,  0.0f},
                         { 1.0f,  0.0f,  0.0f,  0.0f}};

    float mTransposed[4][4];
    transposeMatrix(m, mTransposed);

    getControlPointsMatrix(currentPatch,0, controlPoints);
    multMatrixMatrix(m, controlPoints, aux);
    multMatrixMatrix(aux, mTransposed, preCalculatedMatrixx);

    getControlPointsMatrix(currentPatch,1, controlPoints);
    multMatrixMatrix(m, controlPoints, aux);
    multMatrixMatrix(aux, mTransposed, preCalculatedMatrixy);

    getControlPointsMatrix(currentPatch, 2, controlPoints);
    multMatrixMatrix(m, controlPoints, aux);
    multMatrixMatrix(aux, mTransposed, preCalculatedMatrixz);

}

float* getSurfacePoint(float u, float v){

    float* values = (float*) malloc(sizeof(float)*4);

    for(int i = 0; i < 4; i++) {
        values[i] = 0.0f;
    }

    float controlPoints[4][4];

    float uVector[4] = {powf(u,3), powf(u,2), u, 1};
    float vVector[4][1] = {{powf(v,3)}, 
                           {powf(v,2)}, 
                           {v}, 
                           {1}};

    float aux[4];

    //calculate x
    multVectorMatrix(uVector, preCalculatedMatrixx, aux);
    multLineColumn(aux, vVector, &values[0]);

    //calculate y
    multVectorMatrix(uVector, preCalculatedMatrixy, aux);
    multLineColumn(aux, vVector, &values[1]);
    
    //calculate z
    multVectorMatrix(uVector, preCalculatedMatrixz, aux);
    multLineColumn(aux, vVector, &values[2]);

    return values;
}

void writeSurface(int tesselation, char* fileName){
    
    float u_step = (float) 1/ (float) tesselation;
    float v_step = (float) 1/ (float) tesselation;

    std::ofstream MyFile(fileName);

    for(int currentPatch = 0; currentPatch < n_patches; currentPatch++){

        preCalculate(currentPatch);

        for(float u = 0; u <= 1.05; u += u_step){
            for(float v = 0; v <= 1.05; v += v_step){

                float* ponto1 = getSurfacePoint(u, v);                    //0,0    esquerda cima
                float* ponto2 = getSurfacePoint(u+u_step, v);             //1,0    direita cima
                float* ponto3 = getSurfacePoint(u, v+v_step);             //0,1    esquerda baixo
                float* ponto4 = getSurfacePoint(u + u_step, v + v_step);  //1,1    direita baixo


                //escrever 0,0 esquerda cima
                MyFile << ponto1[0] << ";" << ponto1[1] << ";" << ponto1[2] << ";";
                //escrever 0,1 esquerda baixo
                MyFile << ponto3[0] << ";" << ponto3[1] << ";" << ponto3[2] << ";";
                //escrever 1,1 direita baixo
                MyFile << ponto4[0] << ";" << ponto4[1] << ";" << ponto4[2] << endl;
            

                //escrever 0,0 esquerda cima
                MyFile << ponto1[0] << ";" << ponto1[1] << ";" << ponto1[2] << ";";
                //escrever 1,1 direita baixo
                MyFile << ponto4[0] << ";" << ponto4[1] << ";" << ponto4[2] << ";";
                //escrever 1,0 direita cima
                MyFile << ponto2[0] << ";" << ponto2[1] << ";" << ponto2[2] << endl;
                
            }
        }
    }
    MyFile.close();
}

void writePlane(int length, int divisions, char* fileName) {

    std::ofstream MyFile(fileName);

    float size = static_cast<float>(length) / (float) divisions;
    float startx = -(length / static_cast<float>(2));
    float startxFixo = startx;
    float startz = length / static_cast<float>(2);
    float startzFixo = startz;
    float text_ind_x = 0.0f, text_ind_y = 0.0f;

    float text_step = 1.0f / divisions;


    for (int j = 0; j < divisions; j++, text_ind_y += text_step) {

        startz = startzFixo - (j * size);

        for (int i = 0; i < divisions; i++ , text_ind_x += text_step) {

            startx = startxFixo + (i * size);

            /* coordenadas de textura */
            MyFile << text_ind_x << ";" << text_ind_y << ";";
            MyFile << text_ind_x + text_step << ";" << text_ind_y << ";";
            MyFile << text_ind_x + text_step << ";" << text_ind_y + text_step << std::endl;

            /* coordenadas das normais*/
            MyFile << 0 << ";" << 1 << ";" << 0 << ";";
            MyFile << 0 << ";" << 1 << ";" << 0 << ";";
            MyFile << 0 << ";" << 1 << ";" << 0 << std::endl;

            /* coordenadas de Vertice */
            MyFile << startx << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz - size << std::endl;


            /* coordenadas de textura */
            MyFile << text_ind_x << ";" << text_ind_y << ";";
            MyFile << text_ind_x + text_step << ";" << text_ind_y + text_step << ";";
            MyFile << text_ind_x  << ";" << text_ind_y + text_step << std::endl;

            /* coordenadas das normais*/
            MyFile << 0 << ";" << 1 << ";" << 0 << ";";
            MyFile << 0 << ";" << 1 << ";" << 0 << ";";
            MyFile << 0 << ";" << 1 << ";" << 0 << std::endl;

            /* coordenadas de Vertice */
            MyFile << startx << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz - size << ";";
            MyFile << startx << ";" << 0 << ";" << startz - size << std::endl;

        }

        text_ind_x = 0.0f;
    }

    MyFile.close();
}

void drawRing(float translate, float side, int it, char* filename){

    std::ofstream MyFile(filename);

    float alpha_1 = atan(side/(2*translate));
    float alpha_2 = atan(side/(2*(translate+side)));
    float alpha_3 = atan(-(side/(2*(translate+side))));
    float alpha_4 = atan(-side/(2*translate));

    float r1 = (float) sqrt(pow(translate,2) + pow((side/2),2));
    float r2 = (float) sqrt(pow(translate+side,2) + pow(side/2,2));

    float d = 0;

    for(int i = 0; i < it; i++){

        MyFile << r1 * sin(alpha_4+d) << ";" << 0 << ";" << r1*cos(alpha_4+d) << ";";
        MyFile << r2 * sin(alpha_2+d) << ";" << 0 << ";" << r2*cos(alpha_2+d) << ";";
        MyFile << r1 * sin(alpha_1+d) << ";" << 0 << ";" << r1*cos(alpha_1+d) << ";" << "\n";

        MyFile << r1 * sin(alpha_4+d) << ";" << 0 << ";" << r1*cos(alpha_4+d) << ";";
        MyFile << r2 * sin(alpha_3+d) << ";" << 0 << ";" << r2*cos(alpha_3+d) << ";";
        MyFile << r2 * sin(alpha_2+d) << ";" << 0 << ";" << r2*cos(alpha_2+d) << ";" << "\n";

        MyFile << r1 * sin(alpha_1+d) << ";" << 0 << ";" << r1*cos(alpha_1+d) << ";";
        MyFile << r2 * sin(alpha_2+d) << ";" << 0 << ";" << r2*cos(alpha_2+d) << ";";
        MyFile << r1 * sin(alpha_4+d) << ";" << 0 << ";" << r1*cos(alpha_4+d) << ";" << "\n";

        MyFile << r2 * sin(alpha_2+d) << ";" << 0 << ";" << r2*cos(alpha_2+d) << ";";
        MyFile << r2 * sin(alpha_3+d) << ";" << 0 << ";" << r2*cos(alpha_3+d) << ";";
        MyFile << r1 * sin(alpha_4+d) << ";" << 0 << ";" << r1*cos(alpha_4+d) << ";" << "\n";


        d += 2*M_PI/it;
    }
    MyFile.close();
}

void writeCube(int length, int divisions, char* fileName) {

    std::ofstream MyFile(fileName);

    float text_ind_x = 0.0f, text_ind_y = 0.0f;


    // ------------------ BASES CUBO

    float size = static_cast<float>(length) / divisions; //length de cada divisao
    float startx = -(length / static_cast<float>(2)); //x inicial de forma ao cubo centrar na origem
    float startxFixo = startx;
    float startz = length / static_cast<float>(2);
    float startzFixo = startz;
    float bottomY = -(length / static_cast<float>(2));
    float topY = bottomY + length;
    float step_text = 1.0f / divisions;


    for (int j = 0; j < divisions; j++) {

        startz = startzFixo - (j * size);

        for (int i = 0; i < divisions; i++) {

            startx = startxFixo + (i * size);


            //-----------BASE 1 bottom ---------------------------------------------

            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y + step_text << ";";
            MyFile << text_ind_x + step_text << ";" <<  text_ind_y  << std::endl;

            /* normais do vértice*/

            MyFile << "0;-1;0;0;-1;0;0;-1;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << bottomY << ";" << startz << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz - size << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz << std::endl;


            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x << ";" << text_ind_y + step_text << ";";
            MyFile << text_ind_x + step_text << ";" <<  text_ind_y + step_text << std::endl;

            /* normais do vértice*/

            MyFile << "0;-1;0;0;-1;0;0;-1;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << bottomY << ";" << startz << ";";
            MyFile << startx << ";" << bottomY << ";" << startz - size << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz - size << std::endl;



            // --------------BASES 2 top ---------------------------------------------

            /* coordenadas das texturas */

            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y  << ";";
            MyFile << text_ind_x + step_text << ";" <<  text_ind_y  + step_text << std::endl;

            /* normais do vértice*/

            MyFile << "0;1;0;0;1;0;0;1;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz - size << std::endl;


            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y + step_text << ";";
            MyFile << text_ind_x  << ";" <<  text_ind_y + step_text << std::endl;

            /* normais do vértice*/

            MyFile << "0;1;0;0;1;0;0;1;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz - size << ";";
            MyFile << startx << ";" << topY << ";" << startz - size << std::endl;
            text_ind_x += step_text;
        }
        text_ind_y += step_text;
        text_ind_x = 0;
    }
    text_ind_y = 0;

    // ---------------- front left AND back right-------------------

    startx = -(length / static_cast<float>(2)); //x inicial de forma ao cubo centrar na origem
    startxFixo = startx;

    startz = (length / static_cast<float>(2));
    float endz = -(length / static_cast<float>(2));

    float starty = -(length / static_cast<float>(2));
    float startYfixo = starty;


    for (int j = 0; j < divisions; j++) {

        starty = startYfixo + (j * size);

        for (int i = 0; i < divisions; i++) {

            startx = startxFixo + (i * size);

            //------------ FACE 1---------------------------------
            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y  << ";";
            MyFile << text_ind_x  << ";" <<  text_ind_y + step_text << std::endl;

            /* normais do vértice*/

            MyFile << "0;0;1;0;0;1;0;0;1" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << starty << ";" << startz << ";";
            MyFile << startx + size << ";" << starty << ";" << startz << ";";
            MyFile << startx << ";" << starty + size << ";" << startz << std::endl;


            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y + step_text<< ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y  << ";";
            MyFile << text_ind_x + step_text << ";" <<  text_ind_y + step_text << std::endl;

            /* normais do vértice*/
            MyFile << "0;0;1;0;0;1;0;0;1" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << starty + size << ";" << startz << ";";
            MyFile << startx + size << ";" << starty << ";" << startz << ";";
            MyFile << startx + size << ";" << starty + size << ";" << startz << std::endl;


            //---------------FACE 2-----------------------------------------
            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x  << ";" << text_ind_y + step_text  << ";";
            MyFile << text_ind_x + step_text << ";" <<  text_ind_y  << std::endl;

            /* normais do vértice*/

            MyFile << "0;0;-1;0;0;-1;0;0;-1" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << starty << ";" << endz << ";";
            MyFile << startx << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty << ";" << endz << std::endl;


            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y + step_text<< ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y + step_text  << ";";
            MyFile << text_ind_x + step_text << ";" <<  text_ind_y << std::endl;

            /* normais do vértice*/

            MyFile << "0;0;-1;0;0;-1;0;0;-1" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty << ";" << endz << std::endl;
            text_ind_x += step_text;
        }
        text_ind_y += step_text;
        text_ind_x = 0;
    }


    text_ind_y = 0;


    // ------ front right / back left-------


    startx = -(length / static_cast<float>(2));
    float endx = startx + length;

    startz = (length / static_cast<float>(2));
    startzFixo = startz;

    starty = -(length / static_cast<float>(2));
    startYfixo = starty;


    for (int j = 0; j < divisions; j++) {
        starty = startYfixo + (j * size);


        for (int i = 0; i < divisions; i++) {
            startz = startzFixo - (i * size);


            // ----------------------LADO 1 Front-Right---------------------------------

            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y << ";";
            MyFile << text_ind_x  + step_text << ";" <<  text_ind_y + step_text << std::endl;

            /* normais do vértice*/

            MyFile << "1;0;0;1;0;0;1;0;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << endx << ";" << starty << ";" << startz<< ";";
            MyFile << endx << ";" << starty << ";" << startz-size << ";";
            MyFile << endx << ";" << starty+size << ";" << startz -size << std::endl;


            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x  + step_text << ";" << text_ind_y  + step_text<< ";";
            MyFile << text_ind_x << ";" <<  text_ind_y + step_text << std::endl;

            /* normais do vértice*/

            MyFile << "1;0;0;1;0;0;1;0;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << endx << ";" << starty << ";" << startz << ";";
            MyFile << endx << ";" << starty +size << ";" << startz - size << ";";
            MyFile << endx << ";" << starty + size << ";" << startz << std::endl;


            // --------------------------LADO 2 Back-Left--------------------------

            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x + step_text << ";" << text_ind_y + step_text << ";";
            MyFile << text_ind_x + step_text  << ";" <<  text_ind_y << std::endl;

            /* normais do vértice*/

            MyFile << "-1;0;0;-1;0;0;-1;0;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << starty << ";" << startz << ";";
            MyFile << startx << ";" << starty + size << ";" << startz-size << ";";
            MyFile << startx << ";" << starty << ";" << startz - size << std::endl;


            /* coordenadas das texturas */
            MyFile << text_ind_x << ";" <<  text_ind_y << ";";
            MyFile << text_ind_x << ";" << text_ind_y + step_text << ";";
            MyFile << text_ind_x + step_text<< ";" <<  text_ind_y + step_text << std::endl;

            /* normais do vértice*/

            MyFile << "-1;0;0;-1;0;0;-1;0;0" << std::endl;

            /* coordenadas dos vertices */
            MyFile << startx << ";" << starty << ";" << startz << ";";
            MyFile << startx << ";" << starty+ size << ";" << startz  << ";";
            MyFile << startx << ";" << starty + size << ";" << startz -size << std::endl;
            text_ind_x += step_text;
        }
        text_ind_y += step_text;
        text_ind_x = 0;
    }

    MyFile.close();
}

float* getTexPointsCone(int slice, int stack, int cone_stacks, int cone_slices, float* points){

    float x1 = points[stack];
    float x2 = points[stack+1];
    //float cycle_var = stack / (2*cone_stacks);
    float tex_steplower = (1 - (2*x1))/ cone_slices;
    float tex_stephigher = (1 - (2*x2))/ cone_slices;

    float* ret = (float*) malloc(sizeof(float)*4);

    ret[0] = x1 + tex_steplower*slice;
    ret[1] = x1 + tex_steplower*slice + tex_steplower;
    ret[2] = x2 + tex_stephigher*slice;
    ret[3] = x2 + tex_stephigher*slice + tex_stephigher;
    return ret;
}


void getConeVertices(int slices, float height, vector<vertex> vertices){

    for(int i=0 ; i<slices ; i++){




    }





    vertex topo = {0,height,0};
    vertices.push_back(topo);





}

void writeCone(float radius, float height, int cone_slices, int cone_stacks, char* fileName) {
    float px_top = 0,
        px_bottom = 0,
        py_top = 0,
        py_bottom = 0,
        pz_top = 0,
        pz_bottom = 0,
        angle_bottom = 0,
        angle_top = 0,
        starting_top_angle,
        starting_bottom_angle;

    float i = 0.0f;
    int index = 0;
    float* points = (float*) malloc(sizeof(float) * cone_stacks);
    float cycle_var;
    float y_step = (1.0f/(float)cone_stacks);

    vector<vertex> verticesCone;
    getConeVertices(cone_slices,height,verticesCone);

    while(index <= cone_stacks){
        cycle_var = i / (float) (2*cone_stacks);
        points[index] = cycle_var;
        i++;
        index++;
    }

    std::ofstream MyFile(fileName);

    float radius_step = radius / (float)cone_stacks;

    float bottom_radius = radius;
    float top_radius = radius - radius_step;

    float stack_height = height / (float)cone_stacks;
    float angle_step = (2 * M_PI) / (float)cone_slices;

    for (int j = 0; j < cone_slices; j++) {

        for (int i = 0; i < cone_stacks; i++) {

            float* getPoints = getTexPointsCone(i, j, cone_stacks, cone_slices, points);

            px_bottom = bottom_radius * sin(angle_bottom);
            pz_bottom = bottom_radius * cos(angle_bottom);

            //-----------------1º triângulo-----------------------

            /* coordenadas de textura */

            MyFile << getPoints[1] << ";" << i*y_step << ";";
            MyFile << getPoints[3] << ";" << i*y_step + y_step << ";";
            MyFile << getPoints[0] << ";" << i*y_step << std::endl;

            /* coordenadas dos vertices */
            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << ";";

            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);

            MyFile << px_top << ";" << (i + 1) * stack_height << ";" << pz_top << ";";

            angle_bottom -= angle_step;

            px_bottom = bottom_radius * sin(angle_bottom);
            pz_bottom = bottom_radius * cos(angle_bottom);

            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << std::endl;


            //---------------2º triângulo-------------------

            /* coordenadas das texturas */
            MyFile << getPoints[0] << ";" << i*y_step << ";";
            MyFile << getPoints[3] << ";" << i*y_step + y_step << ";";
            MyFile << getPoints[2] << ";" << i*y_step + y_step << std::endl;

            /* coordenadas dos vertices */
            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << ";";

            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);

            MyFile << px_top << ";" << (i + 1) * stack_height << ";" << pz_top << ";";

            angle_top -= angle_step;

            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);

            MyFile << px_top << ";" << (i + 1) * stack_height << ";" << pz_top << std::endl;

            bottom_radius = top_radius;
            top_radius -= radius_step;
            angle_top += angle_step;
            angle_bottom += angle_step;


        }
        angle_top -= angle_step;
        angle_bottom -= angle_step;
        bottom_radius = radius;
        top_radius = radius - radius_step;
    }

    angle_bottom = 0;

    for (int i = 0; i <= cone_slices; i++){

        px_bottom = radius * sin(angle_bottom);
        pz_bottom = radius * cos(angle_bottom);

        /* coordenadas das texturas */
        MyFile << "0;0;1;0;1;1" << std::endl;

        /* coordenadas dos vertices */
        MyFile << px_bottom << ";" << 0.0 << ";" << pz_bottom << ";";

        angle_bottom -= angle_step;

        px_bottom = radius * sin(angle_bottom);
        pz_bottom = radius * cos(angle_bottom);
        MyFile << px_bottom << ";" << 0.0 << ";" << pz_bottom << ";";

        MyFile << 0.0 << ";" << 0.0 << ";" << 0.0 << std::endl;

    }
}

void calculateVec(float p0[3], float p1[3], float res[3]) {
    res[0] = p0[0] - p1[0];
    res[1] = p0[1] - p1[1];
    res[2] = p0[2] - p1[2];
}

void writeSphere(float radius, int slices, int stacks, char* filename) {

    float center[3] = {0,0,0};

    std::ofstream Myfile(filename);

    float d_alpha = (float)2 * M_PI / slices;
    float d_beta = (float)M_PI / stacks;

    float i_alpha = 0.0f;
    float i_beta = M_PI / 2;

    float x, y, z;

    float tex_ind_x = 1.0f, tex_ind_y = 1.0f;
    float tex_step_x = 1.0f / slices, tex_step_y = 1.0f / stacks;

    for (int currStack = 0; currStack < stacks; currStack++, tex_ind_y -= tex_step_y) {
        for (int currSlice = 0; currSlice < slices; currSlice++, tex_ind_x += tex_step_x) {
            vertex v1 = assignCoords(radius, currStack, currSlice, i_beta, d_beta, d_alpha);
            vertex v2 = assignCoords(radius, currStack + 1, currSlice, i_beta, d_beta, d_alpha);
            vertex v3 = assignCoords(radius, currStack + 1, currSlice + 1, i_beta, d_beta, d_alpha);
            vertex v4 = assignCoords(radius, currStack, currSlice + 1, i_beta, d_beta, d_alpha);

            float ver1[3] = {v1.x, v1.y, v1.z};
            float ver2[3] = {v2.x, v2.y, v2.z};
            float ver3[3] = {v3.x, v3.y, v3.z};
            float ver4[3] = {v4.x, v4.y, v4.z};

            float vector1[3] = {sin(currStack*d_beta)*cos(currSlice*d_alpha), cos(currStack*d_beta), sin(currStack*d_beta)*sin(currSlice*d_alpha)};
            float vector2[3] = {sin((currStack+1)*d_beta)*cos(currSlice*d_alpha), cos((currStack+1)*d_beta), sin((currStack+1)*d_beta)*sin(currSlice*d_alpha)};
            float vector3[3] = {sin((currStack+1)*d_beta)*cos((currSlice+1)*d_alpha), cos((currStack+1)*d_beta), sin((currStack+1)*d_beta)*sin((currSlice+1)*d_alpha)};
            float vector4[3] = {sin(currStack*d_beta)*cos((currSlice+1)*d_alpha), cos(currStack*d_beta), sin(currStack*d_beta)*sin((currSlice+1)*d_alpha)};

            normalize(vector1);
            normalize(vector2);
            normalize(vector3);
            normalize(vector4);


            //--------------1º triangulo----------------

            /* coordenadas das texturas */
            Myfile << tex_ind_x << ";" << tex_ind_y << ";";
            Myfile << tex_ind_x << ";" << tex_ind_y - tex_step_y << ";";
            Myfile << tex_ind_x + tex_step_x << ";" << tex_ind_y - tex_step_y << std::endl;

            /* coordenadas das normais */

            Myfile << vector1[0] << ";" << vector1[1] << ";" << vector1[2] << ";";
            Myfile << vector2[0] << ";" << vector2[1] << ";" << vector2[2] << ";";
            Myfile << vector3[0] << ";" << vector3[1] << ";" << vector3[2] << std::endl;


            /* coordenadas dos vertices */
            Myfile << v1.x << ";" << v1.y << ";" << v1.z << ";";
            Myfile << v2.x << ";" << v2.y << ";" << v2.z << ";";
            Myfile << v3.x << ";" << v3.y << ";" << v3.z << std::endl;


            //---------------2º triangulo---------------------

            /* coordenadas das texturas */
            Myfile << tex_ind_x << ";" << tex_ind_y << ";";
            Myfile << tex_ind_x + tex_step_x << ";" << tex_ind_y - tex_step_y << ";";
            Myfile << tex_ind_x + tex_step_x << ";" << tex_ind_y << std::endl;

            /* coordenadas das normais */
            Myfile << vector1[0] << ";" << vector1[1] << ";" << vector1[2] << ";";
            Myfile << vector3[0] << ";" << vector3[1] << ";" << vector3[2] << ";";
            Myfile << vector4[0] << ";" << vector4[1] << ";" << vector4[2] << std::endl;

            /* coordenadas dos vertices */
            Myfile << v1.x << ";" << v1.y << ";" << v1.z << ";";
            Myfile << v3.x << ";" << v3.y << ";" << v3.z << ";";
            Myfile << v4.x << ";" << v4.y << ";" << v4.z << std::endl;
        }
        tex_ind_x = 0;
    }
    Myfile.close();
}

void writeBezier(char* patchFilename, int nr, char* filename) {
    parsePatch(patchFilename);
    writeSurface(nr,filename);
}

/* Dependendo do argumento dado vai desenhar a dada primitiva */
void choosePrimitive() {
    if (strcmp(arguments[1], "sphere") == 0 && argumentsLen >= 4)
        writeSphere(atoi(arguments[2]), atoi(arguments[3]), atoi(arguments[4]), arguments[5]);

    if (strcmp(arguments[1], "box") == 0 && argumentsLen >= 5)
        writeCube(atoi(arguments[2]), atoi(arguments[3]), arguments[4]);

    if (strcmp(arguments[1], "cone") == 0 && argumentsLen >= 5)
        writeCone(atoi(arguments[2]), atoi(arguments[3]), atoi(arguments[4]), atoi(arguments[5]), arguments[6]);

    if (strcmp(arguments[1], "plane") == 0 )
        if(argumentsLen >= 5)
            writePlane(atoi(arguments[2]), atoi(arguments[3]), arguments[4]);
        else
            writePlane(atoi(arguments[2]), atoi(arguments[2]), arguments[3]);


    if(strcmp(arguments[1],"ring") == 0 && argumentsLen >= 4)
        drawRing(atof(arguments[2]),atof(arguments[3]),atoi(arguments[4]),arguments[5]);

    if (strcmp(arguments[1], "bezier") == 0 && argumentsLen >= 4)
        writeBezier(arguments[2], atoi(arguments[3]),arguments[4]);
}


int main(int argc, char** argv) {

    arguments = argv;
    argumentsLen = argc;

    choosePrimitive();

    return 1;
}