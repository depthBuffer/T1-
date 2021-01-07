#ifndef T1_H
#define T1_H

#include "shading/shader.h"
#include "shading/texture.h"
#include "shading/texture_cube.h"

#include "glad/glad.h"

namespace T1
{
    /* 

      Global entry-point for T1's initialization. Starts all T1's relevant initialization 
      calls, and makes sure T1 is ready to operate after calling its initialization function. 
      Note that all GL function pointers are initialized here as well (including any extensions) 
      w/ GLAD.

    */
    void Init(GLADloadproc loadProcFunc);


    // cleans up T1 of all resources that are no longer relevant.
    void Clean();
}

#endif