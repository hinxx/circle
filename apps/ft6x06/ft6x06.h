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
#ifndef FT6X06_H
#define FT6X06_H

#include <circle/device.h>
#include <circle/types.h>
#include <circle/i2cmaster.h>

enum TTouchScreenEvent
{
	TouchScreenEventFingerDown,
	TouchScreenEventFingerUp,
	TouchScreenEventFingerMove,
	TouchScreenEventUnknown
};
#define TOUCH_SCREEN_MAX_POINTS		2

typedef void TTouchScreenEventHandler (TTouchScreenEvent Event,
				       unsigned nID, unsigned nPosX, unsigned nPosY);

class CFT6x06Device : public CDevice
{
public:
    CFT6x06Device(CI2CMaster *pI2CMaster, u8 ucAddress = 0x36);
    ~CFT6x06Device();

    boolean Initialize (void);

    void Update (void);		// call this about 60 times per second

	void RegisterEventHandler (TTouchScreenEventHandler *pEventHandler);

private:
    CI2CMaster *m_pI2CMaster;
    TTouchScreenEventHandler *m_pEventHandler;

    u8 m_ucAddress;
    u8 m_ucVendorID;
    u8 m_ucChipID;
    unsigned m_nKnownIDs;

    u16 m_nPosX[TOUCH_SCREEN_MAX_POINTS];
    u16 m_nPosY[TOUCH_SCREEN_MAX_POINTS];
};

#endif // FT6X06_H
