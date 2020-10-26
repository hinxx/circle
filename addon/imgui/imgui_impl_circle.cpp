// dear imgui: Platform Backend for Circle
// This needs to be used along with a Renderer (e.g. OpenGL3)
// (Info: Circle is a C++ bare metal programming environment for the Raspberry Pi. The VideoCoreIV found in the Raspberry Pi 1, 2 and 3 supports OpenGL ES 2.0 and OpenVG 1.1.)
// (Requires: Circle release Step43.1)

// Implemented features:
//  [X] Platform: Mouse cursor shape and visibility. Disable with 'io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange'.
//  [X] Platform: Clipboard support.
//  [X] Platform: Keyboard arrays indexed using SDL_SCANCODE_* codes, e.g. ImGui::IsKeyPressed(SDL_SCANCODE_SPACE).
// Missing features:
//  [X] Platform: Gamepad support. Enabled with 'io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad'.
//  [ ] Platform: ???

// You can copy and use unmodified imgui_impl_* files in your project. See sample/demo.cpp for an example of using this.
// If you are new to dear imgui, read the documentation at the top of imgui.cpp and at https://github.com/ocornut/imgui.

// CHANGELOG
// (minor and older changes stripped away, please see git history for details)
//  2020-10-25: Initial implementation.

#include "imgui.h"
#include "imgui_impl_circle.h"

// VideoCore IV
#include "bcm_host.h"
// OGLES
#include "GLES2/gl2.h"
#include "EGL/egl.h"
#include "EGL/eglext.h"
// Circle
#include <linux/spinlock.h>
#include <linux/completion.h>
#include <circle/string.h>
#include <circle/input/mousebehaviour.h>
#include <circle/input/mouse.h>
#include <circle/usb/usbkeyboard.h>
#include <circle/devicenameservice.h>

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

} CUBE_STATE_T;

static CUBE_STATE_T _state, *state=&_state;

static unsigned g_Ticks = 0;
static bool g_terminate = false;
static struct mutex vsync_cond_mutex;
static struct completion vsync_cond;

static DEFINE_SPINLOCK (mouse_lock);
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

#define check() assert(glGetError() == 0)
//#define check() do { printk("%s:%d\n", __FUNCTION__, __LINE__); assert(glGetError() == 0); } while(0);

static void mouseEventHandler(TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY, int nWheelMove)
{
    spin_lock(&mouse_lock);

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
    spin_unlock(&mouse_lock);
}

static void keyPressedHandler(const char *pString)
{
    // expect a single character long string, NUL terminated!
    // see CKeyMap::GetString()
    assert(g_cookedKeys[1] == 0);

    spin_lock(&keyboard_lock);
    g_cookedKeys[0] = *pString;
    spin_unlock(&keyboard_lock);
}

static void keyStatusHandlerRaw(unsigned char ucModifiers, const unsigned char RawKeys[6])
{
    spin_lock(&keyboard_lock);
    g_modifierKeys = ucModifiers;
    for (unsigned i = 0; i < 6; i++)
	{
        g_rawKeys[i] = RawKeys[i];
    }
    spin_unlock(&keyboard_lock);
}

static void shutdownHandler (void)
{
    g_terminate = true;
}

static const char* ImGui_ImplCircle_GetClipboardText(void *)
{
    const char *text = g_ClipboardTextData;
    if (!text)
    {
        text = "";
    }
    return text;
}

static void ImGui_ImplCircle_SetClipboardText(void *, const char *text)
{
    if (!text)
    {
        text = "";
    }
    g_ClipboardTextData = text;
}

// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
// If you have multiple SDL events and some of them are not meant to be used by dear imgui, you may need to filter events based on their windowID field.
bool ImGui_ImplCircle_ProcessEvent(void)
{
    ImGuiIO& io = ImGui::GetIO();

    // update mouse position and button states
   spin_lock(&mouse_lock);

   unsigned x, y;
   x = mouse_x;
   y = mouse_y;
   io.MouseDown[0] = g_MousePressed[0];
   io.MouseDown[1] = g_MousePressed[1];
   io.MouseDown[2] = g_MousePressed[2];
   io.MouseWheel += mouse_wheel;
   // TODO
   // io.MouseWheelH += mouse_wheel;
   io.MousePos = ImVec2((float)x, (float)y);

   spin_unlock(&mouse_lock);

   // update keyboard key presses/releases
   spin_lock(&keyboard_lock);

   // signal key release
   // if any of the keys are stil pressed in this frame, they will be set in the key press loop below
   for (unsigned i = 0; i < 6; i++)
   {
       int key = g_rawKeysLast[i];
       if (key == KeyNone)
       {
           continue;
       }
       io.KeysDown[key] = false;
       g_rawKeysLast[i] = KeyNone;
    }

   // signal key press
   for (unsigned i = 0; i < 6; i++)
   {
       int key = g_rawKeys[i];
       if (key == KeyNone)
       {
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

   // set input text if any and also not Enter keypress
   if ((g_cookedKeys[0] != KeyNone) && (g_cookedKeys[0] != '\n'))
   {
       io.AddInputCharacter(g_cookedKeys[0]);
   }
   g_cookedKeys[0] = KeyNone;

   spin_unlock(&keyboard_lock);

    return true;
}

bool ImGui_ImplCircle_Init(void)
{
    // setup backend capabilities flags
    ImGuiIO& io = ImGui::GetIO();
    // we can honor GetMouseCursor() values (optional)
    //io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
    // we can honor io.WantSetMousePos requests (optional, rarely used)
    //io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
    io.BackendPlatformName = "imgui_impl_circle";

    // setup display size (only at init, no window resizing)
    int w = state->screen_width;
    int h = state->screen_height;
    int display_w = state->screen_width;
    int display_h = state->screen_height;
    io.DisplaySize = ImVec2((float)w, (float)h);
    io.DisplayFramebufferScale = ImVec2((float)display_w / w, (float)display_h / h);

    // keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
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
    if (! pMouse->Setup(state->screen_width, state->screen_height))
    {
        printk("Cannot setup mouse\n");
    }
    // using ImGui cursor instead of native one!
    pMouse->SetCursor(0, 0);
    pMouse->RegisterEventHandler(mouseEventHandler);

    CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get()->GetDevice ("ukbd1", FALSE);
    assert(pKeyboard != NULL);
    pKeyboard->RegisterShutdownHandler(shutdownHandler);
    // NOTE:
    // change CUSBKeyboardDevice::ReportHandler() to not return if m_pKeyStatusHandlerRaw
    // is registered to have both KeyPressedHandler and KeyStatusHandlerRaw executed!
    // one can deliver keys to UI and another to serial console
    pKeyboard->RegisterKeyPressedHandler(keyPressedHandler);
    pKeyboard->RegisterKeyStatusHandlerRaw(keyStatusHandlerRaw);

    return true;
}

void ImGui_ImplCircle_Shutdown(void)
{
}

void ImGui_ImplCircle_NewFrame(void)
{
    ImGuiIO& io = ImGui::GetIO();
    IM_ASSERT(io.Fonts->IsBuilt() && "Font atlas not built! It is generally built by the renderer backend. Missing call to renderer _NewFrame() function? e.g. ImGui_ImplOpenGL3_NewFrame().");

    CMouseDevice *pMouse = (CMouseDevice *) CDeviceNameService::Get()->GetDevice("mouse1", FALSE);
    assert(pMouse != NULL);
    pMouse->UpdateCursor();
    mouse_wheel = 0;

    unsigned ticks = CTimer::Get()->GetClockTicks();
    io.DeltaTime = (g_Ticks > 0) ?
                (float)((double)(ticks - g_Ticks) / 1000000) : (float)(1.0f / 60.0f);
    g_Ticks = ticks;
}

//
// following functions would better fit into some other file in order to keep
// ImGui implementation clean
//
static void printConfigInfo(int n, EGLDisplay display, EGLConfig *config)
{

    EGLint red;
    eglGetConfigAttrib(display, *config, EGL_RED_SIZE, &red);
    EGLint blue;
    eglGetConfigAttrib(display, *config, EGL_BLUE_SIZE, &blue);
    EGLint green;
    eglGetConfigAttrib(display, *config, EGL_GREEN_SIZE, &green);
    EGLint alpha;
    eglGetConfigAttrib(display, *config, EGL_ALPHA_SIZE, &alpha);
    EGLint size;
    eglGetConfigAttrib(display, *config, EGL_BUFFER_SIZE, &size);
    EGLint rgb;
   eglGetConfigAttrib(display, *config,  EGL_BIND_TO_TEXTURE_RGB, &rgb);
   EGLint rgba;
   eglGetConfigAttrib(display, *config,  EGL_BIND_TO_TEXTURE_RGBA, &rgba);
   printk("[%02d] R %d G %d B %d A %d S %d %s %s\n",
          n, red, green, blue, alpha, size,
          (rgb == EGL_TRUE)?"Y":"N", (rgba == EGL_TRUE)?"Y":"N");
}

static void vsyncCallback(DISPMANX_UPDATE_HANDLE_T u, void *data)
{
    (void)u;
    (void)data;

    mutex_lock(&vsync_cond_mutex);
    complete(&vsync_cond);
    mutex_unlock(&vsync_cond_mutex);
}

void CircleInit(void)
{
    bcm_host_init();

    // clear application state
    memset(state, 0, sizeof(*state));

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
   eglGetConfigs(state->display, NULL, 0, &num_configs);
   printk("EGL has %d configs\n", num_configs);
   //EGLConfig *configs = (EGLConfig *)calloc(num_configs, sizeof *configs);
   //eglGetConfigs(state->display, configs, num_configs, &num_configs);
   //for (int i = 0; i < num_configs; i++) {
   //     printConfigInfo(i, state->display, &configs[i]);
   //}
   //free(configs);

   // get first EGL frame buffer configuration
   result = eglChooseConfig(state->display, attribute_list, &config, 1, &num_config);
   assert(EGL_FALSE != result);
   check();
   printk("selected config:\n");
   printConfigInfo(1, state->display, &config);

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
   printk("vc4 screen %d x %d\n", state->screen_width, state->screen_height);

   dst_rect.x = 0;
   dst_rect.y = 0;
   dst_rect.width = state->screen_width;
   dst_rect.height = state->screen_height;

   src_rect.x = 0;
   src_rect.y = 0;
   src_rect.width = state->screen_width << 16;
   src_rect.height = state->screen_height << 16;

   state->dispman_display = vc_dispmanx_display_open(0 /* LCD */);
   dispman_update = vc_dispmanx_update_start(0);

   // from SDL_rpivideo.c
   VC_DISPMANX_ALPHA_T dispman_alpha;
   // disable alpha, otherwise the app looks composed with whatever dispman
   // is showing (X11, console,etc)
   dispman_alpha.flags = DISPMANX_FLAGS_ALPHA_FIXED_ALL_PIXELS;
   dispman_alpha.opacity = 0xFF;
   dispman_alpha.mask = 0;

   state->dispman_element = vc_dispmanx_element_add(dispman_update, state->dispman_display,
      0/*layer*/, &dst_rect, 0/*src*/,
      &src_rect, DISPMANX_PROTECTION_NONE,
      &dispman_alpha /*alpha*/, 0/*clamp*/, DISPMANX_NO_ROTATE/*transform*/);

   nativewindow.element = state->dispman_element;
   nativewindow.width = state->screen_width;
   nativewindow.height = state->screen_height;
   vc_dispmanx_update_submit_sync(dispman_update);

   check();

   state->surface = eglCreateWindowSurface(state->display, config, &nativewindow, NULL);
   assert(state->surface != EGL_NO_SURFACE);
   check();

   // connect the context to the surface
   result = eglMakeCurrent(state->display, state->surface, state->surface, state->context);
   assert(EGL_FALSE != result);
   check();

   // Set background color and clear buffers
   glClearColor(0.15f, 0.25f, 0.35f, 1.0f);
   glClear(GL_COLOR_BUFFER_BIT);
   check();

   // start generating vsync callbacks
   init_completion(&vsync_cond);
   mutex_init(&vsync_cond_mutex);
   vc_dispmanx_vsync_callback(state->dispman_display, vsyncCallback, NULL);

    // enable vsync
    result = eglSwapInterval(state->display, 1);
    assert(EGL_FALSE != result);
    check();

    printk("vc4 initialized!\n");
}

void CircleExit(void)
{
   // clear screen
   glClear(GL_COLOR_BUFFER_BIT);
   eglSwapBuffers(state->display, state->surface);

   mutex_lock(&vsync_cond_mutex);
   wait_for_completion(&vsync_cond);
   mutex_unlock(&vsync_cond_mutex);
   vc_dispmanx_vsync_callback(state->dispman_display, NULL, NULL);

   eglDestroySurface(state->display, state->surface);

   DISPMANX_UPDATE_HANDLE_T dispman_update = vc_dispmanx_update_start(0);
   int result = vc_dispmanx_element_remove(dispman_update, state->dispman_element);
   assert(result == 0);
   vc_dispmanx_update_submit_sync(dispman_update);
   result = vc_dispmanx_display_close(state->dispman_display);
   assert (result == 0);

   // release OpenGL resources
   eglMakeCurrent( state->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT );
   eglDestroyContext( state->display, state->context );
   eglTerminate( state->display );

   printk("vc4 terminated!\n");
}

void CircleSwapBuffers(void)
{
    eglSwapBuffers(state->display, state->surface);
    // NOTE:
    // enable this check once the glBlendEquationSeparate() use on RPi is clarified
    // see ImGui_ImplOpenGL3_RenderDrawData()!
    // check();

    // wait for vsync
    mutex_lock(&vsync_cond_mutex);
    wait_for_completion(&vsync_cond);
    mutex_unlock(&vsync_cond_mutex);
}

bool CircleTerminate(void)
{
    return g_terminate;
}
