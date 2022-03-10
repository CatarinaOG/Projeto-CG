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
#include "tinyxml.h"


GLdouble x1, t1, z1, x2, y2, z2, x3, y3, z3;
char* filename;

void changeSize(int w, int h) {

	// Prevent a divide by zero, when window is too short
	// (you cant make a window with zero width).
	if(h == 0)
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
	gluPerspective(45.0f ,ratio, 1.0f ,1000.0f);

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

void readXML() {

	TiXmlDocument doc(filename);
	bool loadOkay = doc.LoadFile();
	
	if (loadOkay) {

		TiXmlElement* l_pRootElement = doc.FirstChildElement("world");

		if (l_pRootElement != NULL){

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
			}

            TiXmlElement* l_group = l_pRootElement->FirstChildElement("group");

            if (NULL != l_group) {

                TiXmlElement* l_models = l_group->FirstChildElement("models");

                if (NULL != l_models) {

                    TiXmlElement* l_model = l_models->FirstChildElement("model");

                    while (l_model) {

                        printf("%s\n",l_model->Attribute("file")); //aqui vai função para gerar primitiva
                        l_model = l_model->NextSiblingElement("model");
                    }
                }
                else {
                    printf("models null");
                }
            }
		}
	}
	else {
		printf("Ficheiro não carregado");
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

    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    drawCone(1, 2, 4, 3);

    // End of frame
    glutSwapBuffers();
}


int main(int argc, char **argv) {

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
    filename = argv[1];
    readXML();

// Required callback registry 
	glutDisplayFunc(renderScene);
	glutReshapeFunc(changeSize);
	
//  OpenGL settings
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	
// enter GLUT's main cycle
	glutMainLoop();
	
	return 1;
}
