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
using namespace std;

GLdouble x1, t1, z1, x2, y2, z2, x3, y3, z3, fov, near, far;
char* filename;

struct vertex {
    float x, y, z;
};

vector<vertex> vertices;
vector<float> transformations;
vector<char*> models;
vector< TiXmlElement*> groupStack;

/*

    1 -> translate
    2 -> rotate
    3 -> scale

    4 -> pushMatrix
    5 -> popMatrix

*/



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


void draw(vector<vertex> vertices) {
    int drawn = 0;
    glBegin(GL_TRIANGLES);
    for (std::size_t i = 0; i < vertices.size(); i += 3) {
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
        glVertex3f(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
        glVertex3f(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
        drawn++;
    }
    cout << "Número de triangulos desenhados: " << drawn;
    glEnd();
}


void readCamera(TiXmlElement* l_pRootElement){
    TiXmlElement* l_camera = l_pRootElement->FirstChildElement("camera");

    if (NULL != l_camera){

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


        TiXmlElement* l_proj = l_camera->FirstChildElement("projection");

        fov = std::stod(l_proj->Attribute("fov"));
        near = std::stod(l_proj->Attribute("near"));
        far = std::stod(l_proj->Attribute("far"));
    }
}


void readModels(){
    /*if (NULL != l_models) {

                    TiXmlElement* l_model = l_models->FirstChildElement("model");

                    while (l_model) {

                        parser(l_model->Attribute("file"));
                        cout << l_model->Attribute("file");
                        l_model = l_model->NextSiblingElement("model");
                    }
                }
                else {
                    printf("models null");
                }
                */
}


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
        //printf("Ficheiro nao carregado");
    }


    for(int i = 0; i < transformations.size() ; i++) {
        cout << transformations[i] << endl;
    }


}

void renderScene(void) {

    // clear buffers
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // set the camera
    glLoadIdentity();
    gluLookAt(x1, t1, z1,
              x2, y2, z2,
              x3, y3, z3);

    glBegin(GL_LINES);


    glColor3f(1.0f, 1.0f, 1.0f);
    glBegin(GL_TRIANGLES);
    for (std::size_t i = 0; i < vertices.size(); i += 3) {
        glVertex3f(vertices[i].x, vertices[i].y, vertices[i].z);
        glVertex3f(vertices[i + 1].x, vertices[i + 1].y, vertices[i + 1].z);
        glVertex3f(vertices[i + 2].x, vertices[i + 2].y, vertices[i + 2].z);
    }
    glEnd();


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