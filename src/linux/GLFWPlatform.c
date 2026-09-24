/*
    Copyright Jim Marshall 2026.
    File: GLFWPlatform.c
    Description: Functions to handle windowing and input etc
*/
/////////////////////////////////////////////////////////////////////////////////////////// Standard Library Includes

/////////////////////////////////////////////////////////////////////////////////////////// SDK Includes

/////////////////////////////////////////////////////////////////////////////////////////// Third Party Includes

#include <glad/glad.h>
#include <GLFW/glfw3.h>

/////////////////////////////////////////////////////////////////////////////////////////// First Party Includes

#include "Platform.h"
#include "Log.h"

/////////////////////////////////////////////////////////////////////////////////////////// Typedefs

/////////////////////////////////////////////////////////////////////////////////////////// Defines

#define SCR_WIDTH 1024
#define SCR_HEIGHT 640

/////////////////////////////////////////////////////////////////////////////////////////// Enums

/////////////////////////////////////////////////////////////////////////////////////////// Structs

/////////////////////////////////////////////////////////////////////////////////////////// Private Globals

/////////////////////////////////////////////////////////////////////////////////////////// Public Globals

/////////////////////////////////////////////////////////////////////////////////////////// Private Functions

GLFWwindow* gWindow = NULL;

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    //Common_FramebufferSizeChangeHandler(width, height);
}

void MouseCallback(GLFWwindow* window, double xposIn, double yposIn)
{
    //In_RecieveMouseMove(GetInputContext(), xposIn, yposIn);
}

void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    //In_RecieveScroll(GetInputContext(), xoffset, yoffset);
}

void MouseBtnCallback(GLFWwindow* window, int button, int action, int mods)
{
    //In_RecieveMouseButton(GetInputContext(), button, action, mods);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    //In_RecieveKeyboardKey(GetInputContext(), key, scancode, action, mods);
}

void joystick_callback(int jid, int event)
{
    if (event == GLFW_CONNECTED)
    {
        //In_SetControllerPresent(jid);
    }
    else if (event == GLFW_DISCONNECTED)
    {
        //In_SetControllerPresent(-1);
    }
}


/////////////////////////////////////////////////////////////////////////////////////////// Public Functions

double Platform_GetTime()
{
    return glfwGetTime();
}

void Platform_PollInput()
{
    glfwPollEvents();
}

int Platform_Init()
{
         // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    Log_Verbose("glfwInit");
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    // glfw window creation
    // --------------------
    const char* windowTitle = "Stardew Engine";
    gWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, windowTitle, NULL, NULL);
    if (gWindow == NULL)
    {
        /*std::cout << "Failed to create GLFW window" << std::endl;*/
        Log_Error("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(gWindow);
    glfwSwapInterval(0); // Enable vsync
    
    glfwJoystickPresent(GLFW_JOYSTICK_1);

    glfwSetFramebufferSizeCallback(gWindow, FramebufferSizeCallback);
    glfwSetCursorPosCallback(gWindow, MouseCallback);
    glfwSetScrollCallback(gWindow, ScrollCallback);
    glfwSetMouseButtonCallback(gWindow, MouseBtnCallback);
    glfwSetKeyCallback(gWindow, key_callback);

    // tell GLFW to capture our mouse
    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    Log_Verbose("loading Opengl procs\n");
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        Log_Verbose("Failed to initialize GLAD");
        return -1;
    }
    return 0;
}

bool Platform_ShouldWindowClose()
{
    bool b = glfwWindowShouldClose(gWindow);
    return b;
}

void Platform_SwapBuffers()
{
    glfwSwapBuffers(gWindow);
}