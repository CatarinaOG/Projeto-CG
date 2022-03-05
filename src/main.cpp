#include <iostream>
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include <GL/glut.h>
#endif
#define _USE_MATH_DEFINES
#include <math.h>
#include <fstream>

char** arguments;
int argumentsLen;

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

void axis_system() {
	glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);

	glBegin(GL_LINES);
	glColor3f(1.0, 0.0, 0.0);
	glVertex3f(0, 0, 0);
	glVertex3f(3, 0, 0);

	glColor3f(0.0, 1.0, 0.0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 3, 0);

	glColor3f(0.0, 0.0, 1.0);
	glVertex3f(0, 0, 0);
	glVertex3f(0, 0, 3);

	glEnd();
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

			MyFile << startx << ";" << 0 << ";" << startz << std::endl;
			MyFile << startx + size << ";" << 0 << ";" << startz << std::endl;
			MyFile << startx + size << ";" << 0 << ";" << startz - size << std::endl;

			glVertex3f(startx, 0, startz);
			glVertex3f(startx + size, 0, startz - size);
			glVertex3f(startx, 0, startz - size);

			MyFile << startx << ";" << 0 << ";" << startz << std::endl;
			MyFile << startx + size << ";" << 0 << ";" << startz - size << std::endl;
			MyFile << startx << ";" << 0 << ";" << startz - size << std::endl;
		}
	}
	glEnd();
	
	MyFile.close();
}

void choosePrimitive() {


	if (strcmp(arguments[1], "sphere ") && argumentsLen >= 6) {
		//drawSphere((int)arguments[2], (int)arguments[3], (int)arguments[4], arguments[5]);
	}

	if (strcmp(arguments[1], "box ") && argumentsLen >= 5) {
		//drawBox((int)arguments[2], (int)arguments[3], arguments[4]);
	}

	if (strcmp(arguments[1], "cone ") && argumentsLen >= 7) {
		//drawCone((int)arguments[2], (int)arguments[3], (int)arguments[4], (int)arguments[5], arguments[6]);
	}

	if (strcmp(arguments[1], "plane ") && argumentsLen >= 5) {
		drawPlane(atoi(arguments[2]), atoi(arguments[3]), arguments[4]);
	}

}

void renderScene(void) {

	// clear buffers
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera
	glLoadIdentity();
	gluLookAt(1.5, 1, 1,
		0.0, 0.0, 0.0,
		0.0f, 1.0f, 0.0f);

	glPolygonMode(GL_FRONT, GL_LINE);
	axis_system();
	glColor3f(1, 1, 1);

	choosePrimitive();

	// End of frame
	glutSwapBuffers();

}




int main(int argc, char **argv) {

	arguments = argv;
	argumentsLen = argc;

// init GLUT and the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH|GLUT_DOUBLE|GLUT_RGBA);
	glutInitWindowPosition(100,100);
	glutInitWindowSize(800,800);
	glutCreateWindow("CG@DI-UM");
		
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
