//
// Created by Bruno Filipe Miranda Pereira on 04/04/2022.
//

#ifndef TP1_ENGINE_CLION_XML_PARSE_H
#define TP1_ENGINE_CLION_XML_PARSE_H

#include <GL/glut.h>

#include <string>
#include <vector>
#include <iostream>
#include "../tinyxml.h"

namespace myXML{
    struct TexAndColor {
        char* texFile;
        int diffuse[3];
        int ambient[3];
        int specular[3];
        int emissive[3];
        int shineness;
    };

    struct Lights{
        float point[3];
        float directional[3];
        float spotlight[7];
    };

    class xml_parse{

    public:

        std::vector<float> transformations;
        std::vector<const char*> models;
        std::vector<int> nrPointsPerCurve;
        std::vector<float*> controlPoints;
        std::vector<float> translateTimes;
        std::vector<int> rotateTimes;
        std::vector<char*> translateAligns;
        std::vector<float> rotatePoint;

        //Texturas
        std::vector<TexAndColor*> modelsTexAndColors;
        Lights* lights = (Lights*) malloc(sizeof(struct Lights));

        GLdouble x1, t1, z1, x2, y2, z2, x3, y3, z3, fov, near, far;
        void readTransformations(TiXmlElement* l_group);
        void readLights(TiXmlElement* l_lights);
        void readCamera(TiXmlElement* l_pRootElement);
        void readXML(char* filename);
    };

}

#endif //TP1_ENGINE_CLION_XML_PARSE_H