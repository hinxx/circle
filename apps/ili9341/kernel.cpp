//
// kernel.cpp
//
// ILI9341 - graphical LCD 240x320, 4 wire SPI mode
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
#include <circle/debug.h>

static const char FromKernel[] = "kernel";

#define X_CONST 240
#define Y_CONST 320

#define SPI_MASTER_DEVICE	0		// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise

#define SPI_CLOCK_SPEED		80000000	// Hz
#define SPI_CPOL		0
#define SPI_CPHA		0

#define SPI_CHIP_SELECT		0		// 0 or 1, or 2 (for SPI1)


CKernel::CKernel (void)
:	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
#ifndef USE_SPI_MASTER_AUX
	m_SPIMaster (SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE)
#else
	m_SPIMaster (SPI_CLOCK_SPEED)
#endif
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

	bOK = m_Logger.Initialize (&m_Serial);

	if (bOK)
	{
		bOK = m_Interrupt.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Timer.Initialize ();
	}

	if (bOK)
	{
		bOK = m_SPIMaster.Initialize ();
	}

	return TRUE;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write(FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	m_Logger.Write(FromKernel, LogNotice, "ILI9341 4 wire SPI mode");

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

void CKernel::Write_Command(unsigned c)
{
	m_RS.Write(LOW);
	u8 cmd = (u8)c;
	if (m_SPIMaster.Write (SPI_CHIP_SELECT, &cmd, 1) != 1)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error");
	}

}

void CKernel::Write_Data(unsigned c)
{
	m_RS.Write(HIGH);
	u8 data = (u8)c;
	if (m_SPIMaster.Write (SPI_CHIP_SELECT, &data, 1) != 1)
	{
		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error");
	}
}

void CKernel::Lcd_Init(void)
{
	// use pinout header GPIO numbers
	m_RS.AssignPin(25);
	m_RS.SetMode(GPIOModeOutput, TRUE);
	m_RS.Write(HIGH);

	// initialize
	Write_Command(0x11);//sleep out
	CTimer::SimpleMsDelay(20);
	//Write_Command(0x01); //reset
	//delay(15);
	Write_Command(0x28); //display off
	CTimer::SimpleMsDelay(5);
	Write_Command(0xCF); //power control b
	Write_Data(0x00);
	Write_Data(0x83); //83 81 AA
	Write_Data(0x30);
	Write_Command(0xED); //power on seq control
	Write_Data(0x64); //64 67
	Write_Data(0x03);
	Write_Data(0x12);
	Write_Data(0x81);
	Write_Command(0xE8); //timing control a
	Write_Data(0x85);
	Write_Data(0x01);
	Write_Data(0x79); //79 78
	Write_Command(0xCB); //power control a
	Write_Data(0x39);
	Write_Data(0X2C);
	Write_Data(0x00);
	Write_Data(0x34);
	Write_Data(0x02);
	Write_Command(0xF7); //pump ratio control
	Write_Data(0x20);
	Write_Command(0xEA); //timing control b
	Write_Data(0x00);
	Write_Data(0x00);
	Write_Command(0xC0); //power control 2
	Write_Data(0x26); //26 25
	Write_Command(0xC1); //power control 2
	Write_Data(0x11);
	Write_Command(0xC5); //vcom control 1
	Write_Data(0x35);
	Write_Data(0x3E);
	Write_Command(0xC7); //vcom control 2
	Write_Data(0xBE); //BE 94
	Write_Command(0xB1); //frame control
	Write_Data(0x00);
	Write_Data(0x1B); //1B 70
	Write_Command(0xB6); //display control
	Write_Data(0x0A);
	Write_Data(0x82);
	Write_Data(0x27);
	Write_Data(0x00);
	Write_Command(0xB7); //emtry mode
	Write_Data(0x07);
	Write_Command(0x3A); //pixel format
	Write_Data(0x55); //16bit
	Write_Command(0x36); //mem access
	Write_Data((1<<3)|(1<<6));
	//Write_Data((1<<3)|(1<<7)); //rotate 180
	Write_Command(0x29); //display on
	CTimer::SimpleMsDelay(5);
}

void CKernel::SetXY(unsigned x0, unsigned x1, unsigned y0, unsigned y1)
{
	Write_Command(0x2A); //column
	Write_Data(x0>>8);
	Write_Data(x0);
	Write_Data(x1>>8);
	Write_Data(x1);
	Write_Command(0x2B); //page
	Write_Data(y0>>8);
	Write_Data(y0);
	Write_Data(y1>>8);
	Write_Data(y1);
	Write_Command(0x2C); //write

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
