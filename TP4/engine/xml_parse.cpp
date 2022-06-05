#include "../include/xml_parse.h"
using namespace  std;
using namespace myXML;



void fillAsDefault(TexAndColor* texAndColor){

     texAndColor->texFile = NULL;
     texAndColor->diffuse[0] = 200;
     texAndColor->diffuse[1] = 200;
     texAndColor->diffuse[2] = 200;
     texAndColor->ambient[0] = 50;
     texAndColor->ambient[1] = 50;
     texAndColor->ambient[2] = 50;
     texAndColor->specular[0] = 0;
     texAndColor->specular[1] = 0;
     texAndColor->specular[2] = 0;
     texAndColor->emissive[0] = 0;
     texAndColor->emissive[1] = 0;
     texAndColor->emissive[2] = 0;
     texAndColor->shininess = 0;

 }


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
                TiXmlElement* l_models = l_element;
                TiXmlElement* l_model = l_models->FirstChildElement();
                printf("model: %s\n",l_model->Value());

            while (l_model != NULL){
                    const char* model = strdup(l_model->Attribute("file"));
                    models.push_back(model);
                    transformations.push_back(6.0f);

                    TiXmlElement* l_model_propriety = l_model->FirstChildElement();

                    TexAndColor* texAndColor = (TexAndColor*) malloc(sizeof(struct TexAndColor));
                    fillAsDefault(texAndColor);

                    while(l_model_propriety != NULL) {

                        if (strcmp(l_model_propriety->Value(), "texture") == 0) {

                            TiXmlElement *l_model_texture = l_model_propriety;
                            texAndColor->texFile = strdup(l_model_texture->Attribute("file"));
                            printf("file: %s\n",l_model_texture->Attribute("file"));
                        }
                        else{
                            TiXmlElement* l_model_color = l_model_propriety;
                            TiXmlElement* l_model_color_propriety = l_model_color->FirstChildElement();

                            while(l_model_color_propriety != NULL){

                                if (strcmp(l_model_color_propriety->Value(), "diffuse") == 0){
                                    const int diffuseR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int diffuseG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int diffuseB = atoi(l_model_color_propriety->Attribute("B"));

                                    texAndColor->diffuse[0] = diffuseR;
                                    texAndColor->diffuse[1] = diffuseG;
                                    texAndColor->diffuse[2] = diffuseB;
                                }
                                else if (strcmp(l_model_color_propriety->Value(), "ambient") == 0) {
                                    const int ambientR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int ambientG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int ambientB = atoi(l_model_color_propriety->Attribute("B"));

                                    texAndColor->ambient[0] = ambientR;
                                    texAndColor->ambient[1] = ambientG;
                                    texAndColor->ambient[2] = ambientB;
                                }
                                else if (strcmp(l_model_color_propriety->Value(), "specular") == 0){
                                    const int specularR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int specularG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int specularB = atoi(l_model_color_propriety->Attribute("B"));

                                    texAndColor->specular[0] = specularR;
                                    texAndColor->specular[1] = specularG;
                                    texAndColor->specular[2] = specularB;
                                }
                                else if (strcmp(l_model_color_propriety->Value(), "emissive") == 0) {
                                    const int emissiveR = atoi(l_model_color_propriety->Attribute("R"));
                                    const int emissiveG = atoi(l_model_color_propriety->Attribute("G"));
                                    const int emissiveB = atoi(l_model_color_propriety->Attribute("B"));

                                    texAndColor->emissive[0] = emissiveR;
                                    texAndColor->emissive[1] = emissiveG;
                                    texAndColor->emissive[2] = emissiveB;
                                }
                                else{
                                    int shininess = atof(l_model_color_propriety->Attribute("value"));
                                    texAndColor->shininess = shininess;
                                }

                                l_model_color_propriety = l_model_color_propriety->NextSiblingElement();
                            }
                        }
                        l_model_propriety = l_model_propriety->NextSiblingElement();
                    }

                    modelsTexAndColors.push_back(texAndColor);
                    l_model = l_model->NextSiblingElement();
                }
        }
        l_element = l_element->NextSiblingElement(); //avançar o elemento XML
    }
    transformations.push_back(5.0f); // pop_matrix
}

void xml_parse::readLights(TiXmlElement* l_lights){

    TiXmlElement* l_light = l_lights->FirstChildElement();

    while(l_light != NULL){
        const char* light_type = l_light->Attribute("type");

        if(strcmp(light_type,"point") == 0){

            LightPoint* l = (LightPoint*) malloc(sizeof(struct LightPoint));

            l->posX = atof(l_light->Attribute("posx"));
            l->posY = atof(l_light->Attribute("posy"));
            l->posZ = atof(l_light->Attribute("posz"));

            lightsPoint.push_back(l);

        }
        else if(strcmp(light_type,"directional") == 0){

            LightDirectional* l = (LightDirectional*) malloc(sizeof(struct LightDirectional));

            l->dirX = atof(l_light->Attribute("dirx"));
            l->dirY = atof(l_light->Attribute("diry"));
            l->dirZ = atof(l_light->Attribute("dirz"));

            lightsDirectional.push_back(l);
        }
        else {

            LightSpotlight* l = (LightSpotlight*) malloc(sizeof(struct LightSpotlight));

            l->posX = atof(l_light->Attribute("posx"));
            l->posY = atof(l_light->Attribute("posy"));
            l->posZ = atof(l_light->Attribute("posz"));

            l->dirX = atof(l_light->Attribute("dirx"));
            l->dirX = atof(l_light->Attribute("diry"));
            l->dirX = atof(l_light->Attribute("dirz"));

            l->cutoff = atof(l_light->Attribute("cutoff"));

            lightsSpotlight.push_back(l);
        }

        l_light = l_light->NextSiblingElement();
    }

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

            TiXmlElement* l_lights = l_pRootElement->FirstChildElement("lights");
            if(l_lights != NULL) readLights(l_lights);

            TiXmlElement* l_group = l_pRootElement->FirstChildElement("group");
            readTransformations(l_group);


        }
    }
    else {
        printf("Ficheiro nao carregado: %s", filename);
    }
}
