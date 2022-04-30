//
// Created by Bruno Filipe Miranda Pereira on 04/04/2022.
//

#ifndef TP1_ENGINE_CLION_XML_PARSE_H
#define TP1_ENGINE_CLION_XML_PARSE_H

#include <string>
#include <vector>
#include <iostream>
#include <GLUT/glut.h>
#include "../tinyxml.h"

namespace myXML{
    class xml_parse{
    public:
        std::vector<float> transformations;
        std::vector<const char*> models;
        std::vector<float> controlPoints;
        int translateTime;
        int rotateTime;

        const char* translateAlign;

        std::vector<float> rotatePoint;

        GLdouble x1, t1, z1, x2, y2, z2, x3, y3, z3, fov, near, far;
        void readTransformations(TiXmlElement* l_group);
        void readCamera(TiXmlElement* l_pRootElement);
        void readXML(char* filename);
    };

}

#endif //TP1_ENGINE_CLION_XML_PARSE_H
