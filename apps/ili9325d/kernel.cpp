//
// kernel.cpp
//
// ILI9325D - graphical LCD 240x320, 8-bit mode
// Copyright (C) 2020  H. Kocevar <hinxx@protonmail.com>
// 
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
#include "kernel.h"
#include <circle/gpiopin.h>
#include <circle/timer.h>

static const char FromKernel[] = "kernel";

#define X_CONST 240
#define Y_CONST 320

CKernel::CKernel (void)
:	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer)
{
    m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
}

boolean CKernel::Initialize (void)
{
    boolean bOK = TRUE;

    if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

    if (bOK)
	{
		bOK = m_Logger.Initialize (&m_Serial);
	}

    if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

    return TRUE;
}

TShutdownMode CKernel::Run (void)
{
    m_Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

    m_Logger.Write(FromKernel, LogNotice, "ILI9325D 8-bit mode");

    Lcd_Init();
    LCD_clear();

    CTimer::SimpleMsDelay(500);
    Paint(0xF800);
    CTimer::SimpleMsDelay(500);
    Paint(0x07e0);
    CTimer::SimpleMsDelay(500);
	Paint(0x001f);
    CTimer::SimpleMsDelay(500);
	Paint(0xffff);
    CTimer::SimpleMsDelay(500);
	Paint(0x0000);

    for (unsigned i = 0; i < 0xFFFF; i += 0x1F) {
        Paint(i);
        m_Logger.Write(FromKernel, LogNotice, "color %4X", i);
    }

    m_Logger.Write(FromKernel, LogNotice, "\nRebooting..");
	return ShutdownReboot;
}

void CKernel::Write_Command(unsigned int c)
{
    m_RS.Write(LOW);
	m_nCS.Write(LOW);
    for (int i = 0; i < 8; i++) {
        m_DB[i].Write((c & (1 << (i + 8))) ? HIGH : LOW);
    }
	m_nWR.Write(LOW);
	m_nWR.Write(HIGH);

    for (int i = 0; i < 8; i++) {
        m_DB[i].Write((c & (1 << i)) ? HIGH : LOW);
    }
	m_nWR.Write(LOW);
	m_nWR.Write(HIGH);
	m_nCS.Write(HIGH);
}

void CKernel::Write_Data(unsigned int c)
{
    m_RS.Write(HIGH);
	m_nCS.Write(LOW);
    for (int i = 0; i < 8; i++) {
        m_DB[i].Write((c & (1 << (i + 8))) ? HIGH : LOW);
    }
	m_nWR.Write(LOW);
	m_nWR.Write(HIGH);

    for (int i = 0; i < 8; i++) {
        m_DB[i].Write((c & (1 << i)) ? HIGH : LOW);
    }
	m_nWR.Write(LOW);
	m_nWR.Write(HIGH);
	m_nCS.Write(HIGH);
}

void CKernel::Write_Command_Data(unsigned cmd, unsigned dat)
{
	Write_Command(cmd);
	Write_Data(dat);
}

void CKernel::Lcd_Init(void)
{
    // use pinout header GPIO numbers
    m_nWR.AssignPin(1);
    m_nWR.SetMode(GPIOModeOutput, TRUE);
    m_nWR.Write(HIGH);

    m_RS.AssignPin(5);
    m_RS.SetMode(GPIOModeOutput, TRUE);
    m_RS.Write(HIGH);

    m_RST.AssignPin(23);
    m_RST.SetMode(GPIOModeOutput, TRUE);
    m_RST.Write(HIGH);

    m_nCS.AssignPin(22);
    m_nCS.SetMode(GPIOModeOutput, TRUE);
    m_nCS.Write(HIGH);

    int DB[8] = { 21, 20, 26, 16, 19, 13, 12, 6 };
    for (int i = 0; i < 8; i++) {
        m_DB[i].AssignPin(DB[i]);
        m_DB[i].SetMode(GPIOModeOutput, TRUE);
        m_DB[i].Write(HIGH);
    }

    // reset
    m_RST.Write(HIGH);
    CTimer::SimpleMsDelay(1);
    m_RST.Write(LOW);
    CTimer::SimpleMsDelay(1);
    m_RST.Write(HIGH);
    CTimer::SimpleMsDelay(20);

    // initialize
    Write_Command_Data(0xE5, 0x78F0); // set SRAM internal timing
	Write_Command_Data(0x01, 0x0100); // set Driver Output Control
	Write_Command_Data(0x02, 0x0200); // set 1 line inversion
	Write_Command_Data(0x03, 0x1030); // set GRAM write direction and BGR=1.
	Write_Command_Data(0x04, 0x0000); // Resize register
	Write_Command_Data(0x08, 0x0207); // set the back porch and front porch
	Write_Command_Data(0x09, 0x0000); // set non-display area refresh cycle ISC[3:0]
	Write_Command_Data(0x0A, 0x0000); // FMARK function
	Write_Command_Data(0x0C, 0x0000); // RGB interface setting
	Write_Command_Data(0x0D, 0x0000); // Frame marker Position
	Write_Command_Data(0x0F, 0x0000); // RGB interface polarity
	//*************Power On sequence ****************//
	Write_Command_Data(0x10, 0x0000); // SAP, BT[3:0], AP, DSTB, SLP, STB
	Write_Command_Data(0x11, 0x0007); // DC1[2:0], DC0[2:0], VC[2:0]
	Write_Command_Data(0x12, 0x0000); // VREG1OUT voltage
	Write_Command_Data(0x13, 0x0000); // VDV[4:0] for VCOM amplitude
	Write_Command_Data(0x07, 0x0001);
	CTimer::SimpleMsDelay(200); // Dis-charge capacitor power voltage
	Write_Command_Data(0x10, 0x1690); // SAP, BT[3:0], AP, DSTB, SLP, STB
	Write_Command_Data(0x11, 0x0227); // Set DC1[2:0], DC0[2:0], VC[2:0]
	CTimer::SimpleMsDelay(50); // Delay 50ms
	Write_Command_Data(0x12, 0x000D); // 0012
	CTimer::SimpleMsDelay(50); // Delay 50ms
	Write_Command_Data(0x13, 0x1200); // VDV[4:0] for VCOM amplitude
	Write_Command_Data(0x29, 0x000A); // 04  VCM[5:0] for VCOMH
	Write_Command_Data(0x2B, 0x000D); // Set Frame Rate
	CTimer::SimpleMsDelay(50); // Delay 50ms
	Write_Command_Data(0x20, 0x0000); // GRAM horizontal Address
	Write_Command_Data(0x21, 0x0000); // GRAM Vertical Address
	// ----------- Adjust the Gamma Curve ----------//
	Write_Command_Data(0x30, 0x0000);
	Write_Command_Data(0x31, 0x0404);
	Write_Command_Data(0x32, 0x0003);
	Write_Command_Data(0x35, 0x0405);
	Write_Command_Data(0x36, 0x0808);
	Write_Command_Data(0x37, 0x0407);
	Write_Command_Data(0x38, 0x0303);
	Write_Command_Data(0x39, 0x0707);
	Write_Command_Data(0x3C, 0x0504);
	Write_Command_Data(0x3D, 0x0808);
	//------------------ Set GRAM area ---------------//
	Write_Command_Data(0x50, 0x0000); // Horizontal GRAM Start Address
	Write_Command_Data(0x51, 0x00EF); // Horizontal GRAM End Address
	Write_Command_Data(0x52, 0x0000); // Vertical GRAM Start Address
	Write_Command_Data(0x53, 0x013F); // Vertical GRAM Start Address
	Write_Command_Data(0x60, 0xA700); // Gate Scan Line
	Write_Command_Data(0x61, 0x0001); // NDL,VLE, REV
	Write_Command_Data(0x6A, 0x0000); // set scrolling line
	//-------------- Partial Display Control ---------//
	Write_Command_Data(0x80, 0x0000);
	Write_Command_Data(0x81, 0x0000);
	Write_Command_Data(0x82, 0x0000);
	Write_Command_Data(0x83, 0x0000);
	Write_Command_Data(0x84, 0x0000);
	Write_Command_Data(0x85, 0x0000);
	//-------------- Panel Control -------------------//
	Write_Command_Data(0x90, 0x0010);
	Write_Command_Data(0x92, 0x0000);
	Write_Command_Data(0x07, 0x0133); // 262K color and display ON

    Write_Command(0x0022);
}

void CKernel::SetXY(unsigned x0, unsigned x1, unsigned y0, unsigned y1)
{
    Write_Command_Data(0x20, x0);
	Write_Command_Data(0x21, y0);
	Write_Command_Data(0x50, x0);
	Write_Command_Data(0x52, y0);
	Write_Command_Data(0x51, x1);
	Write_Command_Data(0x53, y1);
	Write_Command(0x22);
}

void CKernel::Paint(unsigned color)
{
    SetXY(0, X_CONST-1, 0, Y_CONST-1);
    for(unsigned i = 0; i < Y_CONST; i++) {
        for (unsigned j = 0; j < X_CONST; j++) {
            Write_Data(color);
        }
    }
}

void CKernel::LCD_clear(void)
{
    SetXY(0, X_CONST-1, 0, Y_CONST-1);
    for(unsigned i = 0; i < Y_CONST; i++) {
        for(unsigned j = 0; j < X_CONST; j++) {
            Write_Data(0x0000);
        }
    }
}
