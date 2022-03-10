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
    gluPerspective(45.0f, ratio, 1.0f, 1000.0f);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}



void drawPlane(int length, int divisions, char* fileName) {

    std::ofstream MyFile(fileName);

    glBegin(GL_TRIANGLES);

    float size = static_cast<float>(length) / divisions;
    float startx = -(length / static_cast<float>(2));
    float startxFixo = startx;
    float startz = length / static_cast<float>(2);
    float startzFixo = startz;


    for (int j = 0; j < divisions; j++) {

        startz = startzFixo - (j * size);

        for (int i = 0; i < divisions; i++) {

            startx = startxFixo + (i * size);

            glVertex3f(startx, 0, startz);
            glVertex3f(startx + size, 0, startz);
            glVertex3f(startx + size, 0, startz - size);

            MyFile << startx << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz - size << std::endl;

            glVertex3f(startx, 0, startz);
            glVertex3f(startx + size, 0, startz - size);
            glVertex3f(startx, 0, startz - size);

            MyFile << startx << ";" << 0 << ";" << startz << ";";
            MyFile << startx + size << ";" << 0 << ";" << startz - size << ";";
            MyFile << startx << ";" << 0 << ";" << startz - size << std::endl;
        }
    }
    glEnd();

    MyFile.close();
}

void drawCube(int length, int divisions, char* fileName) {
    glBegin(GL_TRIANGLES);
    glColor3f(0.0f, 1.0f, 0.0f);

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

            glVertex3f(startx, bottomY, startz);
            glVertex3f(startx + size, bottomY, startz - size);
            glVertex3f(startx + size, bottomY, startz);

            MyFile << startx << ";" << bottomY << ";" << startz << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz - size << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz << std::endl;

            glVertex3f(startx, bottomY, startz);
            glVertex3f(startx, bottomY, startz - size);
            glVertex3f(startx + size, bottomY, startz - size);

            MyFile << startx << ";" << bottomY << ";" << startz << ";";
            MyFile << startx << ";" << bottomY << ";" << startz - size << ";";
            MyFile << startx + size << ";" << bottomY << ";" << startz - size << std::endl;


            // ------------------ BASES 2

            glVertex3f(startx, topY, startz);
            glVertex3f(startx + size, topY, startz);
            glVertex3f(startx + size, topY, startz - size);

            MyFile << startx << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz - size << std::endl;

            glVertex3f(startx, topY, startz);
            glVertex3f(startx + size, topY, startz - size);
            glVertex3f(startx, topY, startz - size);

            MyFile << startx << ";" << topY << ";" << startz << ";";
            MyFile << startx + size << ";" << topY << ";" << startz - size << ";";
            MyFile << startx << ";" << topY << ";" << startz - size << std::endl;
        }
    }

    // ---------------- front left back AND right-------------------

    size = static_cast<float>(length) / divisions; //length de cada divisao

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

            glVertex3f(startx, starty, startz);
            glVertex3f(startx + size, starty, startz);
            glVertex3f(startx, starty + size, startz);

            MyFile << startx << ";" << starty << ";" << startz << ";";
            MyFile << startx + size << ";" << starty << ";" << startz << ";";
            MyFile << startx << ";" << starty + size << ";" << startz << std::endl;

            glVertex3f(startx, starty + size, startz);
            glVertex3f(startx + size, starty, startz);
            glVertex3f(startx + size, starty + size, startz);

            MyFile << startx << ";" << starty + size << ";" << startz << ";";
            MyFile << startx + size << ";" << starty << ";" << startz << ";";
            MyFile << startx + size << ";" << starty + size << ";" << startz << std::endl;


            //---------------------------- FACE 2

            glVertex3f(startx, starty, endz);
            glVertex3f(startx, starty + size, endz);
            glVertex3f(startx + size, starty, endz);

            MyFile << startx << ";" << starty << ";" << endz << ";";
            MyFile << startx << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty << ";" << endz << std::endl;

            glVertex3f(startx, starty + size, endz);
            glVertex3f(startx + size, starty + size, endz);
            glVertex3f(startx + size, starty, endz);

            MyFile << startx << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty + size << ";" << endz << ";";
            MyFile << startx + size << ";" << starty << ";" << endz << std::endl;
        }
    }


    // ------ front right / back left-------


    size = static_cast<float>(length) / divisions; //length de cada divisao

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


            glVertex3f(startx, starty, startz + size);
            glVertex3f(startx, starty + size, startz);
            glVertex3f(startx, starty, startz);


            MyFile << startx << ";" << starty << ";" << startz + size << ";";
            MyFile << startx << ";" << starty + size << ";" << startz << ";";
            MyFile << startx << ";" << starty << ";" << startz << std::endl;


            glVertex3f(startx, starty + size, startz);
            glVertex3f(startx, starty, startz + size);
            glVertex3f(startx, starty + size, startz + size);


            MyFile << startx << ";" << starty + size << ";" << startz << ";";
            MyFile << startx << ";" << starty << ";" << startz + size << ";";
            MyFile << startx << ";" << starty + size << ";" << startz + size << std::endl;


            // ------- LADO 2

            glVertex3f(endx, starty, startz);
            glVertex3f(endx, starty + size, startz);
            glVertex3f(endx, starty, startz + size);


            MyFile << endx << ";" << starty << ";" << startz << ";";
            MyFile << endx << ";" << starty + size << ";" << startz << ";";
            MyFile << endx << ";" << starty << ";" << startz + size << std::endl;


            glVertex3f(endx, starty + size, startz + size);
            glVertex3f(endx, starty, startz + size);
            glVertex3f(endx, starty + size, startz);


            MyFile << endx << ";" << starty + size << ";" << startz + size << ";";
            MyFile << endx << ";" << starty << ";" << startz + size << ";";
            MyFile << endx << ";" << starty + size << ";" << startz << std::endl;

        }
    }

    MyFile.close();
    glEnd();
}

void drawCone(float radius, float height, int cone_slices, int cone_stacks, char* fileName) {
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
    glColor3f(1, 1, 1);

    glBegin(GL_TRIANGLES);
    for (int j = 0; j < cone_slices; j++) {

        for (int i = 0; i < cone_stacks; i++) {

            px_bottom = bottom_radius * sin(angle_bottom);
            pz_bottom = bottom_radius * cos(angle_bottom);
            glVertex3f(px_bottom, i * stack_height, pz_bottom);

            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << ";";


            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);
            glVertex3f(px_top, (i + 1) * stack_height, pz_top);

            MyFile << px_top << ";" << (i + 1) * stack_height << ";" << pz_top << ";";

            angle_bottom -= angle_step;


            px_bottom = bottom_radius * sin(angle_bottom);
            pz_bottom = bottom_radius * cos(angle_bottom);
            glVertex3f(px_bottom, i * stack_height, pz_bottom);

            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << std::endl;

            glVertex3f(px_bottom, i * stack_height, pz_bottom);

            MyFile << px_bottom << ";" << i * stack_height << ";" << pz_bottom << ";";

            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);
            glVertex3f(px_top, (i + 1) * stack_height, pz_top);

            MyFile << px_top << ";" << (i + 1) * stack_height << ";" << pz_top << ";";

            angle_top -= angle_step;

            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);
            glVertex3f(px_top, (i + 1) * stack_height, pz_top);

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
        glVertex3f(px_bottom, 0.0, pz_bottom);
        MyFile << px_bottom << ";" << 0.0 << ";" << pz_bottom << ";";

        angle_bottom -= angle_step;

        px_bottom = radius * sin(angle_bottom);
        pz_bottom = radius * cos(angle_bottom);
        glVertex3f(px_bottom, 0.0, pz_bottom);
        MyFile << px_bottom << ";" << 0.0 << ";" << pz_bottom << ";";

        glVertex3f(0.0, 0.0, 0.0);
        MyFile << 0.0 << ";" << 0.0 << ";" << 0.0 << std::endl;

    }
    glEnd();
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

std::vector<vertex> sphereCoords(float radius, int slices, int stacks, char* filename) {

    std::ofstream Myfile(filename);

    vector<vertex> vertices;

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

            //1º triangulo

            Myfile << v1.x << ";" << v1.y << ";" << v1.z << ";";
            Myfile << v2.x << ";" << v2.y << ";" << v2.z << ";";
            Myfile << v3.x << ";" << v3.y << ";" << v3.z << std::endl;

            //2º triangulo

            Myfile << v1.x << ";" << v1.y << ";" << v1.z << ";";
            Myfile << v3.x << ";" << v3.y << ";" << v3.z << ";";
            Myfile << v4.x << ";" << v4.y << ";" << v4.z << std::endl;

            vertices.push_back(v1);
            vertices.push_back(v2);
            vertices.push_back(v3);
            vertices.push_back(v1);
            vertices.push_back(v3);
            vertices.push_back(v4);
        }
    }
    Myfile.close();

    return vertices;
}

void drawSphere(vector<vertex> vertices) {

    glBegin(GL_TRIANGLES);
    for (std::size_t i = 0; i < vertices.size(); i += 3) {
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
        glVertex3f(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
        glVertex3f(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
    }
    glEnd();
}


void choosePrimitive() {
    if (strcmp(arguments[1], "sphere") == 0 && argumentsLen >= 4) {
        vector<vertex> vertices = sphereCoords(atoi(arguments[2]), atoi(arguments[3]), atoi(arguments[4]), arguments[5]);
        drawSphere(vertices);
    }

    if (strcmp(arguments[1], "box") == 0 && argumentsLen >= 5) {
        drawCube(atoi(arguments[2]), atoi(arguments[3]), arguments[4]);
    }

    if (strcmp(arguments[1], "cone") == 0 && argumentsLen >= 5) {
        drawCone(atoi(arguments[2]), atoi(arguments[3]), atoi(arguments[4]), atoi(arguments[5]), arguments[6]);
    }

    if (strcmp(arguments[1], "plane") == 0 && argumentsLen >= 5) {
        drawPlane(atoi(arguments[2]), atoi(arguments[3]), arguments[4]);
    }
}

void renderScene(void) {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(5.0, 5.0, 5.0,
        0.0, 0.0, 0.0,
        0.0f, 1.0f, 0.0f);

    glBegin(GL_LINES);
    // X axis in red
    glColor3f(1.0f, 0.0f, 0.0f);
    glVertex3f(-100.0f, 0.0f, 0.0f);
    glVertex3f(100.0f, 0.0f, 0.0f);
    // Y Axis in Green
    glColor3f(0.0f, 1.0f, 0.0f);
    glVertex3f(0.0f, -100.0f, 0.0f);
    glVertex3f(0.0f, 100.0f, 0.0f);
    // Z Axis in Blue
    glColor3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, -100.0f);
    glVertex3f(0.0f, 0.0f, 100.0f);
    glEnd();

    // set the camera
    glLoadIdentity();
    gluLookAt(5.0, 5.0, 5.0,
        0.0, 0.0, 0.0,
        0.0f, 1.0f, 0.0f);

    // put the geometric transformations here



    // put drawing instructions here

    choosePrimitive();

    //drawSphere(vertices);

    //drawCone(2,5,3,4);

    // End of frame
    glutSwapBuffers();
}

int main(int argc, char** argv) {

    arguments = argv;
    argumentsLen = argc;

    // init GLUT and the window
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(800, 800);
    glutCreateWindow("CG@DI-UM");

    // Required callback registry
    glutDisplayFunc(renderScene);
    glutReshapeFunc(changeSize);


    // put here the registration of the keyboard callbacks

    //glutKeyboardFunc(function_name);


    //  OpenGL settings
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // enter GLUT's main cycle
    glutMainLoop();

    return 1;
}