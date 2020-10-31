//
// kernel.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
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

#define I2C_MASTER_DEVICE	1		// 0 on Raspberry Pi 1 Rev. 1 boards, 1 otherwise
#define I2C_MASTER_CONFIG	0		// 0 or 1 on Raspberry Pi 4, 0 otherwise
#define I2C_FAST_MODE		FALSE		// standard mode (100 Kbps) or fast mode (400 Kbps)

static const char FromKernel[] = "kernel";

CKernel *CKernel::s_pThis = 0;

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer),
	m_I2CMaster (I2C_MASTER_DEVICE, I2C_FAST_MODE, I2C_MASTER_CONFIG),
	m_FT6206 (&m_I2CMaster)
{
	s_pThis = this;

	m_ActLED.Blink (5);	// show we are alive
}

CKernel::~CKernel (void)
{
	s_pThis = 0;
}

boolean CKernel::Initialize (void)
{
	boolean bOK = TRUE;

	if (bOK)
	{
		bOK = m_Screen.Initialize ();
	}

	if (bOK)
	{
		bOK = m_Serial.Initialize (115200);
	}

	if (bOK)
	{
		CDevice *pTarget = m_DeviceNameService.GetDevice (m_Options.GetLogDevice (), FALSE);
		if (pTarget == 0)
		{
			pTarget = &m_Screen;
		}

		bOK = m_Logger.Initialize (pTarget);
	}

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
		bOK = m_I2CMaster.Initialize ();
	}

	if (bOK)
	{
		bOK = m_FT6206.Initialize ();
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	CFT6206Device *pTouchScreen =
		(CFT6206Device *) m_DeviceNameService.GetDevice ("touch1", FALSE);
	if (pTouchScreen == 0)
	{
		m_Logger.Write (FromKernel, LogPanic, "Touchscreen not found");
	}

	pTouchScreen->RegisterEventHandler (TouchScreenEventHandler);

	m_Logger.Write (FromKernel, LogNotice, "Just use your touchscreen!");

	for (unsigned nCount = 0; 1; nCount++)
	{
		pTouchScreen->Update ();

		m_Screen.Rotor (0, nCount);
		m_Timer.MsDelay (1000/60);
	}

	return ShutdownReboot;
}

void CKernel::TouchScreenEventHandler (TTouchScreenEvent Event,
				       unsigned nID, unsigned nPosX, unsigned nPosY)
{
	assert (s_pThis != 0);

	CString Message;

	switch (Event)
	{
	case TouchScreenEventFingerDown:
		Message.Format ("Finger #%u down at %u / %u", nID+1, nPosX, nPosY);
		break;

	case TouchScreenEventFingerUp:
		Message.Format ("Finger #%u up", nID+1);
		break;

	case TouchScreenEventFingerMove:
		Message.Format ("Finger #%u moved to %u / %u", nID+1, nPosX, nPosY);
		break;

	default:
		assert (0);
		break;
	}

	s_pThis->m_Logger.Write (FromKernel, LogNotice, Message);
}
