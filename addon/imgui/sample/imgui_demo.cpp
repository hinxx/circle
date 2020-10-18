/*
Copyright (c) 2012, Broadcom Europe Ltd
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the copyright holder nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

// OpenGL|ES 2 demo using shader to compute mandelbrot/julia sets
// Thanks to Peter de Rivas for original Python code

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
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl3.h"

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

   GLuint verbose;
   GLuint vshader;
   GLuint fshader;
   GLuint mshader;
   GLuint program;
   GLuint program2;
   GLuint tex_fb;
   GLuint tex;
   GLuint buf;
// julia attribs
   GLuint unif_color, attr_vertex, unif_scale, unif_offset, unif_tex, unif_centre;
// mandelbrot attribs
   GLuint attr_vertex2, unif_scale2, unif_offset2, unif_centre2;
} CUBE_STATE_T;

static CUBE_STATE_T _state, *state=&_state;

static struct mutex vsync_cond_mutex;
static struct completion vsync_cond;

#define check() assert(glGetError() == 0)

static void showlog(GLint shader)
{
   // Prints the compile log for a shader
   char log[1024];
   glGetShaderInfoLog(shader,sizeof log,NULL,log);
   printk("%d:shader:\n%s\n", shader, log);
}

static void showprogramlog(GLint shader)
{
   // Prints the information log for a program object
   char log[1024];
   glGetProgramInfoLog(shader,sizeof log,NULL,log);
   printk("%d:program:\n%s\n", shader, log);
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
static void init_ogl(CUBE_STATE_T *state)
{
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


static void init_shaders(CUBE_STATE_T *state)
{
   static const GLfloat vertex_data[] = {
        -1.0,-1.0,1.0,1.0,
        1.0,-1.0,1.0,1.0,
        1.0,1.0,1.0,1.0,
        -1.0,1.0,1.0,1.0
   };
   const GLchar *vshader_source =
              "attribute vec4 vertex;"
              "varying vec2 tcoord;"
              "void main(void) {"
              " vec4 pos = vertex;"
              " gl_Position = pos;"
              " tcoord = vertex.xy*0.5+0.5;"
              "}";

   //Mandelbrot
   const GLchar *mandelbrot_fshader_source =
"uniform vec4 color;"
"uniform vec2 scale;"
"uniform vec2 centre;"
"varying vec2 tcoord;"
"void main(void) {"
"  float intensity;"
"  vec4 color2;"
"  float cr=(gl_FragCoord.x-centre.x)*scale.x;"
"  float ci=(gl_FragCoord.y-centre.y)*scale.y;"
"  float ar=cr;"
"  float ai=ci;"
"  float tr,ti;"
"  float col=0.0;"
"  float p=0.0;"
"  int i=0;"
"  for(int i2=1;i2<16;i2++)"
"  {"
"    tr=ar*ar-ai*ai+cr;"
"    ti=2.0*ar*ai+ci;"
"    p=tr*tr+ti*ti;"
"    ar=tr;"
"    ai=ti;"
"    if (p>16.0)"
"    {"
"      i=i2;"
"      break;"
"    }"
"  }"
"  color2 = vec4(float(i)*0.0625,0,0,1);"
"  gl_FragColor = color2;"
"}";

   // Julia
   const GLchar *julia_fshader_source =
"uniform vec4 color;"
"uniform vec2 scale;"
"uniform vec2 centre;"
"uniform vec2 offset;"
"varying vec2 tcoord;"
"uniform sampler2D tex;"
"void main(void) {"
"  float intensity;"
"  vec4 color2;"
"  float ar=(gl_FragCoord.x-centre.x)*scale.x;"
"  float ai=(gl_FragCoord.y-centre.y)*scale.y;"
"  float cr=(offset.x-centre.x)*scale.x;"
"  float ci=(offset.y-centre.y)*scale.y;"
"  float tr,ti;"
"  float col=0.0;"
"  float p=0.0;"
"  int i=0;"
"  vec2 t2;"
"  t2.x=tcoord.x+(offset.x-centre.x)*(0.5/centre.y);"
"  t2.y=tcoord.y+(offset.y-centre.y)*(0.5/centre.x);"
"  for(int i2=1;i2<16;i2++)"
"  {"
"    tr=ar*ar-ai*ai+cr;"
"    ti=2.0*ar*ai+ci;"
"    p=tr*tr+ti*ti;"
"    ar=tr;"
"    ai=ti;"
"    if (p>16.0)"
"    {"
"      i=i2;"
"      break;"
"    }"
"  }"
"  color2 = vec4(0,float(i)*0.0625,0,1);"
"  color2 = color2+texture2D(tex,t2);"
"  gl_FragColor = color2;"
"}";

        state->vshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(state->vshader, 1, &vshader_source, 0);
        glCompileShader(state->vshader);
        check();

        if (state->verbose)
            showlog(state->vshader);

        state->fshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(state->fshader, 1, &julia_fshader_source, 0);
        glCompileShader(state->fshader);
        check();

        if (state->verbose)
            showlog(state->fshader);

        state->mshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(state->mshader, 1, &mandelbrot_fshader_source, 0);
        glCompileShader(state->mshader);
        check();

        if (state->verbose)
            showlog(state->mshader);

        // julia
        state->program = glCreateProgram();
        glAttachShader(state->program, state->vshader);
        glAttachShader(state->program, state->fshader);
        glLinkProgram(state->program);
        check();

        if (state->verbose)
            showprogramlog(state->program);

        state->attr_vertex = glGetAttribLocation(state->program, "vertex");
        state->unif_color  = glGetUniformLocation(state->program, "color");
        state->unif_scale  = glGetUniformLocation(state->program, "scale");
        state->unif_offset = glGetUniformLocation(state->program, "offset");
        state->unif_tex    = glGetUniformLocation(state->program, "tex");
        state->unif_centre = glGetUniformLocation(state->program, "centre");

        // mandelbrot
        state->program2 = glCreateProgram();
        glAttachShader(state->program2, state->vshader);
        glAttachShader(state->program2, state->mshader);
        glLinkProgram(state->program2);
        check();

        if (state->verbose)
            showprogramlog(state->program2);

        state->attr_vertex2 = glGetAttribLocation(state->program2, "vertex");
        state->unif_scale2  = glGetUniformLocation(state->program2, "scale");
        state->unif_offset2 = glGetUniformLocation(state->program2, "offset");
        state->unif_centre2 = glGetUniformLocation(state->program2, "centre");
        check();

        glClearColor ( 0.0, 1.0, 1.0, 1.0 );

        glGenBuffers(1, &state->buf);

        check();

        // Prepare a texture image
        glGenTextures(1, &state->tex);
        check();
        glBindTexture(GL_TEXTURE_2D,state->tex);
        check();
        // glActiveTexture(0)
        glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,state->screen_width,state->screen_height,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,0);
        check();
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        check();
        // Prepare a framebuffer for rendering
        glGenFramebuffers(1,&state->tex_fb);
        check();
        glBindFramebuffer(GL_FRAMEBUFFER,state->tex_fb);
        check();
        glFramebufferTexture2D(GL_FRAMEBUFFER,GL_COLOR_ATTACHMENT0,GL_TEXTURE_2D,state->tex,0);
        check();
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        check();
        // Prepare viewport
        glViewport ( 0, 0, state->screen_width, state->screen_height );
        check();

        // Upload vertex data to a buffer
        glBindBuffer(GL_ARRAY_BUFFER, state->buf);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_data),
                             vertex_data, GL_STATIC_DRAW);
        glVertexAttribPointer(state->attr_vertex, 4, GL_FLOAT, 0, 16, 0);
        glEnableVertexAttribArray(state->attr_vertex);
        glVertexAttribPointer(state->attr_vertex2, 4, GL_FLOAT, 0, 16, 0);
        glEnableVertexAttribArray(state->attr_vertex2);

        check();
}


static void draw_mandelbrot_to_texture(CUBE_STATE_T *state, GLfloat cx, GLfloat cy, GLfloat scale)
{
        // Draw the mandelbrot to a texture
        glBindFramebuffer(GL_FRAMEBUFFER,state->tex_fb);
        check();
        glBindBuffer(GL_ARRAY_BUFFER, state->buf);

        glUseProgram ( state->program2 );
        check();

        glUniform2f(state->unif_scale2, scale, scale);
        glUniform2f(state->unif_centre2, cx, cy);
        check();
        glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );
        check();

        glFlush();
        glFinish();
        check();
}

static void draw_triangles(CUBE_STATE_T *state, GLfloat cx, GLfloat cy, GLfloat scale, GLfloat x, GLfloat y)
{
        // Now render to the main frame buffer
        glBindFramebuffer(GL_FRAMEBUFFER,0);
        // Clear the background (not really necessary I suppose)
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        check();

        glBindBuffer(GL_ARRAY_BUFFER, state->buf);
        check();
        glUseProgram ( state->program );
        check();
        glBindTexture(GL_TEXTURE_2D,state->tex);
        check();
        glUniform4f(state->unif_color, 0.5, 0.5, 0.8, 1.0);
        glUniform2f(state->unif_scale, scale, scale);
        glUniform2f(state->unif_offset, x, y);
        glUniform2f(state->unif_centre, cx, cy);
        glUniform1i(state->unif_tex, 0); // I don't really understand this part, perhaps it relates to active texture?
        check();

        glDrawArrays ( GL_TRIANGLE_FAN, 0, 4 );
        check();

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glFlush();
        glFinish();
        check();

        eglSwapBuffers(state->display, state->surface);
        check();
}

#ifdef __circle__

static DEFINE_SPINLOCK (mouse_lock);
static int mouse_buttons = 0, mouse_dx = 0, mouse_dy = 0;
static unsigned mouse_x = 0;
static unsigned mouse_y = 0;
static bool g_MousePressed[3] = { false, false, false };
static int mouse_wheel = 0;

void mouse_callback (unsigned buttons, int dx, int dy)
{
   spin_lock (&mouse_lock);

   mouse_buttons = (int) buttons;
   mouse_dx += dx;
   mouse_dy += dy;

   spin_unlock (&mouse_lock);
}

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

static int get_mouse(CUBE_STATE_T *state, int *outx, int *outy)
{
   const int width=state->screen_width, height=state->screen_height;
   static int x=800, y=400;
   int buttons, dx, dy;

   spin_lock (&mouse_lock);

   buttons = mouse_buttons;
   dx = mouse_dx;
   dy = mouse_dy;

   mouse_dx = mouse_dy = 0;

   spin_unlock (&mouse_lock);

   x += dx;
   y += dy;
   if (x < 0) x = 0;
   if (y < 0) y = 0;
   if (x > width) x = width;
   if (y > height) y = height;

   if (outx) *outx = x;
   if (outy) *outy = y;

   return buttons & 3;
}

static DEFINE_SPINLOCK (keyboard_lock);
static unsigned char g_cookedKeys[2] = {0};
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

static unsigned char g_rawKeys[6] = {0};
//#define KEY_LCTRL_MASK		(1 << 0)
//#define KEY_LSHIFT_MASK		(1 << 1)
//#define KEY_ALT_MASK		(1 << 2)
//#define KEY_LWIN_MASK		(1 << 3)
//#define KEY_RCTRL_MASK		(1 << 4)
//#define KEY_RSHIFT_MASK		(1 << 5)
//#define KEY_ALTGR_MASK		(1 << 6)
//#define KEY_RWIN_MASK		(1 << 7)
static unsigned char g_modifierKeys = 0;
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

static bool g_terminate = false;
void ShutdownHandler (void) {
//    assert (s_pThis != 0);
//	s_pThis->m_ShutdownMode = ShutdownReboot;
    g_terminate = true;
}

static CString g_clipboard;
void setClipboardText(void *user_data, const char *text) {
    (void)user_data;
    if (!text) {
        text = "";
    }
    g_clipboard = text;
    //printk("SET clipboard content: '%s'", (const char *)g_clipboard, g_clipboard.GetLength());
}

const char *getClipboardText(void *user_data) {
    (void)user_data;
    const char *text = g_clipboard;
    if (!text) {
        text = "";
    }
    //printk("GET clipboard content: '%s'", (const char *)text, strlen(text));
    return text;
}

#else

static int get_mouse(CUBE_STATE_T *state, int *outx, int *outy)
{
    static int fd = -1;
    const int width=state->screen_width, height=state->screen_height;
    static int x=800, y=400;
    const int XSIGN = 1<<4, YSIGN = 1<<5;
    if (fd<0) {
       fd = open("/dev/input/mouse0",O_RDONLY|O_NONBLOCK);
    }
    if (fd>=0) {
        struct {char buttons, dx, dy; } m;
        while (1) {
           int bytes = read(fd, &m, sizeof m);
           if (bytes < (int)sizeof m) goto _exit;
           if (m.buttons&8) {
              break; // This bit should always be set
           }
           read(fd, &m, 1); // Try to sync up again
        }
        if (m.buttons&3)
           return m.buttons&3;
        x+=m.dx;
        y+=m.dy;
        if (m.buttons&XSIGN)
           x-=256;
        if (m.buttons&YSIGN)
           y-=256;
        if (x<0) x=0;
        if (y<0) y=0;
        if (x>width) x=width;
        if (y>height) y=height;
   }
_exit:
   if (outx) *outx = x;
   if (outy) *outy = y;
   return 0;
}

#endif

//==============================================================================

static void exit_func(void)
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

int _main0 ()
{
   int terminate = 0;
   GLfloat cx, cy;
   bcm_host_init();

   // Clear application state
   memset( state, 0, sizeof( *state ) );

   // Start OGLES
   init_ogl(state);
   init_shaders(state);
   cx = state->screen_width/2;
   cy = state->screen_height/2;
   printk("Screen %d x %d\n", state->screen_width, state->screen_height);

   draw_mandelbrot_to_texture(state, cx, cy, 0.003);
   while (!terminate)
   {
      int x, y, b;
      b = get_mouse(state, &x, &y);
      if (b) break;
      draw_triangles(state, cx, cy, 0.003, x, y);
   }

   exit_func();

   return 0;
}


int _main ()
{
//   int terminate = 0;
//   GLfloat cx, cy;
   bcm_host_init();

   // Clear application state
   memset( state, 0, sizeof( *state ) );

   // Start OGLES
   init_ogl(state);
//   init_shaders(state);
//   cx = state->screen_width/2;
//   cy = state->screen_height/2;
   printk("Screen %d x %d\n", state->screen_width, state->screen_height);


   CMouseDevice *pMouse = (CMouseDevice *) CDeviceNameService::Get()->GetDevice("mouse1", FALSE);
   assert(pMouse != NULL);
   if (! pMouse->Setup(state->screen_width, state->screen_height)) {
       printk("Cannot setup mouse\n");
   }
   pMouse->SetCursor(state->screen_width/2, state->screen_height/2);
   // XXX: consider using ImGui cursor instead of native one!
   pMouse->ShowCursor(TRUE);
   pMouse->RegisterEventHandler(mouse_event_callback);

   CUSBKeyboardDevice *pKeyboard = (CUSBKeyboardDevice *) CDeviceNameService::Get()->GetDevice ("ukbd1", FALSE);
   assert(pKeyboard != NULL);
   pKeyboard->RegisterShutdownHandler(ShutdownHandler);
   // change CUSBKeyboardDevice::ReportHandler() to not return if m_pKeyStatusHandlerRaw
   // is registered to have both KeyPressedHandler and KeyStatusHandlerRaw executed!
   // one can deliver keys to UI and another to serial console
   pKeyboard->RegisterKeyPressedHandler(KeyPressedHandler);
   pKeyboard->RegisterKeyStatusHandlerRaw(KeyStatusHandlerRaw);

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO& io = ImGui::GetIO(); (void)io;
   //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
   //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

   // Setup Dear ImGui style
   ImGui::StyleColorsDark();
   //ImGui::StyleColorsClassic();

   // Setup Platform/Renderer bindings
//   ImGui_ImplSDL2_InitForOpenGL(NULL, NULL);
   io.BackendPlatformName = "imgui_impl_circle";

   io.SetClipboardTextFn = setClipboardText;
   io.GetClipboardTextFn = getClipboardText;
   io.ClipboardUserData = NULL;

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

   const char* glsl_version = "#version 100";
   ImGui_ImplOpenGL3_Init(glsl_version);
//   ImGui_ImplOpenGL3_Init(NULL);
   printk("after ImGui_ImplOpenGL3_Init\n");

   bool show_demo_window = true;
   bool show_another_window = false;
   ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

//   draw_mandelbrot_to_texture(state, cx, cy, 0.003);
//   while (!terminate)
//   {
//      int x, y, b;
//      b = get_mouse(state, &x, &y);
//      if (b) break;
//      draw_triangles(state, cx, cy, 0.003, x, y);
//   }

   int iter = 0;
   bool terminate = false;
   unsigned char g_rawKeysLast[6] = {0};
   while(! terminate) {
        iter++;

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

   // Start the Dear ImGui frame
   ImGui_ImplOpenGL3_NewFrame();
   //printk("after ImGui_ImplOpenGL3_NewFrame\n");
//   ImGui_ImplSDL2_NewFrame(window);

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

   ImGui::NewFrame();
   if (show_demo_window)
       ImGui::ShowDemoWindow(&show_demo_window);


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
       ImGui::Text("loop iter = %d", iter);
       ImGui::Text("mouse X %4d Y %4d", (int)io.MousePos.x, (int)io.MousePos.y);
       ImGui::Text("mouse buttons = LEFT %d RIGHT %d MIDDLE %d", io.MouseDown[0], io.MouseDown[1], io.MouseDown[2]);

       ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

       // on mouse click
       if (ImGui::Button("Terminate"))
           terminate = true;
       // also with Ctrl+Alt+Del
       if (g_terminate)
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
   // Clear the background (not really necessary I suppose)
   //glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

   // debug
   ImDrawData *drawData = ImGui::GetDrawData();
   if (drawData->CmdListsCount == 0) {
       printk("iter %d, drawData->CmdListsCount == 0!\n", iter);
   }
   // debug

   ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
   //printk("after ImGui_ImplOpenGL3_RenderDrawData\n");

//   glFlush();
//   glFinish();

   //SDL_GL_SwapWindow(window);
   eglSwapBuffers(state->display, state->surface);
   //check();

#if 0
   CScheduler *sched = CScheduler::Get();
   sched->usSleep(16);
#else
   // wait for vsync
   mutex_lock(&vsync_cond_mutex);
   wait_for_completion(&vsync_cond);
   mutex_unlock(&vsync_cond_mutex);
#endif

   }

   // Cleanup
   ImGui_ImplOpenGL3_Shutdown();
   printk("after ImGui_ImplOpenGL3_Shutdown\n");
//   ImGui_ImplSDL2_Shutdown();

   ImGui::DestroyContext();

   exit_func();

   return 0;
}
