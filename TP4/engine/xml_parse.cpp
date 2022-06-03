#include "../include/xml_parse.h"
using namespace  std;
using namespace myXML;

/*
void fillAsDefault(TexAndColor texAndColor){

     texAndColor.texFile = "";
     texAndColor.diffuse = {200,200,200};
     texAndColor.ambient = {50,50,50};
     texAndColor.specular = {0,0,0};
     texAndColor.emissive = {0,0,0};
     texAndColor.shininess = 0;

 }

 */

void  xml_parse::readTransformations(TiXmlElement* l_group){
    transformations.push_back(4.0f); // push_matrix
    int countCurves = 0;
    int countPoints;

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
                        translateTimes.push_back(stof(time));
                        translateAligns.push_back((char*)strdup(l_transformation->Attribute("align")));
                        transformations.push_back(7.0f);
                        TiXmlElement* l_point = l_transformation->FirstChildElement();

                        countPoints = 0;

                        while(l_point != NULL){
                            float translate_x =  std::stod(l_point->Attribute("x"));
                            float translate_y =  std::stod(l_point->Attribute("y"));
                            float translate_z =  std::stod(l_point->Attribute("z"));

                            float* pt =(float*) malloc(sizeof(float) * 3);
                            pt[0] = translate_x;
                            pt[1] = translate_y;
                            pt[2] = translate_z;
                            controlPoints.push_back(pt);
                            l_point = l_point->NextSiblingElement();
                            countPoints++;
                        }
                        nrPointsPerCurve.push_back(countPoints);
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
                        rotateTimes.push_back(std::stod(time));
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
            if (strcmp(l_element->Value(), "lights") == 0){
                TiXmlElement* l_lights = l_element;
                TiXmlElement* l_light = l_lights->FirstChildElement();

                //lights = malloc(sizeof(Lights));

                while(l_light != NULL){

                    const char* light_type = l_light->Attribute("type");

                    if(strcmp(light_type,"point") == 0){
                        const int pointPosX = atoi(l_light->Attribute("posX"));
                        const int pointPosY = atoi(l_light->Attribute("posY"));
                        const int pointPosZ = atoi(l_light->Attribute("posZ"));

                        //lights.point = {pointPosX, pointPosY, pointPosZ};

                    }
                    else if(strcmp(light_type,"directional") == 0){
                        const int directionalDirX = atoi(l_light->Attribute("dirX"));
                        const int directionalDirY = atoi(l_light->Attribute("dirY"));
                        const int directionalDirZ = atoi(l_light->Attribute("dirZ"));

                        //lights.directional = {directionalDirX, directionalDirY, directionalDirZ};
                    }
                    else {
                        const int spotlightPosX = atoi(l_light->Attribute("posX"));
                        const int spotlightPosY = atoi(l_light->Attribute("posY"));
                        const int spotlightPosZ = atoi(l_light->Attribute("posZ"));

                        const int spotlightDirX = atoi(l_light->Attribute("dirX"));
                        const int spotlightDirY = atoi(l_light->Attribute("dirY"));
                        const int spotlightDirZ = atoi(l_light->Attribute("dirZ"));

                        const int spotlightCutOff = atoi(l_light->Attribute("cutoff"));

                        //lights.spotlight = {spotlightPosX, spotlightPosY, spotlightPosZ,
                        //                    spotlightDirX, spotlightDirY, spotlightDirZ,
                        //                    spotlightCutOff};
                    }

                    l_light = l_light->NextSiblingElement();
                }


            }
            else{
                TiXmlElement* l_models = l_element;
                TiXmlElement* l_model = l_models->FirstChildElement();

                while (l_model != NULL){
                    const char* model = strdup(l_model->Attribute("file"));
                    models.push_back(model);
                    transformations.push_back(6.0f);

                    TiXmlElement* l_model_propriety = l_model->FirstChildElement();

                    //TexAndColor texAndColor = malloc(sizeof(TextAndColor));
                    //fillAsDefault(texAndColor);

                    while(l_model_propriety != NULL) {
                        if (strcmp(l_model_propriety->Value(), "texture") == 0) {
                            TiXmlElement *l_model_texture = l_model_propriety;
                            const char *textureFile = strdup(l_model_texture->Attribute("file"));
                            //texAndColor.textFile = textureFile;

                        }
                        else{
                            TiXmlElement* l_model_color = l_model_propriety;
                            TiXmlElement* l_model_color_propriety = l_model->FirstChildElement();

                            while(l_model_color_propriety != NULL){

                                if (strcmp(l_model_color_propriety->Value(), "diffuse") == 0){
                                    const int diffuseR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int diffuseG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int diffuseB = atoi(l_model_color_propriety->Attribute("B"));

                                    //texAndColor.diffuse = {diffuseR, diffuseG,diffuseB};
                                }
                                else if (strcmp(l_model_color_propriety->Value(), "ambient") == 0) {
                                    const int ambientR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int ambientG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int ambientB = atoi(l_model_color_propriety->Attribute("B"));

                                    //texAndColor.ambient = {ambientR, ambientG,ambientB};
                                }
                                else if (strcmp(l_model_color_propriety->Value(), "specular") == 0){
                                    const int specularR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int specularG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int specularB = atoi(l_model_color_propriety->Attribute("B"));

                                    //texAndColor.specular = {specularR, specularG,specularB};
                                }
                                else if (strcmp(l_model_color_propriety->Value(), "emissive") == 0) {
                                    const int emissiveR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int emissiveG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int emissiveB = atoi(l_model_color_propriety->Attribute("B"));

                                    //texAndColor.emissive = {emissiveR, emissiveG,emissiveB};
                                }
                                else{
                                    const int shininess = atoi(l_model_color_propriety->Attribute("value"));

                                    //texAndColor.shineness = shininess;
                                }

                                l_model_color_propriety = l_model_color_propriety->NextSiblingElement();
                            }
                        }
                        l_model_propriety = l_model_propriety->NextSiblingElement();
                    }

                    //modelsTexAndColors.push_back(texAndColor);
                    l_model = l_model->NextSiblingElement();
                }
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
        t1 = std::stod(l_position->Attribute("y"));
        z1 = std::stod(l_position->Attribute("z"));


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
