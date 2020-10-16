#ifndef IMGUI_DEMO_H
#define IMGUI_DEMO_H

#include <circle/input/mousebehaviour.h>

int _main (void);
void mouse_callback (unsigned nButtons, int nDisplacementX, int nDisplacementY);
void mouse_event_callback(TMouseEvent Event, unsigned nButtons, unsigned nPosX, unsigned nPosY, int nWheelMove);

#endif // IMGUI_DEMO_H
