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


    for (int j = 0; j < divisions; j++) {

        startz = startzFixo - (j * size);

        for (int i = 0; i < divisions; i++) {

            startx = startxFixo + (i * size);

            MyFile << startx << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz - size << std::endl;

            MyFile << startx << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz - size << ";";
            MyFile << startx << ";" << 0 << ";" << startz - size << std::endl;
        }
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


    // ------------------ BASES CUBO

    float size = static_cast<float>(length) / divisions; //length de cada divisao
    float startx = -(length / static_cast<float>(2)); //x inicial de forma ao cubo centrar na origem
    float startxFixo = startx;
    float startz = length / static_cast<float>(2);
    float startzFixo = startz;
    float bottomY = -(length / static_cast<float>(2));
    float topY = bottomY + length;


    for (int j = 0; j < divisions; j++) {

        startz = startzFixo - (j * size);

        for (int i = 0; i < divisions; i++) {

            startx = startxFixo + (i * size);

            // ------------------ BASES 1

            MyFile << startx << ";" << bottomY << ";" << startz << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz - size << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz << std::endl;

            MyFile << startx << ";" << bottomY << ";" << startz << ";";
            MyFile << startx << ";" << bottomY << ";" << startz - size << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz - size << std::endl;


            // ------------------ BASES 2

            MyFile << startx << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz - size << std::endl;

            MyFile << startx << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz - size << ";";
            MyFile << startx << ";" << topY << ";" << startz - size << std::endl;
        }
    }

    // ---------------- front left back AND right-------------------

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

            //---------------------------- FACE 1

            MyFile << startx << ";" << starty << ";" << startz << ";";
            MyFile << startx + size << ";" << starty << ";" << startz << ";";
            MyFile << startx << ";" << starty + size << ";" << startz << std::endl;

            MyFile << startx << ";" << starty + size << ";" << startz << ";";
            MyFile << startx + size << ";" << starty << ";" << startz << ";";
            MyFile << startx + size << ";" << starty + size << ";" << startz << std::endl;


            //---------------------------- FACE 2

            MyFile << startx << ";" << starty << ";" << endz << ";";
            MyFile << startx << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty << ";" << endz << std::endl;

            MyFile << startx << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty << ";" << endz << std::endl;
        }
    }


    // ------ front right / back left-------


    startx = -(length / static_cast<float>(2));
    float endx = startx + length;

    startz = -(length / static_cast<float>(2));
    startzFixo = startz;

    starty = -(length / static_cast<float>(2));
    startYfixo = starty;


    for (int j = 0; j < divisions; j++) {
        startz = startzFixo + (j * size);


        for (int i = 0; i < divisions; i++) {
            starty = startYfixo + (i * size);

            // ------- LADO 1

            MyFile << startx << ";" << starty << ";" << startz + size << ";";
            MyFile << startx << ";" << starty + size << ";" << startz << ";";
            MyFile << startx << ";" << starty << ";" << startz << std::endl;

            MyFile << startx << ";" << starty + size << ";" << startz << ";";
            MyFile << startx << ";" << starty << ";" << startz + size << ";";
            MyFile << startx << ";" << starty + size << ";" << startz + size << std::endl;


            // ------- LADO 2

            MyFile << endx << ";" << starty + size << ";" << startz + size << ";";
            MyFile << endx << ";" << starty << ";" << startz + size << ";";
            MyFile << endx << ";" << starty + size << ";" << startz << std::endl;

            MyFile << endx << ";" << starty << ";" << startz << ";";
            MyFile << endx << ";" << starty + size << ";" << startz << ";";
            MyFile << endx << ";" << starty << ";" << startz + size << std::endl;

            

        }
    }

    MyFile.close();
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

    std::ofstream MyFile(fileName);

    float radius_step = radius / (float)cone_stacks;

    float bottom_radius = radius;
    float top_radius = radius - radius_step;

    float stack_height = height / (float)cone_stacks;


    float angle_step = (2 * M_PI) / (float)cone_slices;

    for (int j = 0; j < cone_slices; j++) {

        for (int i = 0; i < cone_stacks; i++) {

            px_bottom = bottom_radius * sin(angle_bottom);
            pz_bottom = bottom_radius * cos(angle_bottom);

            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << ";";


            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);

            MyFile << px_top << ";" << (i + 1) * stack_height << ";" << pz_top << ";";

            angle_bottom -= angle_step;


            px_bottom = bottom_radius * sin(angle_bottom);
            pz_bottom = bottom_radius * cos(angle_bottom);

            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << std::endl;


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

    for (int i = 0; i <= cone_slices; i++) {

        px_bottom = radius * sin(angle_bottom);
        pz_bottom = radius * cos(angle_bottom);
        MyFile << px_bottom << ";" << 0.0 << ";" << pz_bottom << ";";

        angle_bottom -= angle_step;

        px_bottom = radius * sin(angle_bottom);
        pz_bottom = radius * cos(angle_bottom);
        MyFile << px_bottom << ";" << 0.0 << ";" << pz_bottom << ";";

        MyFile << 0.0 << ";" << 0.0 << ";" << 0.0 << std::endl;

    }
}

void writeSphere(float radius, int slices, int stacks, char* filename) {

    std::ofstream Myfile(filename);

    float d_alpha = (float)2 * M_PI / slices;
    float d_beta = (float)M_PI / stacks;

    float i_alpha = 0.0f;
    float i_beta = M_PI / 2;

    float x, y, z;

    for (int currStack = 0; currStack < stacks; currStack++) {
        //int currStack = 3;
        for (int currSlice = 0; currSlice < slices; currSlice++) {
            vertex v1 = assignCoords(radius, currStack, currSlice, i_beta, d_beta, d_alpha);
            vertex v2 = assignCoords(radius, currStack + 1, currSlice, i_beta, d_beta, d_alpha);
            vertex v3 = assignCoords(radius, currStack + 1, currSlice + 1, i_beta, d_beta, d_alpha);

            vertex v4 = assignCoords(radius, currStack, currSlice + 1, i_beta, d_beta, d_alpha);

            //1� triangulo

            Myfile << v1.x << ";" << v1.y << ";" << v1.z << ";";
            Myfile << v2.x << ";" << v2.y << ";" << v2.z << ";";
            Myfile << v3.x << ";" << v3.y << ";" << v3.z << std::endl;

            //2� triangulo

            Myfile << v1.x << ";" << v1.y << ";" << v1.z << ";";
            Myfile << v3.x << ";" << v3.y << ";" << v3.z << ";";
            Myfile << v4.x << ";" << v4.y << ";" << v4.z << std::endl;
        }
    }
    Myfile.close();
}

void writeBezier(char* patchFilename, int nr, char* filename) {

    parsePatch(patchFilename);
    writeSurface(nr,filename);
}


void choosePrimitive() {
    if (strcmp(arguments[1], "sphere") == 0 && argumentsLen >= 4) {
        writeSphere(atoi(arguments[2]), atoi(arguments[3]), atoi(arguments[4]), arguments[5]);
    }

    if (strcmp(arguments[1], "box") == 0 && argumentsLen >= 5) {
        writeCube(atoi(arguments[2]), atoi(arguments[3]), arguments[4]);
    }

    if (strcmp(arguments[1], "cone") == 0 && argumentsLen >= 5) {
        writeCone(atoi(arguments[2]), atoi(arguments[3]), atoi(arguments[4]), atoi(arguments[5]), arguments[6]);
    }

    if (strcmp(arguments[1], "plane") == 0 ) {
        if(argumentsLen >= 5)
            writePlane(atoi(arguments[2]), atoi(arguments[3]), arguments[4]);
        else {
            writePlane(atoi(arguments[2]), atoi(arguments[2]), arguments[3]);
        }
    }

    if(strcmp(arguments[1],"ring") == 0 && argumentsLen >= 4){
        drawRing(atof(arguments[2]),atof(arguments[3]),atoi(arguments[4]),arguments[5]);
    }

    if (strcmp(arguments[1], "bezier") == 0 && argumentsLen >= 4) {
        writeBezier(arguments[2], atoi(arguments[3]),arguments[4]);
    }
}


int main(int argc, char** argv) {

    arguments = argv;
    argumentsLen = argc;

    //choosePrimitive();
    choosePrimitive();

    return 1;
}