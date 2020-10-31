//
// ft6x06.h
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2020  H. Kocevar <hinxx@protonmail.com>
//
// See: https://www.adafruit.com/product/2298
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
#include <circle/devicenameservice.h>
#include <circle/logger.h>

#include <circle/input/ft6206.h>

//#define FT62XX_ADDR 0x38           //!< I2C address
//#define FT62XX_G_FT5201ID 0xA8     //!< FocalTech's panel ID
#define FT62XX_REG_NUMTOUCHES 0x02 //!< Number of touch points

//#define FT62XX_NUM_X 0x33 //!< Touch X position
//#define FT62XX_NUM_Y 0x34 //!< Touch Y position

//#define FT62XX_REG_MODE 0x00        //!< Device mode, either WORKING or FACTORY
//#define FT62XX_REG_CALIBRATE 0x02   //!< Calibrate mode
//#define FT62XX_REG_WORKMODE 0x00    //!< Work mode
//#define FT62XX_REG_FACTORYMODE 0x40 //!< Factory mode
#define FT62XX_REG_THRESHOLD 0x80   //!< Threshold for touch detection
//#define FT62XX_REG_POINTRATE 0x88   //!< Point rate
//#define FT62XX_REG_FIRMVERS 0xA6    //!< Firmware version
#define FT62XX_REG_CHIPID 0xA3      //!< Chip selecting
#define FT62XX_REG_VENDID 0xA8      //!< FocalTech's panel ID

#define FT6X06_VENDOR_ID 0x11  //!< FocalTech's panel ID
#define FT6X06_CHIP_ID 0x06  //!< Chip selecting

// calibrated for Adafruit 2.8" ctp screen
#define FT62XX_DEFAULT_THRESHOLD 128 //!< Default threshold for touch detection

#define FT62XX_TOUCH_DOWN		0
#define FT62XX_TOUCH_UP			1
#define FT62XX_TOUCH_CONTACT		2

static const char FromFt6x06[] = "ft6x06";

CFT6x06Device::CFT6x06Device(CI2CMaster *pI2CMaster, u8 ucAddress, u8 ucThreshold)
:	m_pI2CMaster (pI2CMaster),
	m_pEventHandler (0),
	m_ucAddress (ucAddress),
	m_ucThreshold (ucThreshold),
	m_nKnownIDs (0)
{
	assert(m_pI2CMaster != 0);
	assert(m_ucAddress > 0);
}

CFT6x06Device::~CFT6x06Device()
{
	m_pI2CMaster = 0;
}

boolean CFT6x06Device::Initialize(void)
{
	assert(m_pI2CMaster != 0);

	u8 txBuffer[2] = { 0 };
	u8 rxData = 0;

	txBuffer[0] = FT62XX_REG_VENDID;
	if (!WriteRead(txBuffer, 1, &rxData, 1))
	{
		return FALSE;
	}
	u8 ucVendorID = rxData;

	txBuffer[0] = FT62XX_REG_CHIPID;
	if (!WriteRead(txBuffer, 1, &rxData, 1))
	{
		return FALSE;
	}
	u8 ucChipID = rxData;

	txBuffer[0] = FT62XX_REG_THRESHOLD;
	txBuffer[1] = m_ucThreshold;
	if (!WriteRead(txBuffer, 2, 0, 0))
	{
		return FALSE;
	}

	if (ucVendorID == FT6X06_VENDOR_ID && ucChipID == FT6X06_CHIP_ID)
	{
		CLogger::Get()->Write(FromFt6x06, LogNotice, "Detected FT6206 device");
	}
	else
	{
		CLogger::Get()->Write(FromFt6x06, LogWarning, "Unsupported device 0x%02X%02X",
				      ucVendorID, ucChipID);

		return FALSE;
	}

	CDeviceNameService::Get()->AddDevice("touch1", this, FALSE);

	return TRUE;
}

void CFT6x06Device::Update(void)
{
	assert (m_pI2CMaster != 0);

	// select first register
	u8 txBuffer = 0x00;
	// read 16 registers
	u8 rxData[16] = { 0 };
	if (!WriteRead(&txBuffer, 1, rxData, sizeof(rxData)))
	{
		return;
	}

	// figure out number of detected touches
	unsigned touches = rxData[FT62XX_REG_NUMTOUCHES];
	if (touches > TOUCH_SCREEN_MAX_POINTS || touches == 0)
	{
		return;
	}

	unsigned nModifiedIDs = 0;
	for (unsigned i = 0; i < TOUCH_SCREEN_MAX_POINTS; i++)
	{
		unsigned x = ((rxData[0x03 + i * 6] & 0x0F) << 8) | rxData[0x04 + i * 6];
		unsigned y = ((rxData[0x05 + i * 6] & 0x0F) << 8) | rxData[0x06 + i * 6];
		unsigned nEventID = (rxData[0x03 + i * 6] & 0xC0) >> 6;
		unsigned nTouchID = rxData[0x05 + i * 6] >> 4;
		//CLogger::Get()->Write(FromFt6x06, LogNotice, "ID %4d X %4d Y %4d EV %4d", nTouchID, x, y, nEventID);
		// check for invalid touch ID
		if (nTouchID == 0x0F)
		{
			continue;
		}
		assert (nTouchID < TOUCH_SCREEN_MAX_POINTS);

		nModifiedIDs |= 1 << nTouchID;

		if (nEventID == FT62XX_TOUCH_CONTACT || nEventID == FT62XX_TOUCH_DOWN)
		{
			if (!((1 << nTouchID) & m_nKnownIDs))
			{
				m_nPosX[nTouchID] = x;
				m_nPosY[nTouchID] = y;

				if (m_pEventHandler != 0)
				{
					(*m_pEventHandler) (TouchScreenEventFingerDown, nTouchID, x, y);
				}
			}
			else
			{
				if (x != m_nPosX[nTouchID] || y != m_nPosY[nTouchID])
				{
					m_nPosX[nTouchID] = x;
					m_nPosY[nTouchID] = y;

					if (m_pEventHandler != 0)
					{
						(*m_pEventHandler) (TouchScreenEventFingerMove, nTouchID, x, y);
					}
				}
			}
		}
	}

	unsigned nReleasedIDs = m_nKnownIDs & ~nModifiedIDs;
	for (unsigned i = 0; nReleasedIDs != 0 && i < TOUCH_SCREEN_MAX_POINTS; i++)
	{
		if (nReleasedIDs & (1 << i))
		{
			if (m_pEventHandler != 0)
			{
				(*m_pEventHandler) (TouchScreenEventFingerUp, i, 0, 0);
			}

			nModifiedIDs &= ~(1 << i);
		}
	}

	m_nKnownIDs = nModifiedIDs;
}

void CFT6x06Device::RegisterEventHandler(TTouchScreenEventHandler *pEventHandler)
{
	assert (m_pEventHandler == 0);
	m_pEventHandler = pEventHandler;
	assert (m_pEventHandler != 0);
}

boolean CFT6x06Device::WriteRead(const void *pTxBuffer, unsigned nTxCount,
				 void *pRxBuffer, unsigned nRxCount)
{
	assert(pTxBuffer != 0);

	if (m_pI2CMaster->Write(m_ucAddress, pTxBuffer, nTxCount) != (int) nTxCount)
	{
		CLogger::Get()->Write(FromFt6x06, LogError, "I2C write error");

		return FALSE;
	}
	if (pRxBuffer)
	{
		if (m_pI2CMaster->Read(m_ucAddress, pRxBuffer, nRxCount) != (int) nRxCount)
		{
			CLogger::Get()->Write(FromFt6x06, LogError, "I2C read error");

			return FALSE;
		}
	}

	return TRUE;
}
