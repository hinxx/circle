//
// kernel.cpp
//
// FT6x06 - capacitive touchscreen
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
    m_FT6x06 (&m_I2CMaster, 0x38)
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

        pTarget = &m_Serial;
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
		bOK = m_FT6x06.Initialize ();
	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

#if 0
    u8 cmd = 0xA8;
    if (m_I2CMaster.Write(0x38, &cmd, sizeof(cmd)) != (int) sizeof(cmd)) {
		CLogger::Get()->Write(FromKernel, LogPanic, "I2C write error (cmd)");
	}
    u8 data[16] = { 0xFF };
    if (m_I2CMaster.Read(0x38, data, 1) != (int) 1) {
		CLogger::Get()->Write(FromKernel, LogPanic, "I2C read error (data)");
	}
    CLogger::Get()->Write(FromKernel, LogNotice, "0x%02X = 0x%02X", cmd, data[0]);

    cmd = 0xA3;
    if (m_I2CMaster.Write(0x38, &cmd, sizeof(cmd)) != (int) sizeof(cmd)) {
		CLogger::Get()->Write(FromKernel, LogPanic, "I2C write error (cmd)");
	}
    if (m_I2CMaster.Read(0x38, data, 1) != (int) 1) {
		CLogger::Get()->Write(FromKernel, LogPanic, "I2C read error (data)");
	}
    CLogger::Get()->Write(FromKernel, LogNotice, "0x%02X = 0x%02X", cmd, data[0]);

    while (1) {

    cmd = 0;
    if (m_I2CMaster.Write(0x38, &cmd, sizeof(cmd)) != (int) sizeof(cmd)) {
        CLogger::Get()->Write(FromKernel, LogPanic, "I2C write error (cmd)");
    }
    if (m_I2CMaster.Read(0x38, data, 16) != (int) 16) {
        CLogger::Get()->Write(FromKernel, LogPanic, "I2C read error (data)");
    }
//    for (int i = 0; i < 16; i++) {
//        CLogger::Get()->Write(FromKernel, LogNotice, "0x%02X = 0x%02X", i, data[i]);
//    }
    CLogger::Get()->Write(FromKernel, LogNotice, "[2] 0x%02X [3] 0x%02X [3] 0x%02X [3] 0x%02X [3] 0x%02X [3] 0x%02X [3] 0x%02X",
                          data[2], data[3], data[4], data[5], data[6], data[7], data[8]);

    u8 touches = data[0x02];
    if ((touches > 2) || (touches == 0)) {
        touches = 0;
    }

    if (touches) {
        u16 touchX[2], touchY[2], touchID[2];
        for (u8 i = 0; i < 2; i++) {
            touchX[i] = data[0x03 + i * 6] & 0x0F;
            touchX[i] <<= 8;
            touchX[i] |= data[0x04 + i * 6];
            touchY[i] = data[0x05 + i * 6] & 0x0F;
            touchY[i] <<= 8;
            touchY[i] |= data[0x06 + i * 6];
            touchID[i] = data[0x05 + i * 6] >> 4;
        }

        CLogger::Get ()->Write (FromKernel, LogNotice, "X %4d Y %4d Z %4d", touchX[0], touchY[0], touchID[0]);
    }

    m_Timer.MsDelay (1000);

    }
    return ShutdownReboot;
#else
    CFT6x06Device *pTouchScreen =
		(CFT6x06Device *) m_DeviceNameService.GetDevice ("touch1", FALSE);
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
#endif
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
