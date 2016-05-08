#include <glew.h>
#include <freeglut.h>
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cstdio>
#include <cstdlib>
#include <tiny_obj_loader.h>
#include "fbxloader.h"
#include <IL/il.h>
#include <iostream>

#define MENU_TIMER_START 1
#define MENU_TIMER_STOP 2
#define MENU_EXIT 3
#define ZW 4
#define ZF 5

float eyex,eyey;
float deltaAngle = 0.0f;
int xOrigin = -1;
float angles = 0.0f;
float viewportAspect;
float a=-4.0f,b=1.0f,c=0.0f;
GLubyte timer_cnt = 0;
bool timer_enabled = true;
unsigned int timer_speed = 16;

using namespace glm;
int walking =1;
int fury=0;


mat4 mvp;
float animate=0.1f;
int dir=0;
GLint um4mvp;

std::vector<tinyobj::shape_t> shapes;
std::vector<tinyobj::material_t> materials;
std::vector<tinyobj::shape_t> shapesz;
std::vector<tinyobj::material_t> materialsz;
std::vector<tinyobj::shape_t> shapesz2;
std::vector<tinyobj::material_t> materialsz2;
fbx_handles myFbx;
fbx_handles myFbx2;

std::vector <GLuint>textures;
typedef struct
{
    GLuint vao;
    GLuint vbo[2];
    GLuint normal;
    GLuint tex;
    GLuint element;
    int matid;
    int count;
} ee;

std::vector<ee> ss;
std::vector<ee> zs;
std::vector <GLuint>zt;
std::vector<ee> zs2;
std::vector <GLuint>zt2;

void checkError(const char *functionName)
{
    GLenum error;
    while (( error = glGetError() ) != GL_NO_ERROR)
    {
        fprintf (stderr, "GL error 0x%X detected in %s\n", error, functionName);
    }
}

// Print OpenGL context related information.
void dumpInfo(void)
{
    printf("Vendor: %s\n", glGetString (GL_VENDOR));
    printf("Renderer: %s\n", glGetString (GL_RENDERER));
    printf("Version: %s\n", glGetString (GL_VERSION));
    printf("GLSL: %s\n", glGetString (GL_SHADING_LANGUAGE_VERSION));
}

char** loadShaderSource(const char* file)
{
    FILE* fp = fopen(file, "rb");
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *src = new char[sz + 1];
    fread(src, sizeof(char), sz, fp);
    src[sz] = '\0';
    char **srcp = new char*[1];
    srcp[0] = src;
    return srcp;
}

void freeShaderSource(char** srcp)
{
    delete srcp[0];
    delete srcp;
}

void shaderLog(GLuint shader)
{
    GLint isCompiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
    if(isCompiled == GL_FALSE)
    {
        GLint maxLength = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

        // The maxLength includes the NULL character
        GLchar* errorLog = new GLchar[maxLength];
        glGetShaderInfoLog(shader, maxLength, &maxLength, &errorLog[0]);

        printf("%s\n", errorLog);
        delete errorLog;
    }
}

void My_Init()
{
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    GLuint program = glCreateProgram();
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    char** vertexShaderSource = loadShaderSource("vertex.vs.glsl");
    char** fragmentShaderSource = loadShaderSource("fragment.fs.glsl");
    glShaderSource(vertexShader, 1, vertexShaderSource, NULL);
    glShaderSource(fragmentShader, 1, fragmentShaderSource, NULL);
    freeShaderSource(vertexShaderSource);
    freeShaderSource(fragmentShaderSource);
    glCompileShader(vertexShader);
    glCompileShader(fragmentShader);
    shaderLog(vertexShader);
    shaderLog(fragmentShader);
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    um4mvp = glGetUniformLocation(program, "um4mvp");
    glUseProgram(program);


}

void My_LoadModels()
{


    std::string err;

    bool ret = tinyobj::LoadObj(shapes, materials, err, "sponza.obj");
    if(ret==false)
        printf("error loading sponza.obj\n");
//	printf("material.size:%d,shapes.size:%d\n",materials.size(),shapes.size());
    // TODO: If You Want to Load FBX, Use these. The Returned Values are The Same.

    // Save this Object, You Will Need It to Retrieve Animations Later.
    // bool ret = LoadFbx(myFbx, shapes, materials, err, "zombie_walk.FBX");
   // printf("material.size:%d,shapes.size:%d\n",materials.size(),shapes.size());

    if(ret)
    {

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);

        // For Each Material
        for(int i = 0; i < materials.size(); i++)
        {
            // materials[i].diffuse_texname; // This is the Texture Path You Need
            GLuint tes;
            glGenTextures(1, &tes);
            ILuint ilTexName;
            ilGenImages(1, &ilTexName);
            ilBindImage(ilTexName);
            //printf("material size:%d\n",materials.size());
            if(ilLoadImage(materials[i].diffuse_texname.c_str()))
            {
                unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 3];
                // Image Width = ilGetInteger(IL_IMAGE_WIDTH)
                // Image Height = ilGetInteger(IL_IMAGE_HEIGHT)
                ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGB, IL_UNSIGNED_BYTE, data);



                // TODO: Generate an OpenGL Texture and use the [unsigned char *data] as Input Here.
                glBindTexture(GL_TEXTURE_2D, tes);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGB, GL_UNSIGNED_BYTE, data);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                delete[] data;
                ilDeleteImages(1, &ilTexName);
            }
            textures.push_back(tes);


        }
       // printf("textures.size()=%d\n",textures.size());


        //printf("shapes size:%d\n",shapes.size());
        // For Each Shape (or Mesh, Object)
        for(int i = 0; i < shapes.size(); i++)
        {
            ee k;
            glGenVertexArrays(1,&k.vao);
            glBindVertexArray(k.vao);
            // shapes[i].mesh.positions; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.normals; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.texcoords; // VertexCount * 2 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.indices; // TriangleCount * 3 Unsigned Integers, Load Them to a GL_ELEMENT_ARRAY_BUFFER
            // shapes[i].mesh.material_ids[0] // The Material ID of This Shape

            glGenBuffers(1, &k.vbo[0]);
            glGenBuffers(1, &k.normal);
            glGenBuffers(1, &k.vbo[1]);
            glGenBuffers(1, &k.element);


            glBindBuffer(GL_ARRAY_BUFFER, k.vbo[0]);
            glBufferData(GL_ARRAY_BUFFER,shapes[i].mesh.positions.size()*sizeof(float), shapes[i].mesh.positions.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ARRAY_BUFFER, k.normal);
            glBufferData(GL_ARRAY_BUFFER,shapes[i].mesh.normals.size()*sizeof(float), shapes[i].mesh.normals.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ARRAY_BUFFER, k.vbo[1]);
            glBufferData(GL_ARRAY_BUFFER,shapes[i].mesh.texcoords.size()*sizeof(float), shapes[i].mesh.texcoords.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, k.element);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,shapes[i].mesh.indices.size()*sizeof(unsigned int), shapes[i].mesh.indices.data(),GL_STATIC_DRAW);

            k.matid=shapes[i].mesh.material_ids[0];
            k.count = shapes[i].mesh.indices.size();


            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);




            ss.push_back(k);
            // TODO:
            // 1. Generate and Bind a VAO
            // 2. Generate and Bind a Buffer for position/normal/texcoord
            // 3. Upload Data to The Buffers
            // 4. Generate and Bind a Buffer for indices (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)
            // 5. glVertexAttribPointer Calls (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)



        }
      //  printf("ss.size()=%d\n",ss.size());
    }
}

void My_Loadzombie()
{


    std::string err;

    //bool ret = tinyobj::LoadObj(shapes, materials, err, "sponza.obj");
    //if(ret==false)
    //	printf("error loading sponza.obj\n");
//	printf("material.size:%d,shapes.size:%d\n",materials.size(),shapes.size());
    // TODO: If You Want to Load FBX, Use these. The Returned Values are The Same.

    // Save this Object, You Will Need It to Retrieve Animations Later.
    bool ret = LoadFbx(myFbx, shapesz, materialsz, err, "zombie_walk.FBX");
	if(ret!=true)
		printf("error loading zombie_walk.FBX\n");
  //  printf("material.size:%d,shapes.size:%d\n",materialsz.size(),shapesz.size());

    if(ret)
    {

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);

        // For Each Material
        for(int i = 0; i < materialsz.size(); i++)
        {
            // materials[i].diffuse_texname; // This is the Texture Path You Need
            GLuint tes;
            glGenTextures(1, &tes);
            ILuint ilTexName;
            ilGenImages(1, &ilTexName);
            ilBindImage(ilTexName);
            //printf("material size:%d\n",materials.size());
            if(ilLoadImage(materialsz[i].diffuse_texname.c_str()))
            {
                unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 3];
                // Image Width = ilGetInteger(IL_IMAGE_WIDTH)
                // Image Height = ilGetInteger(IL_IMAGE_HEIGHT)
                ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGB, IL_UNSIGNED_BYTE, data);



                // TODO: Generate an OpenGL Texture and use the [unsigned char *data] as Input Here.
                glBindTexture(GL_TEXTURE_2D, tes);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGB, GL_UNSIGNED_BYTE, data);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                delete[] data;
                ilDeleteImages(1, &ilTexName);
            }
            zt.push_back(tes);


        }
     //   printf("textures.size()=%d\n",zt.size());


        //printf("shapes size:%d\n",shapes.size());
        // For Each Shape (or Mesh, Object)
        for(int i = 0; i < shapesz.size(); i++)
        {
            ee k;
            glGenVertexArrays(1,&k.vao);
            glBindVertexArray(k.vao);
            // shapes[i].mesh.positions; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.normals; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.texcoords; // VertexCount * 2 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.indices; // TriangleCount * 3 Unsigned Integers, Load Them to a GL_ELEMENT_ARRAY_BUFFER
            // shapes[i].mesh.material_ids[0] // The Material ID of This Shape

            glGenBuffers(1, &k.vbo[0]);
            glGenBuffers(1, &k.normal);
            glGenBuffers(1, &k.vbo[1]);
            glGenBuffers(1, &k.element);


            glBindBuffer(GL_ARRAY_BUFFER, k.vbo[0]);
            glBufferData(GL_ARRAY_BUFFER,shapesz[i].mesh.positions.size()*sizeof(float), shapesz[i].mesh.positions.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ARRAY_BUFFER, k.normal);
            glBufferData(GL_ARRAY_BUFFER,shapesz[i].mesh.normals.size()*sizeof(float), shapesz[i].mesh.normals.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ARRAY_BUFFER, k.vbo[1]);
            glBufferData(GL_ARRAY_BUFFER,shapesz[i].mesh.texcoords.size()*sizeof(float), shapesz[i].mesh.texcoords.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, k.element);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,shapesz[i].mesh.indices.size()*sizeof(unsigned int), shapesz[i].mesh.indices.data(),GL_STATIC_DRAW);

            k.matid=shapesz[i].mesh.material_ids[0];
            k.count = shapesz[i].mesh.indices.size();


            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);




            zs.push_back(k);
            // TODO:
            // 1. Generate and Bind a VAO
            // 2. Generate and Bind a Buffer for position/normal/texcoord
            // 3. Upload Data to The Buffers
            // 4. Generate and Bind a Buffer for indices (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)
            // 5. glVertexAttribPointer Calls (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)



        }
     //   printf("zs.size()=%d\n",zs.size());
    }
}

void My_Loadzombie2()
{


    std::string err;

    //bool ret = tinyobj::LoadObj(shapes, materials, err, "sponza.obj");
    //if(ret==false)
    //	printf("error loading sponza.obj\n");
//	printf("material.size:%d,shapes.size:%d\n",materials.size(),shapes.size());
    // TODO: If You Want to Load FBX, Use these. The Returned Values are The Same.

    // Save this Object, You Will Need It to Retrieve Animations Later.
    bool ret = LoadFbx(myFbx2, shapesz2, materialsz2, err, "zombie_fury.FBX");
	if(ret!=true)
		printf("error loading zombie_walk.FBX\n");
  //  printf("material.size:%d,shapes.size:%d\n",materialsz2.size(),shapesz2.size());

    if(ret)
    {

        glActiveTexture(GL_TEXTURE0);
        glEnable(GL_TEXTURE_2D);

        // For Each Material
        for(int i = 0; i < materialsz2.size(); i++)
        {
            // materials[i].diffuse_texname; // This is the Texture Path You Need
            GLuint tes;
            glGenTextures(1, &tes);
            ILuint ilTexName;
            ilGenImages(1, &ilTexName);
            ilBindImage(ilTexName);
            //printf("material size:%d\n",materials.size());
            if(ilLoadImage(materialsz2[i].diffuse_texname.c_str()))
            {
                unsigned char *data = new unsigned char[ilGetInteger(IL_IMAGE_WIDTH) * ilGetInteger(IL_IMAGE_HEIGHT) * 3];
                // Image Width = ilGetInteger(IL_IMAGE_WIDTH)
                // Image Height = ilGetInteger(IL_IMAGE_HEIGHT)
                ilCopyPixels(0, 0, 0, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 1, IL_RGB, IL_UNSIGNED_BYTE, data);



                // TODO: Generate an OpenGL Texture and use the [unsigned char *data] as Input Here.
                glBindTexture(GL_TEXTURE_2D, tes);

                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, ilGetInteger(IL_IMAGE_WIDTH), ilGetInteger(IL_IMAGE_HEIGHT), 0, GL_RGB, GL_UNSIGNED_BYTE, data);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
                delete[] data;
                ilDeleteImages(1, &ilTexName);
            }
            zt2.push_back(tes);


        }



        //printf("shapes size:%d\n",shapes.size());
        // For Each Shape (or Mesh, Object)
        for(int i = 0; i < shapesz2.size(); i++)
        {
            ee k;
            glGenVertexArrays(1,&k.vao);
            glBindVertexArray(k.vao);
            // shapes[i].mesh.positions; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.normals; // VertexCount * 3 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.texcoords; // VertexCount * 2 Floats, Load Them to a GL_ARRAY_BUFFER
            // shapes[i].mesh.indices; // TriangleCount * 3 Unsigned Integers, Load Them to a GL_ELEMENT_ARRAY_BUFFER
            // shapes[i].mesh.material_ids[0] // The Material ID of This Shape

            glGenBuffers(1, &k.vbo[0]);
            glGenBuffers(1, &k.normal);
            glGenBuffers(1, &k.vbo[1]);
            glGenBuffers(1, &k.element);


            glBindBuffer(GL_ARRAY_BUFFER, k.vbo[0]);
            glBufferData(GL_ARRAY_BUFFER,shapesz2[i].mesh.positions.size()*sizeof(float), shapesz2[i].mesh.positions.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ARRAY_BUFFER, k.normal);
            glBufferData(GL_ARRAY_BUFFER,shapesz2[i].mesh.normals.size()*sizeof(float), shapesz2[i].mesh.normals.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ARRAY_BUFFER, k.vbo[1]);
            glBufferData(GL_ARRAY_BUFFER,shapesz2[i].mesh.texcoords.size()*sizeof(float), shapesz2[i].mesh.texcoords.data(),GL_STATIC_DRAW);
            glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,NULL);

            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, k.element);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER,shapesz2[i].mesh.indices.size()*sizeof(unsigned int), shapesz2[i].mesh.indices.data(),GL_STATIC_DRAW);

            k.matid=shapesz2[i].mesh.material_ids[0];
            k.count = shapesz2[i].mesh.indices.size();


            glEnableVertexAttribArray(0);
            glEnableVertexAttribArray(1);
            glEnableVertexAttribArray(2);




            zs2.push_back(k);
            // TODO:
            // 1. Generate and Bind a VAO
            // 2. Generate and Bind a Buffer for position/normal/texcoord
            // 3. Upload Data to The Buffers
            // 4. Generate and Bind a Buffer for indices (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)
            // 5. glVertexAttribPointer Calls (Will Be Saved In The VAO, You Can Restore Them By Binding The VAO)



        }
    }
}

// GLUT callback. Called to draw the scene.
void My_Display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    mvp = perspective(radians(45.0f), viewportAspect, 0.1f, 100.0f);
    mvp = mvp * lookAt(vec3(a, b, c), vec3(1.0f, 1.0f, eyey), vec3(0.0f, 1.0f, 0.0f));


    int shapeCount = shapes.size();
    // TODO:
    // For Each Shapes You Loaded In My_LoadModels()
    for(int i = 0; i < shapeCount; i++)
    {
        glBindVertexArray(ss[i].vao);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ss[i].element);
        glBindTexture(GL_TEXTURE_2D, textures[ss[i].matid]);
        mat4 scmvp = mvp * scale(mat4(), vec3(0.01f, 0.01f, 0.01f));
        glUniformMatrix4fv(um4mvp, 1, GL_FALSE, &scmvp[0][0]);
        //	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
        glDrawElements(GL_TRIANGLES, ss[i].count, GL_UNSIGNED_INT, 0);
        // 1. Bind The VAO of the Shape

        // 3. Bind Textures
        // 4. Update Uniform Values by glUniform*
        // 5. glDrawElements Call
    }

    // TODO: For Your FBX Model, Get New Animation Here
    if(walking==1)
    {
        std::vector<tinyobj::shape_t> new_shapes;
        GetFbxAnimation(myFbx, new_shapes, animate); // The Last Parameter is A Float in [0, 1], Specifying The Animation Location You Want to Retrieve
        for(int i = 0; i < new_shapes.size(); i++)
        {
            glBindVertexArray(zs[i].vao);
            glBindBuffer(GL_ARRAY_BUFFER,zs[i].vbo[0]);
            glBufferSubData(GL_ARRAY_BUFFER,0,new_shapes[i].mesh.positions.size()*sizeof(float),new_shapes[i].mesh.positions.data());
            // new_shapes[i].mesh.positions; // VertexCount * 3 Floats, Updated Vertex Positions of This Shape
            // 1. Bind The Buffer You Created In My_LoadModels() For This Shape
            // 2. Update Position Data by glBufferSubData() Call
        }

        shapeCount = shapesz.size();
        for(int i = 0; i < shapeCount; i++)
        {
            glBindVertexArray(zs[i].vao);
            glBindTexture(GL_TEXTURE_2D, zt[zs[i].matid]);
            mat4 scmvp = mvp ;
            scmvp = rotate(scmvp,-89.5f,vec3(1.0f,0.0f,0.0f));
            scmvp = translate(scmvp,vec3(0.0f,0.0f,0.55f));
            scmvp =  scale(scmvp, vec3(0.05f, 0.05f, 0.05f));
            glUniformMatrix4fv(um4mvp, 1, GL_FALSE, &scmvp[0][0]);
            //	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
            glDrawElements(GL_TRIANGLES, zs[i].count, GL_UNSIGNED_INT, 0);
            // 1. Bind The VAO of the Shape

            // 3. Bind Textures
            // 4. Update Uniform Values by glUniform*
            // 5. glDrawElements Call
        }

    }
    if(fury==1)
    {
        std::vector<tinyobj::shape_t> new_shapes;
        GetFbxAnimation(myFbx2, new_shapes, animate); // The Last Parameter is A Float in [0, 1], Specifying The Animation Location You Want to Retrieve
        for(int i = 0; i < new_shapes.size(); i++)
        {
            glBindVertexArray(zs2[i].vao);
            glBindBuffer(GL_ARRAY_BUFFER,zs2[i].vbo[0]);
            glBufferSubData(GL_ARRAY_BUFFER,0,new_shapes[i].mesh.positions.size()*sizeof(float),new_shapes[i].mesh.positions.data());
            // new_shapes[i].mesh.positions; // VertexCount * 3 Floats, Updated Vertex Positions of This Shape
            // 1. Bind The Buffer You Created In My_LoadModels() For This Shape
            // 2. Update Position Data by glBufferSubData() Call
        }


        shapeCount = shapesz2.size();
        for(int i = 0; i < shapeCount; i++)
        {
            glBindVertexArray(zs2[i].vao);
            glBindTexture(GL_TEXTURE_2D, zt2[zs2[i].matid]);
            mat4 scmvp = mvp ;
            scmvp = rotate(scmvp,-89.5f,vec3(1.0f,0.0f,0.0f));
            scmvp = translate(scmvp,vec3(0.0f,0.0f,0.55f));
            scmvp =  scale(scmvp, vec3(0.05f, 0.05f, 0.05f));
            glUniformMatrix4fv(um4mvp, 1, GL_FALSE, &scmvp[0][0]);
            //	glUniformMatrix4fv(um4mvp, 1, GL_FALSE, value_ptr(mvp));
            glDrawElements(GL_TRIANGLES, zs2[i].count, GL_UNSIGNED_INT, 0);
            // 1. Bind The VAO of the Shape

            // 3. Bind Textures
            // 4. Update Uniform Values by glUniform*
            // 5. glDrawElements Call
        }

    }



    glutSwapBuffers();
}


void My_Reshape(int width, int height)
{
    glViewport(0, 0, width, height);



    viewportAspect = (float)width / (float)height;


}

void My_Timer(int val)
{
    timer_cnt++;
    if(animate>=1.0f)
        animate=0.0f;
    animate+=0.005f;

    //printf("animate:%f\n",animate);
    glutPostRedisplay();
    if(timer_enabled)
    {
        glutTimerFunc(timer_speed, My_Timer, val);
    }
}

void mouseMove(int x, int y)
{

    // this will only be true when the left button is down
    if (xOrigin >= 0)
    {

        // update deltaAngle
        deltaAngle = (x - xOrigin) * 0.01f;
		if(deltaAngle>3.0f)
			deltaAngle=3.0f;
		else if(deltaAngle<-3.0f)
			deltaAngle=-3.0f;
		if(angles>3.0f)
			angles=3.0f;
		else if(angles<0.0f)
			angles=0.0f;
        float temp;
        temp=angles+deltaAngle;
      // printf("angles,deltaAngle=%f,%f\n",angles,deltaAngle);
        if(temp>3.0f)
            temp=3.0f;
        else if(temp<0.0f)
            temp=0.0f;
 
        // update camera's direction
        eyex = sin(temp);
        eyey= -cos(temp);
    }
}
void My_Mouse(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {

        // when the button is released
        if (state == GLUT_UP)
        {
            angles += deltaAngle;
            xOrigin = -1;
        }
        else   // state = GLUT_DOWN
        {
            xOrigin = x;
        }
    }
    if(state == GLUT_DOWN)
    {
        printf("Mouse %d is pressed at (%d, %d)\n", button, x, y);
    }
    else if(state == GLUT_UP)
    {
        printf("Mouse %d is released at (%d, %d)\n", button, x, y);
    }
}

void My_Keyboard(unsigned char key, int x, int y)
{
    if (key=='w'||key=='W')
        a+=0.5f;
    else a+=0.0f;
    if (key=='a'||key=='A')
        c-=0.5f;
    else c-=0.0f;
    if (key=='s'||key=='S')
        a-=0.5f;
    else a-=0.0f;
    if (key=='d'||key=='D')
        c+=0.5f;
    else c-=0.0f;
    if (key=='z'||key=='Z')
        b+=0.5f;
    else b+=0.0f;
    if (key=='x'||key=='X')
        b-=0.5f;
    else b-=0.0f;


    printf("Key %c is pressed at (%d, %d)\n", key, x, y);
}

void My_SpecialKeys(int key, int x, int y)
{
    switch(key)
    {
    case GLUT_KEY_F1:
        printf("F1 is pressed at (%d, %d)\n", x, y);
        break;
    case GLUT_KEY_PAGE_UP:
        printf("Page up is pressed at (%d, %d)\n", x, y);
        break;
    case GLUT_KEY_LEFT:
        printf("Left arrow is pressed at (%d, %d)\n", x, y);
        break;
    default:
        printf("Other special key is pressed at (%d, %d)\n", x, y);
        break;
    }
}

void My_Menu(int id)
{
    switch(id)
    {
    case MENU_TIMER_START:
        if(!timer_enabled)
        {
            timer_enabled = true;
            glutTimerFunc(timer_speed, My_Timer, 0);
        }
        break;
    case MENU_TIMER_STOP:
        timer_enabled = false;
        break;
    case MENU_EXIT:
        exit(0);
        break;
    case ZW:
        walking =1;
        fury=0;
        break;
    case ZF:
        fury =1;
        walking=0;
        break;
    default:
        break;
    }
}

int main(int argc, char *argv[])
{
    // Initialize GLUT and GLEW, then create a window.
    ////////////////////
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(600, 600);
    glutCreateWindow("Assignment 02 103062137"); // You cannot use OpenGL functions before this line; The OpenGL context must be created first by glutCreateWindow()!
    glewInit();
    ilInit();
    ilEnable(IL_ORIGIN_SET);
    ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
    dumpInfo();
    My_Init();
    My_LoadModels();
    My_Loadzombie();
    My_Loadzombie2();
    ////////////////////

    // Create a menu and bind it to mouse right button.
    ////////////////////////////
    int menu_main = glutCreateMenu(My_Menu);
    int menu_timer = glutCreateMenu(My_Menu);
    int zombie = glutCreateMenu(My_Menu);

    glutSetMenu(menu_main);
    glutAddSubMenu("Timer", menu_timer);
    glutAddSubMenu("Zombie mode",zombie);
    glutAddMenuEntry("Exit", MENU_EXIT);

    glutSetMenu(menu_timer);
    glutAddMenuEntry("Start", MENU_TIMER_START);
    glutAddMenuEntry("Stop", MENU_TIMER_STOP);

    glutSetMenu(zombie);
    glutAddMenuEntry("Walking", ZW);
    glutAddMenuEntry("Fury",ZF);

    glutSetMenu(menu_main);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    ////////////////////////////

    // Register GLUT callback functions.
    ///////////////////////////////
    glutDisplayFunc(My_Display);

    glutReshapeFunc(My_Reshape);
    glutMouseFunc(My_Mouse);
    glutMotionFunc(mouseMove);
    glutKeyboardFunc(My_Keyboard);
    glutSpecialFunc(My_SpecialKeys);
    glutTimerFunc(timer_speed, My_Timer, 0);
    ///////////////////////////////

    // Enter main event loop.
    //////////////
    glutMainLoop();
    //////////////
    return 0;
}
