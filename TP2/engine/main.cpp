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
#include "tinyxml.h"
using namespace std;

GLdouble x1, t1, z1, x2, y2, z2, x3, y3, z3, fov, near, far;
float camX, camY, camZ;
float alpha = 0.0f, beta = 0.0f, radius = 5.0f;


char* filename;

struct vertex {
    float x, y, z;
};

vector<vertex> vertices;
vector<float> transformations;
vector<const char*> models;
vector< TiXmlElement*> groupStack;


/*

    1 -> translate
    2 -> rotate
    3 -> scale

    4 -> pushMatrix
    5 -> popMatrix

    6-> model
*/

void spherical2Cartesian() {

    camX = radius * cos(beta) * sin(alpha);
    camY = radius * sin(beta);
    camZ = radius * cos(beta) * cos(alpha);
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
                radius = 0.1f;
            break;
        case GLUT_KEY_PAGE_UP:
            radius += 0.1f;
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
    gluPerspective(fov, ratio, near, far);

    // return to the model view matrix mode
    glMatrixMode(GL_MODELVIEW);
}

void drawCone(float radius, float height, int cone_slices, int cone_stacks) {
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



            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);
            glVertex3f(px_top, (i + 1) * stack_height, pz_top);


            angle_bottom -= angle_step;


            px_bottom = bottom_radius * sin(angle_bottom);
            pz_bottom = bottom_radius * cos(angle_bottom);
            glVertex3f(px_bottom, i * stack_height, pz_bottom);


            glVertex3f(px_bottom, i * stack_height, pz_bottom);


            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);
            glVertex3f(px_top, (i + 1) * stack_height, pz_top);


            angle_top -= angle_step;

            px_top = top_radius * sin(angle_top);
            pz_top = top_radius * cos(angle_top);
            glVertex3f(px_top, (i + 1) * stack_height, pz_top);


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

        angle_bottom -= angle_step;

        px_bottom = radius * sin(angle_bottom);
        pz_bottom = radius * cos(angle_bottom);
        glVertex3f(px_bottom, 0.0, pz_bottom);

        glVertex3f(0.0, 0.0, 0.0);

    }
    glEnd();
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
            tokenize(line, delimiter, out);
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

/*

    1 -> translate
    2 -> rotate
    3 -> scale

    4 -> pushMatrix
    5 -> popMatrix

    6-> model
*/

void draw() {
    int modelInd = 0;

    for(int i=0 ; i<transformations.size() ; i++){

        switch((int)transformations[i]) {

            case 1:
                glTranslatef(transformations[i + 1], transformations[i + 2], transformations[i + 3]);
                i += 3;
                printf("Translação\n");
                break;

            case 2:
                glRotatef(transformations[i + 1], transformations[i + 2], transformations[i + 3],transformations[i + 4]);
                i += 4;
                printf("Rotação\n");
                break;

            case 3:
                glScalef(transformations[i + 1], transformations[i + 2], transformations[i + 3]);
                i += 3;
                printf("Escala\n");
                break;

            case 4:
                glPushMatrix();
                printf("Push\n");
                break;

            case 5:
                glPopMatrix();
                printf("Pop\n");
                break;

            case 6:
                parser(models[modelInd]);
                modelInd++;
                drawPrimitive();
                vertices.clear();
                printf("File\n");
                break;
        }
    }


}



void readCamera(TiXmlElement* l_pRootElement){
    TiXmlElement* l_camera = l_pRootElement->FirstChildElement("camera");

    if (loadOkay) {

        TiXmlElement* l_pRootElement = doc.FirstChildElement("world");

        if (l_pRootElement != NULL) {

            TiXmlElement* l_camera = l_pRootElement->FirstChildElement("camera");

            if (NULL != l_camera)
            {
                TiXmlElement* l_position = l_camera->FirstChildElement("position");

                x1 = std::stod(l_position->Attribute("x"));
                z1 = std::stod(l_position->Attribute("z"));
                t1 = std::stod(l_position->Attribute("y"));

                TiXmlElement* l_lookAt = l_camera->FirstChildElement("lookAt");

                x2 = std::stod(l_lookAt->Attribute("x"));
                y2 = std::stod(l_lookAt->Attribute("y"));
                z2 = std::stod(l_lookAt->Attribute("z"));

                TiXmlElement* l_up = l_camera->FirstChildElement("up");

                x3 = std::stod(l_up->Attribute("x"));
                y3 = std::stod(l_up->Attribute("y"));
                z3 = std::stod(l_up->Attribute("z"));


void readTransformations(TiXmlElement* l_group){
    transformations.push_back(4.0f); // push_matrix

    TiXmlElement* l_element = l_group->FirstChildElement();

    while (l_element != NULL) {

        if (strcmp(l_element->Value(), "transform") == 0){
            TiXmlElement* l_transform = l_element;

            TiXmlElement* l_transformation = l_transform->FirstChildElement();

            while(l_transformation != NULL){
                if (strcmp(l_transformation->Value(), "translate") == 0) {
                    float translate_x = std::stod(l_transformation->Attribute("x"));
                    float translate_y = std::stod(l_transformation->Attribute("y"));
                    float translate_z = std::stod(l_transformation->Attribute("z"));
                    transformations.push_back(1.0f);
                    transformations.push_back(translate_x);
                    transformations.push_back(translate_y);
                    transformations.push_back(translate_z);
                }
                else if (strcmp(l_transformation->Value(), "rotate") == 0) {
                    float angle = std::stod(l_transformation->Attribute("angle"));
                    float rotate_x = std::stod(l_transformation->Attribute("x"));
                    float rotate_y = std::stod(l_transformation->Attribute("y"));
                    float rotate_z = std::stod(l_transformation->Attribute("z"));
                    transformations.push_back(2.0f);
                    transformations.push_back(angle);
                    transformations.push_back(rotate_x);
                    transformations.push_back(rotate_y);
                    transformations.push_back(rotate_z);
                }
                else if (strcmp(l_transformation->Value(), "scale") == 0) {
                    float scale_x = std::stod(l_transformation->Attribute("x"));
                    float scale_y = std::stod(l_transformation->Attribute("y"));
                    float scale_z = std::stod(l_transformation->Attribute("z"));
                    transformations.push_back(3.0f);
                    transformations.push_back(scale_x);
                    transformations.push_back(scale_y);
                    transformations.push_back(scale_z);
                }
                l_transformation = l_transformation->NextSiblingElement();
            }
        }
        else if(strcmp(l_element->Value(), "group") == 0){
            TiXmlElement* l_nested_group = l_element;
            cout << l_nested_group->Value();
            readTransformations(l_nested_group);
        }else{
            TiXmlElement* l_models = l_element;
            TiXmlElement* l_model = l_models->FirstChildElement();

            while (l_model != NULL){
                const char* model = strdup(l_model->Attribute("file"));
                models.push_back(model);
                transformations.push_back(6.0f);
                l_model = l_model->NextSiblingElement();
            }


        }
        l_element = l_element->NextSiblingElement(); //avançar o elemento XML
    }
    transformations.push_back(5.0f); // pop_matrix
}


void readXML() {

    TiXmlDocument doc(filename);
    bool loadOkay = doc.LoadFile();

    if (loadOkay) {

        TiXmlElement* l_pRootElement = doc.FirstChildElement("world");

        if (l_pRootElement != NULL) {

            readCamera(l_pRootElement);

            TiXmlElement* l_group = l_pRootElement->FirstChildElement("group");
            readTransformations(l_group);
        }
    }
    else {
        printf("Ficheiro nao carregado");
    }


}


void renderScene(void) {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();




     gluLookAt(camX, camY, camZ,
              x2, y2, z2,
              x3, y3, z3);

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
    readXML();


    // Required callback registry
    glutSpecialFunc(processSpecialKeys);
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
