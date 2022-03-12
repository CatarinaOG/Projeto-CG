#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <fstream>
#include <GL/glut.h>
#endif

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <vector>

using namespace std;

char** arguments;
int argumentsLen;


void writePlane(int length, int divisions, char* fileName) {

    std::ofstream MyFile(fileName);

    float size = static_cast<float>(length) / divisions;
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

            MyFile << endx << ";" << starty << ";" << startz << ";";
            MyFile << endx << ";" << starty + size << ";" << startz << ";";
            MyFile << endx << ";" << starty << ";" << startz + size << std::endl;

            MyFile << endx << ";" << starty + size << ";" << startz + size << ";";
            MyFile << endx << ";" << starty << ";" << startz + size << ";";
            MyFile << endx << ";" << starty + size << ";" << startz << std::endl;

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

struct vertex {
    float x, y, z;
};

vertex assignCoords(float radius, int currStack, int currSlice, float i_beta, float d_beta, float d_alpha) {
    vertex v = {
            radius * cos(i_beta - (currStack * d_beta)) * sin(currSlice * d_alpha),
            radius * sin(i_beta - (currStack * d_beta)),
            radius * cos(i_beta - (currStack * d_beta)) * cos(currSlice * d_alpha),
    };
    return v;
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

}


int main(int argc, char** argv) {

    arguments = argv;
    argumentsLen = argc;

    choosePrimitive();


    return 1;
}