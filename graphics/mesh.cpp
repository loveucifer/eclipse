#include "mesh.h"
#include <cstdint>
#include <glad/glad.h>
#include "helpers.h"

namespace eclipse::graphics{
  mesh::mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions)
  :mElementCount(0),mVertexCount(vertexCount),mEbo(0)

// vertexArray - is a raw pointer to our data sitting in normal CPU RAM
// vertexCount - is the no of vertices we have
// dimensions - is like how many floats make up one vertext , 2 for x/y , 3 for x/y/z etc

  
  {

    // MAKING THE VAO
    // vao stands for vertex array object ( refer your notes ) its basically an object
    // that stores fixed set of state values
    
    // lifetime of this mesh will define our data so it cleans up after itself
    // generate vao and store it
    glGenVertexArrays(1,&mVao); ECLIPSE_CHECK_GL_ERROR;
    // bind the pulled vertex array
    glBindVertexArray(mVao); ECLIPSE_CHECK_GL_ERROR;

    /*

        0.0f , 0.0f
        
        1.0f , 0.0f
         
        1.0f , 1.0f

        // if we decide to pass in this data as a pointer , it will just be the size of the pointer
        // so instead to calulate the size ,
        //  vertexCount * dimensions * sizeof(float) 

    */



    glGenBuffers(1,&mPositionVbo);ECLIPSE_CHECK_GL_ERROR;
    glBindBuffer(GL_ARRAY_BUFFER,mPositionVbo); ECLIPSE_CHECK_GL_ERROR;
    glBufferData(GL_ARRAY_BUFFER, vertexCount * dimensions * sizeof(float),vertexArray, GL_STATIC_DRAW);ECLIPSE_CHECK_GL_ERROR;

    glEnableVertexAttribArray(0);ECLIPSE_CHECK_GL_ERROR;

    glVertexAttribPointer(0,dimensions,GL_FLOAT,GL_FALSE,0,0);ECLIPSE_CHECK_GL_ERROR;
    // index , size which is our dimensions , what type it is , is it normialized or not , stride and pointer


    glDisableVertexAttribArray(0);ECLIPSE_CHECK_GL_ERROR;


// disable
    glBindBuffer(GL_ARRAY_BUFFER,0);ECLIPSE_CHECK_GL_ERROR;
   // this is how we unbind 
    glBindVertexArray(0);ECLIPSE_CHECK_GL_ERROR;
  }


mesh::mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions,uint32_t* elementArray , uint32_t elementCount)
:mesh(vertexArray,vertexCount,dimensions)
  {

    mElementCount = elementCount;
       
    glBindVertexArray(mVao); ECLIPSE_CHECK_GL_ERROR;
    glGenBuffers(1,&mEbo);ECLIPSE_CHECK_GL_ERROR;
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,mEbo); ECLIPSE_CHECK_GL_ERROR;
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, elementCount * sizeof(uint32_t),elementArray, GL_STATIC_DRAW);ECLIPSE_CHECK_GL_ERROR;
    // glEnableVertexAttribArray(0);ECLIPSE_CHECK_GL_ERROR;
    // glVertexAttribPointer(0,dimensions,GL_FLOAT,GL_FALSE,0,0);ECLIPSE_CHECK_GL_ERROR;

    // glDisableVertexAttribArray(0);ECLIPSE_CHECK_GL_ERROR;
    // glBindBuffer(GL_ARRAY_BUFFER,0);ECLIPSE_CHECK_GL_ERROR;
    glBindVertexArray(0);ECLIPSE_CHECK_GL_ERROR;
  }

// ebo needs to stay so we cant unbind ebo specifically we can just unbind the vao after we are done
// because its one ebo per vao 

    void mesh::Bind(){
    glBindVertexArray(mVao);ECLIPSE_CHECK_GL_ERROR;
    glEnableVertexAttribArray(0);ECLIPSE_CHECK_GL_ERROR;
  }

  void mesh::UnBind(){
    glDisableVertexAttribArray(0);ECLIPSE_CHECK_GL_ERROR;
    glBindVertexArray(0);ECLIPSE_CHECK_GL_ERROR;
  }

  mesh::~mesh(){
    glDeleteBuffers(1,&mPositionVbo);ECLIPSE_CHECK_GL_ERROR;
    if(mEbo != 0){
      glDeleteBuffers(1,&mEbo);ECLIPSE_CHECK_GL_ERROR;
    }
    glDeleteVertexArrays(1,&mVao);ECLIPSE_CHECK_GL_ERROR;
  }


mesh::mesh(float* vertexArray, uint32_t vertexCount, uint32_t dimensions,float* texcoords, uint32_t* elementArray , uint32_t elementCount)
:mesh(vertexArray,vertexCount,dimensions,elementArray,elementCount)
  {
   // glGenVertexArrays(1,&mVao);
    glBindVertexArray(mVao); ECLIPSE_CHECK_GL_ERROR;
    glGenBuffers(1,&mTexCordsVbo);ECLIPSE_CHECK_GL_ERROR;
    glBindBuffer(GL_ARRAY_BUFFER,mTexCordsVbo); ECLIPSE_CHECK_GL_ERROR;
    glBufferData(GL_ARRAY_BUFFER, vertexCount* 2 * /* tex cords are uv so 2 */ sizeof(float ),texcoords, GL_STATIC_DRAW);ECLIPSE_CHECK_GL_ERROR;
    glEnableVertexAttribArray(1);ECLIPSE_CHECK_GL_ERROR;
    glVertexAttribPointer(1,2,GL_FLOAT,GL_FALSE,0,0);ECLIPSE_CHECK_GL_ERROR;
    // dimensions is 2 same as we setup in the size of buffer data

    glDisableVertexAttribArray(0);ECLIPSE_CHECK_GL_ERROR;
    glBindBuffer(GL_ARRAY_BUFFER,0);ECLIPSE_CHECK_GL_ERROR;
    glBindVertexArray(0);ECLIPSE_CHECK_GL_ERROR;
  }
}
