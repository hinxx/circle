// dear imgui: Platform Backend for Circle
// This needs to be used along with a Renderer (e.g. OpenGL3)
// (Info: Circle is a C++ bare metal programming environment for the Raspberry Pi. The VideoCoreIV found in the Raspberry Pi 1, 2 and 3 supports OpenGL ES 2.0 and OpenVG 1.1.)
// (Requires: Circle release Step43.1)

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard arrays indexed using SDL_SCANCODE_* codes, e.g. ImGui::IsKeyPressed(SDL_SCANCODE_SPACE).
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
// Missing features:
//  [ ] Platform: ???

// You can copy and use unmodified imgui_impl_* files in your project. See main.cpp for an example of using this.
// If you are new to dear imgui, read the documentation at the top of imgui.cpp and at https://github.com/ocornut/imgui.

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2020-10-25: Initial implementation.

#include "imgui.h"
#include "imgui_impl_circle.h"

// VideoCore IV
#include "bcm_host.h"

#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"

#include <linux/spinlock.h>
#include <linux/completion.h>
#include <circle/string.h>
#include <circle/input/mousebehaviour.h>
#include <circle/input/mouse.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/devicenameservice.h>

// Data
//static SDL_Window*  g_Window = NULL;
//static Uint64       g_Time = 0;
//static bool         g_MousePressed[3] = { false, false, false };
//static SDL_Cursor*  g_MouseCursors[ImGuiMouseCursor_COUNT] = {};
//static char*        g_ClipboardTextData = NULL;

typedef struct
{
   uint32_t screen_width;
   uint32_t screen_height;
// OpenGL|ES objects
   DISPMANX_ELEMENT_HANDLE_T dispman_element;
   DISPMANX_DISPLAY_HANDLE_T dispman_display;
   EGLDisplay display;
   EGLSurface surface;
   EGLContext context;

//   GLuint verbose;
//   GLuint vshader;
//   GLuint fshader;
//   GLuint mshader;
//   GLuint program;
//   GLuint program2;
//   GLuint tex_fb;
//   GLuint tex;
//   GLuint buf;
// julia attribs
//   GLuint unif_color, attr_vertex, unif_scale, unif_offset, unif_tex, unif_centre;
// mandelbrot attribs
//   GLuint attr_vertex2, unif_scale2, unif_offset2, unif_centre2;

//   bool terminate;
} CUBE_STATE_T;

static CUBE_STATE_T _state, *state=&_state;

static struct mutex vsync_cond_mutex;
static struct completion vsync_cond;

static DEFINE_SPINLOCK (mouse_lock);
//static int mouse_buttons = 0, mouse_dx = 0, mouse_dy = 0;
static unsigned mouse_x = 0;
static unsigned mouse_y = 0;
static bool g_MousePressed[3] = { false, false, false };
static int mouse_wheel = 0;

static DEFINE_SPINLOCK (keyboard_lock);
static unsigned char g_cookedKeys[2] = {0};
static unsigned char g_rawKeys[6] = {0};
static unsigned char g_modifierKeys = 0;
static unsigned char g_rawKeysLast[6] = {0};

static CString g_ClipboardTextData;

static bool g_terminate = false;

#define check() assert(glGetError() == 0)

void mouse_event_callback(TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY, int nWheelMove)
{
//    printk("mouse %4d %4d buttons %d\n", nPosX, nPosY, nButtons);

    spin_lock (&mouse_lock);
    switch (Event)
	{
	case MouseEventMouseMove:
		mouse_x = nPosX;
		mouse_y = nPosY;
		break;

    case MouseEventMouseDown:
        if (nButtons & MOUSE_BUTTON_LEFT)
            g_MousePressed[0] = true;
        if (nButtons & MOUSE_BUTTON_RIGHT)
            g_MousePressed[1] = true;
        if (nButtons & MOUSE_BUTTON_MIDDLE)
            g_MousePressed[2] = true;
        break;

    case MouseEventMouseUp:
        if (nButtons & MOUSE_BUTTON_LEFT)
            g_MousePressed[0] = false;
        if (nButtons & MOUSE_BUTTON_RIGHT)
            g_MousePressed[1] = false;
        if (nButtons & MOUSE_BUTTON_MIDDLE)
            g_MousePressed[2] = false;
        break;

    case MouseEventMouseWheel:
        if (nWheelMove > 0) mouse_wheel = 1;
        if (nWheelMove < 0) mouse_wheel = -1;
        break;

	default:
		break;
	}
    spin_unlock (&mouse_lock);
}

void KeyPressedHandler (const char *pString) {
//    assert (s_pThis != 0);
//	s_pThis->m_Screen.Write (pString, strlen (pString));

    // expect a single character long string, NUL terminated!
    // see CKeyMap::GetString()
    assert(g_cookedKeys[1] == 0);

    spin_lock (&keyboard_lock);
    g_cookedKeys[0] = *pString;
    spin_unlock (&keyboard_lock);
#if DEBUG
    CString Message;
	Message.Format ("Text input %s", pString);
    printk(Message);
#endif
}

void KeyStatusHandlerRaw (unsigned char ucModifiers, const unsigned char RawKeys[6]) {
//    assert (s_pThis != 0);

    spin_lock (&keyboard_lock);
    g_modifierKeys = ucModifiers;
    for (unsigned i = 0; i < 6; i++)
	{
        g_rawKeys[i] = RawKeys[i];
    }
    spin_unlock (&keyboard_lock);

#if DEBUG
	CString Message;
	Message.Format ("Key status (modifiers %02X)", (unsigned) ucModifiers);

	for (unsigned i = 0; i < 6; i++)
	{
		if (RawKeys[i] != 0)
		{
			CString KeyCode;
			KeyCode.Format (" %02X", (unsigned) RawKeys[i]);

			Message.Append (KeyCode);
		}
	}
	printk(Message);
#endif
}

void ShutdownHandler (void) {
//    assert (s_pThis != 0);
//	s_pThis->m_ShutdownMode = ShutdownReboot;
    g_terminate = true;
}

static const char* ImGui_ImplCircle_GetClipboardText(void*)
{
    const char *text = g_ClipboardTextData;
    if (!text) {
        text = "";
    }
    //printk("GET clipboard content: '%s'", (const char *)text, strlen(text));
    return text;
}

static void ImGui_ImplCircle_SetClipboardText(void*, const char* text)
{
    if (!text) {
        text = "";
    }
    g_ClipboardTextData = text;
    //printk("SET clipboard content: '%s'", (const char *)g_clipboard, g_clipboard.GetLength());
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
// If you have multiple SDL events and some of them are not meant to be used by dear imgui, you may need to filter events based on their windowID field.
bool ImGui_ImplCircle_ProcessEvent(/*const SDL_Event* event*/)
{
    ImGuiIO& io = ImGui::GetIO();
//    switch (event->type)
//    {
//    case SDL_MOUSEWHEEL:
//        {
//            if (event->wheel.x > 0) io.MouseWheelH += 1;
//            if (event->wheel.x < 0) io.MouseWheelH -= 1;
//            if (event->wheel.y > 0) io.MouseWheel += 1;
//            if (event->wheel.y < 0) io.MouseWheel -= 1;
//            return true;
//        }
//    case SDL_MOUSEBUTTONDOWN:
//        {
//            if (event->button.button == SDL_BUTTON_LEFT) g_MousePressed[0] = true;
//            if (event->button.button == SDL_BUTTON_RIGHT) g_MousePressed[1] = true;
//            if (event->button.button == SDL_BUTTON_MIDDLE) g_MousePressed[2] = true;
//            return true;
//        }
//    case SDL_TEXTINPUT:
//        {
//            io.AddInputCharactersUTF8(event->text.text);
//            return true;
//        }
//    case SDL_KEYDOWN:
//    case SDL_KEYUP:
//        {
//            int key = event->key.keysym.scancode;
//            IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
//            io.KeysDown[key] = (event->type == SDL_KEYDOWN);
//            io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
//            io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
//            io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
//#ifdef _WIN32
//            io.KeySuper = false;
//#else
//            io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);
//#endif
//            return true;
//        }
//    }
    // update mouse position and button states
   spin_lock(&mouse_lock);

   unsigned x, y;
   x = mouse_x;
   y = mouse_y;
   io.MouseDown[0] = g_MousePressed[0];
   io.MouseDown[1] = g_MousePressed[1];
   io.MouseDown[2] = g_MousePressed[2];
   io.MouseWheel += mouse_wheel;
   io.MousePos = ImVec2((float)x, (float)y);

   spin_unlock(&mouse_lock);

   // update keyboard key presses/releases
   spin_lock(&keyboard_lock);

   // signal key release
   // if any of the keys are stil pressed in this frame, they will be set in the key press loop below
   for (unsigned i = 0; i < 6; i++) {
       int key = g_rawKeysLast[i];
       if (key == KeyNone) {
           continue;
       }
       io.KeysDown[key] = false;
       g_rawKeysLast[i] = KeyNone;
    }

   // signal key press
   for (unsigned i = 0; i < 6; i++) {
       int key = g_rawKeys[i];
       if (key == KeyNone) {
           continue;
       }
       IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));
       io.KeysDown[key] = true;
       g_rawKeysLast[i] = key;
   }

   // set key modifiers
   io.KeyShift = ((g_modifierKeys & KEY_LSHIFT_MASK) != 0) || (g_modifierKeys & KEY_RSHIFT_MASK) != 0;
   io.KeyCtrl = ((g_modifierKeys & KEY_LCTRL_MASK) != 0) || ((g_modifierKeys & KEY_RCTRL_MASK) != 0);
   io.KeyAlt = ((g_modifierKeys & KEY_ALT_MASK) != 0) || ((g_modifierKeys & KEY_ALTGR_MASK) != 0);
   io.KeySuper = ((g_modifierKeys & KEY_LWIN_MASK) != 0) || ((g_modifierKeys & KEY_RWIN_MASK) != 0);

   // set input text if any
   if (g_cookedKeys[0] != KeyNone) {
       io.AddInputCharacter(g_cookedKeys[0]);
       g_cookedKeys[0] = KeyNone;
   }

   spin_unlock(&keyboard_lock);

    return false;
}

bool ImGui_ImplCircle_Init(/*SDL_Window* window, */void* sdl_gl_context)
{

    // Setup backend capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    //io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;       // We can honor GetMouseCursor() values (optional)
    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;        // We can honor io.WantSetMousePos requests (optional, rarely used)
    io.BackendPlatformName = "imgui_impl_circle";

    // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
    io.KeyMap[ImGuiKey_Tab] = 0x2B;
    io.KeyMap[ImGuiKey_LeftArrow] = 0x50;
    io.KeyMap[ImGuiKey_RightArrow] = 0x4F;
    io.KeyMap[ImGuiKey_UpArrow] = 0x52;
    io.KeyMap[ImGuiKey_DownArrow] = 0x51;
    io.KeyMap[ImGuiKey_PageUp] = 0x4B;
    io.KeyMap[ImGuiKey_PageDown] = 0x4E;
    io.KeyMap[ImGuiKey_Home] = 0x4A;
    io.KeyMap[ImGuiKey_End] = 0x4D;
    io.KeyMap[ImGuiKey_Insert] = 0x49;
    io.KeyMap[ImGuiKey_Delete] = 0x4C;
    io.KeyMap[ImGuiKey_Backspace] = 0x2A;
    io.KeyMap[ImGuiKey_Space] = 0x2C;
    io.KeyMap[ImGuiKey_Enter] = 0x28;
    io.KeyMap[ImGuiKey_Escape] = 0x28;
    io.KeyMap[ImGuiKey_KeyPadEnter] = 0x58;
    io.KeyMap[ImGuiKey_A] = 0x04;
    io.KeyMap[ImGuiKey_C] = 0x06;
    io.KeyMap[ImGuiKey_V] = 0x19;
    io.KeyMap[ImGuiKey_X] = 0x1B;
    io.KeyMap[ImGuiKey_Y] = 0x1C;
    io.KeyMap[ImGuiKey_Z] = 0x1D;

    io.SetClipboardTextFn = ImGui_ImplCircle_SetClipboardText;
    io.GetClipboardTextFn = ImGui_ImplCircle_GetClipboardText;
    io.ClipboardUserData = NULL;

    // draw imgui cursor (do not have to use circle pMouse->ShowCursor(TRUE);)
    io.MouseDrawCursor = true;


    CMouseDevice *pMouse = (CMouseDevice *) CDeviceNameService::Get()->GetDevice("mouse1", FALSE);
    assert(pMouse != NULL);
    if (! pMouse->Setup(state->screen_width, state->screen_height)) {
        printk("Cannot setup mouse\n");
    }
    pMouse->SetCursor(state->screen_width/2, state->screen_height/2);
    // XXX: consider using ImGui cursor instead of native one!
    //   pMouse->ShowCursor(TRUE);
    pMouse->RegisterEventHandler(mouse_event_callback);

    CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get()->GetDevice ("ukbd1", FALSE);
    assert(pKeyboard != NULL);
    pKeyboard->RegisterShutdownHandler(ShutdownHandler);
    // change CUSBKeyboardDevice::ReportHandler() to not return if m_pKeyStatusHandlerRaw
    // is registered to have both KeyPressedHandler and KeyStatusHandlerRaw executed!
    // one can deliver keys to UI and another to serial console
    pKeyboard->RegisterKeyPressedHandler(KeyPressedHandler);
    pKeyboard->RegisterKeyStatusHandlerRaw(KeyStatusHandlerRaw);

    return true;
}

//bool ImGui_ImplSDL2_InitForOpenGL(SDL_Window* window, void* sdl_gl_context)
//{
//    (void)sdl_gl_context; // Viewport branch will need this.
//    return ImGui_ImplSDL2_Init(window);
//}

//bool ImGui_ImplSDL2_InitForVulkan(SDL_Window* window)
//{
//#if !SDL_HAS_VULKAN
//    IM_ASSERT(0 && "Unsupported");
//#endif
//    return ImGui_ImplSDL2_Init(window);
//}

//bool ImGui_ImplSDL2_InitForD3D(SDL_Window* window)
//{
//#if !defined(_WIN32)
//    IM_ASSERT(0 && "Unsupported");
//#endif
//    return ImGui_ImplSDL2_Init(window);
//}

//bool ImGui_ImplSDL2_InitForMetal(SDL_Window* window)
//{
//    return ImGui_ImplSDL2_Init(window);
//}

void ImGui_ImplCircle_Shutdown()
{
//    g_Window = NULL;

    // Destroy last known clipboard data
//    if (g_ClipboardTextData)
//        SDL_free(g_ClipboardTextData);
    g_ClipboardTextData = NULL;
}

//static void ImGui_ImplSDL2_UpdateMousePosAndButtons()
//{
//    ImGuiIO& io = ImGui::GetIO();

//    // Set OS mouse position if requested (rarely used, only when ImGuiConfigFlags_NavEnableSetMousePos is enabled by user)
//    if (io.WantSetMousePos)
//        SDL_WarpMouseInWindow(g_Window, (int)io.MousePos.x, (int)io.MousePos.y);
//    else
//        io.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);

//    int mx, my;
//    Uint32 mouse_buttons = SDL_GetMouseState(&mx, &my);
//    io.MouseDown[0] = g_MousePressed[0] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_LEFT)) != 0;  // If a mouse press event came, always pass it as "mouse held this frame", so we don't miss click-release events that are shorter than 1 frame.
//    io.MouseDown[1] = g_MousePressed[1] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_RIGHT)) != 0;
//    io.MouseDown[2] = g_MousePressed[2] || (mouse_buttons & SDL_BUTTON(SDL_BUTTON_MIDDLE)) != 0;
//    g_MousePressed[0] = g_MousePressed[1] = g_MousePressed[2] = false;

//    if (SDL_GetWindowFlags(g_Window) & SDL_WINDOW_INPUT_FOCUS)
//        io.MousePos = ImVec2((float)mx, (float)my);
//}

void ImGui_ImplCircle_NewFrame(/*SDL_Window* window*/)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

//    // Setup display size (every frame to accommodate for window resizing)
//    int w, h;
//    int display_w, display_h;
//    SDL_GetWindowSize(window, &w, &h);
//    if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
//        w = h = 0;
//    SDL_GL_GetDrawableSize(window, &display_w, &display_h);
//    io.DisplaySize = ImVec2((float)w, (float)h);
//    if (w > 0 && h > 0)
//        io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

//    // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
//    static Uint64 frequency = SDL_GetPerformanceFrequency();
//    Uint64 current_time = SDL_GetPerformanceCounter();
//    io.DeltaTime = g_Time > 0 ? (float)((double)(current_time - g_Time) / frequency) : (float)(1.0f / 60.0f);
//    g_Time = current_time;

    //ImGui_ImplSDL2_UpdateMousePosAndButtons();
    //ImGui_ImplSDL2_UpdateMouseCursor();

    // Update game controllers (if enabled and available)
    //ImGui_ImplSDL2_UpdateGamepads();



    CMouseDevice *pMouse = (CMouseDevice *) CDeviceNameService::Get()->GetDevice("mouse1", FALSE);
    assert(pMouse != NULL);
    pMouse->UpdateCursor();
    mouse_wheel = 0;

    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    // Setup display size (every frame to accommodate for window resizing)
    int w, h;
    int display_w, display_h;
 //   SDL_GetWindowSize(window, &w, &h);
 //   if (SDL_GetWindowFlags(window) & SDL_WINDOW_MINIMIZED)
 //       w = h = 0;
 //   SDL_GL_GetDrawableSize(window, &display_w, &display_h);
 //   io.DisplaySize = ImVec2((float)w, (float)h);
 //   if (w > 0 && h > 0)
 //       io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);
    w = state->screen_width;
    h = state->screen_height;
    display_w = state->screen_width;
    display_h = state->screen_height;
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // Setup time step (we don't use SDL_GetTicks() because it is using millisecond resolution)
 //   static Uint64 frequency = SDL_GetPerformanceFrequency();
 //   Uint64 current_time = SDL_GetPerformanceCounter();
 //   io.DeltaTime = g_Time > 0 ? (float)((double)(current_time - g_Time) / frequency) : (float)(1.0f / 60.0f);
 //   g_Time = current_time;
    io.DeltaTime = (float)(1.0f / 60.0f);

}


void printConfigInfo(int n, EGLDisplay display, EGLConfig *config) {
    EGLint size;

//    printk("Configuration %d is\n", n);

    eglGetConfigAttrib(display, *config, EGL_RED_SIZE, &size);
    EGLint red = size;
//    printk("  Red size is %d\n", size);
    eglGetConfigAttrib(display, *config, EGL_BLUE_SIZE, &size);
    EGLint blue = size;
//    printk("  Blue size is %d\n", size);
    eglGetConfigAttrib(display, *config, EGL_GREEN_SIZE, &size);
    EGLint green = size;
//    printk("  Green size is %d\n", size);
    eglGetConfigAttrib(display, *config, EGL_ALPHA_SIZE, &size);
    EGLint alpha = size;
//    printk("  Alpha size is %d\n", size);
    eglGetConfigAttrib(display, *config, EGL_BUFFER_SIZE, &size);
//    printk("  Buffer size is %d\n", size);

    EGLint rgb;
   eglGetConfigAttrib(display, *config,  EGL_BIND_TO_TEXTURE_RGB , &rgb);
//   if (rgb == EGL_TRUE)
//       printk("  Can be bound to RGB texture\n");
//   else
//       printk("  Can't be bound to RGB texture\n");

   EGLint rgba;
   eglGetConfigAttrib(display, *config,  EGL_BIND_TO_TEXTURE_RGBA , &rgba);
//   if (rgba == EGL_TRUE)
//       printk("  Can be bound to RGBA texture\n");
//   else
//       printk("  Can't be bound to RGBA texture\n");
   printk("[%02d] R %d G %d B %d A %d S %d %s %s\n",
          n, red, green, blue, alpha, size,
          (rgb == EGL_TRUE)?"Y":"N", (rgba == EGL_TRUE)?"Y":"N");
}


static void vsync_callback(DISPMANX_UPDATE_HANDLE_T u, void *data) {
//   SDL_WindowData *wdata = ((SDL_WindowData *) data);
//   SDL_LockMutex(wdata->vsync_cond_mutex);
//   SDL_CondSignal(wdata->vsync_cond);
//   SDL_UnlockMutex(wdata->vsync_cond_mutex);

    (void)u;
    (void)data;
//    printk("vsync callback called..\n");
    mutex_lock(&vsync_cond_mutex);
    complete(&vsync_cond);
    mutex_unlock(&vsync_cond_mutex);
}

/***********************************************************
 * Name: init_ogl
 *
 * Arguments:
 *       CUBE_STATE_T *state - holds OGLES model info
 *
 * Description: Sets the display, OpenGL|ES context and screen stuff
 *
 * Returns: void
 *
 ***********************************************************/
void CircleInit(/*CUBE_STATE_T *state*/)
{

    //g_Window = window;
    bcm_host_init();

    // Clear application state
    memset( state, 0, sizeof( *state ) );

    // Start OGLES
//    init_ogl(state);
 //   init_shaders(state);
 //   cx = state->screen_width/2;
 //   cy = state->screen_height/2;
    printk("vc4 screen %d x %d\n", state->screen_width, state->screen_height);

   int32_t success = 0;
   EGLBoolean result;
   EGLint num_config;

   static EGL_DISPMANX_WINDOW_T nativewindow;

   DISPMANX_UPDATE_HANDLE_T dispman_update;
   VC_RECT_T dst_rect;
   VC_RECT_T src_rect;

   static const EGLint attribute_list[] =
   {
      EGL_RED_SIZE, 8,
      EGL_GREEN_SIZE, 8,
      EGL_BLUE_SIZE, 8,
      EGL_ALPHA_SIZE, 8,
//      EGL_ALPHA_SIZE, 0,
//      EGL_DEPTH_SIZE, 24,
//      EGL_COLOR_BUFFER_TYPE, EGL_RGB_BUFFER,
      EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
      EGL_NONE
   };

   static const EGLint context_attributes[] =
   {
      EGL_CONTEXT_CLIENT_VERSION, 2,
      EGL_NONE
   };
   EGLConfig config;

   // get an EGL display connection
   state->display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
   assert(state->display!=EGL_NO_DISPLAY);
   check();

   // initialize the EGL display connection
   result = eglInitialize(state->display, NULL, NULL);
   assert(EGL_FALSE != result);
   check();

   EGLint num_configs;
   EGLConfig *configs;

   eglGetConfigs(state->display, NULL, 0, &num_configs);
   printk("EGL has %d configs\n", num_configs);

   configs = (EGLConfig *)calloc(num_configs, sizeof *configs);
   eglGetConfigs(state->display, configs, num_configs, &num_configs);

   //CScheduler *sched = CScheduler::Get();
   //int i;
   //for (i = 0; i < num_configs; i++) {
   //     printConfigInfo(i, state->display, &configs[i]);
   //}
   //sched->Sleep(5);

   // get an appropriate EGL frame buffer configuration
   result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
   assert(EGL_FALSE != result);
   check();
   printk("\nselected config:\n");
   printConfigInfo(1, state->display, &config);
   //sched->Sleep(5);

   // get an appropriate EGL frame buffer configuration
   result = eglBindAPI(EGL_OPENGL_ES_API);
   assert(EGL_FALSE != result);
   check();

   // create an EGL rendering context
   state->context = eglCreateContext(state->display, config, EGL_NO_CONTEXT, context_attributes);
   assert(state->context!=EGL_NO_CONTEXT);
   check();

   // create an EGL window surface
   success = graphics_get_display_size(0 /* LCD */, &state->screen_width, &state->screen_height);
   assert( success >= 0 );

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = state->screen_width;
   dst_rect.height = state->screen_height;

   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = state->screen_width << 16;
   src_rect.height = state->screen_height << 16;

   state->dispman_display = vc_dispmanx_display_open( 0 /* LCD */);
   dispman_update = vc_dispmanx_update_start( 0 );

   // from SDL_rpivideo.c
   VC_DISPMANX_ALPHA_T dispman_alpha;
   /* Disable alpha, otherwise the app looks composed with whatever dispman is showing (X11, console,etc) */
   dispman_alpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
   dispman_alpha.opacity = 0xFF;
   dispman_alpha.mask = 0;

   state->dispman_element = vc_dispmanx_element_add ( dispman_update, state->dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE, &dispman_alpha /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE/*transform*/);

   nativewindow.element = state->dispman_element;
   nativewindow.width = state->screen_width;
   nativewindow.height = state->screen_height;
   vc_dispmanx_update_submit_sync( dispman_update );

   check();

   state->surface = eglCreateWindowSurface( state->display, config, &nativewindow, NULL );
   assert(state->surface != EGL_NO_SURFACE);
   check();

   // connect the context to the surface
   result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
   assert(EGL_FALSE != result);
   check();

   // Set background color and clear buffers
   glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
   glClear( GL_COLOR_BUFFER_BIT );
   check();

   // start generating vsync callbacks
   init_completion(&vsync_cond);
   mutex_init(&vsync_cond_mutex);
   vc_dispmanx_vsync_callback(state->dispman_display, vsync_callback, NULL/*(void*)wdata*/);

    // enable vsync
    result = eglSwapInterval(state->display, 1);
    assert(EGL_FALSE != result);
    check();
}


void CircleExit(void)
{
   DISPMANX_UPDATE_HANDLE_T dispman_update;
   int s;
   // clear screen
   glClear( GL_COLOR_BUFFER_BIT );
   eglSwapBuffers(state->display, state->surface);

   mutex_lock(&vsync_cond_mutex);
   wait_for_completion(&vsync_cond);
   mutex_unlock(&vsync_cond_mutex);
   vc_dispmanx_vsync_callback(state->dispman_display, NULL, NULL);

   eglDestroySurface( state->display, state->surface );

   dispman_update = vc_dispmanx_update_start( 0 );
   s = vc_dispmanx_element_remove(dispman_update, state->dispman_element);
   assert(s == 0);
   vc_dispmanx_update_submit_sync( dispman_update );
   s = vc_dispmanx_display_close(state->dispman_display);
   assert (s == 0);

   // Release OpenGL resources
   eglMakeCurrent( state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
   eglDestroyContext( state->display, state->context );
   eglTerminate( state->display );

   printk("Image closed\n");
}

void CircleSwapBuffers(void)
{

    //   glFlush();
    //   glFinish();

       //SDL_GL_SwapWindow(window);
       eglSwapBuffers(state->display, state->surface);
       //check();

       // wait for vsync
       mutex_lock(&vsync_cond_mutex);
       wait_for_completion(&vsync_cond);
       mutex_unlock(&vsync_cond_mutex);
}

bool CircleTerminate(void)
{
    return g_terminate;
}
