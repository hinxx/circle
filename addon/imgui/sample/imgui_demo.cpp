
#ifdef __circle__
#include <linux/kernel.h>
#include <linux/delay.h>
#include <linux/spinlock.h>
#include <linux/mutex.h>
#include <linux/completion.h>
#include <circle/sched/scheduler.h>
#include <circle/input/mousebehaviour.h>
#include <circle/input/mouse.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/devicenameservice.h>
#include <assert.h>
#else
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include <unistd.h>
#endif

#include "bcm_host.h"

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_circle.h"
#include "imgui/imgui_impl_opengl3.h"

int _main ()
{
    CircleInit();

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();

   ImGuiIO& io = ImGui::GetIO(); (void)io;

   // Setup Dear ImGui style
   ImGui::StyleColorsDark();
   //ImGui::StyleColorsClassic();

   ImGui_ImplCircle_Init();
   const char* glsl_version = "#version 100";
   ImGui_ImplOpenGL3_Init(glsl_version);

   bool show_demo_window = true;
   bool show_another_window = false;
   ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

   int iter = 0;
   bool terminate = false;
   while (! terminate)
   {
       // Process the user input
        ImGui_ImplCircle_ProcessEvent();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplCircle_NewFrame();
        ImGui::NewFrame();

        if (show_demo_window)
        {
            ImGui::ShowDemoWindow(&show_demo_window);
        }

        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);
            ImGui::Text("loop iter = %d", iter++);
            ImGui::Text("mouse X %4d Y %4d", (int)io.MousePos.x, (int)io.MousePos.y);
            ImGui::Text("mouse buttons = LEFT %d RIGHT %d MIDDLE %d SIDE1 %d SIDE2 %d",
                        io.MouseDown[0], io.MouseDown[1], io.MouseDown[2], io.MouseDown[3], io.MouseDown[4]);
            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

            // Terminate on mouse click ..
            if (ImGui::Button("Terminate"))
                terminate = true;
            // .. or on Ctrl+Alt+Del
            if (CircleTerminate())
                terminate = true;

            ImGui::End();
        }

        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        CircleSwapBuffers();
   }

   // Cleanup
   ImGui_ImplOpenGL3_Shutdown();
   printk("after ImGui_ImplOpenGL3_Shutdown\n");
   ImGui_ImplCircle_Shutdown();

   ImGui::DestroyContext();

   CircleExit();

   return 0;
}
