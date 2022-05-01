//
// Created by Bruno Filipe Miranda Pereira on 31/03/2022.
//

#include "../include/xml_parse.h"
using namespace  std;
using namespace myXML;



void  xml_parse::readTransformations(TiXmlElement* l_group){
    transformations.push_back(4.0f); // push_matrix
    int countCurves = 0;

    TiXmlElement* l_element = l_group->FirstChildElement();

    while (l_element != NULL) {

        if (strcmp(l_element->Value(), "transform") == 0){
            TiXmlElement* l_transform = l_element;

            TiXmlElement* l_transformation = l_transform->FirstChildElement();

            while(l_transformation != NULL){
                if (strcmp(l_transformation->Value(), "translate") == 0) {

                    const char* time = l_transformation->Attribute("time");
                    if (time == NULL) {

                        float translate_x = std::stod(l_transformation->Attribute("x"));
                        float translate_y = std::stod(l_transformation->Attribute("y"));
                        float translate_z = std::stod(l_transformation->Attribute("z"));
                        transformations.push_back(1.0f);
                        transformations.push_back(translate_x);
                        transformations.push_back(translate_y);
                        transformations.push_back(translate_z);
                    }
                    else{
                        translateTime = stof(time);
                        translateAlign = (char*)strdup(l_transformation->Attribute("align"));
                        transformations.push_back(7.0f);
                        TiXmlElement* l_point = l_transformation->FirstChildElement();
                        vector<float*>* vec = (vector<float*>*)(new vector<float>());
                        while(l_point != NULL){
                            float translate_x =  std::stod(l_point->Attribute("x"));
                            float translate_y =  std::stod(l_point->Attribute("y"));
                            float translate_z =  std::stod(l_point->Attribute("z"));

                            float* pt =(float*) malloc(sizeof(float) * 3);
                            pt[0] = translate_x;
                            pt[1] = translate_y;
                            pt[2] = translate_z;
                            vec->push_back(pt);
                            controlPoints[countPoints] = vec;
                            l_point = l_point->NextSiblingElement();
                            countPoints++;
                        }
                        countCurves++;
                    }
                }
                else if (strcmp(l_transformation->Value(), "rotate") == 0) {

                    const char* time = l_transformation->Attribute("time");

                    if (time == NULL) {
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
                    else{
                        rotateTime = std::stod(time);
                        float rotate_x = std::stod(l_transformation->Attribute("x"));
                        float rotate_y = std::stod(l_transformation->Attribute("y"));
                        float rotate_z = std::stod(l_transformation->Attribute("z"));
                        transformations.push_back(8.0f);
                        rotatePoint.push_back(rotate_x);
                        rotatePoint.push_back(rotate_y);
                        rotatePoint.push_back(rotate_z);
                    }
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

void xml_parse::readCamera(TiXmlElement* l_pRootElement){
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

        fov  = std::stod(l_proj->Attribute("fov"));
        near = std::stod(l_proj->Attribute("near"));
        far  = std::stod(l_proj->Attribute("far"));
    }
}

void xml_parse::readXML(char* filename) {

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












