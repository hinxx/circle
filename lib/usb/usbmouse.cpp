//
// usbmouse.cpp
//
// Circle - A C++ bare metal environment for Raspberry Pi
// Copyright (C) 2014-2018  R. Stange <rsta2@o2online.de>
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
#include <circle/usb/usbmouse.h>
#include <circle/usb/usbhid.h>
#include <circle/logger.h>
#include <circle/debug.h>
#include <assert.h>

// in boot protocol 3 bytes are received, in report protocol 5 bytes are received
//#define REPORT_SIZE 3
#define REPORT_SIZE 5

static const char FromUSBMouse[] = "umouse";

CUSBMouseDevice::CUSBMouseDevice (CUSBFunction *pFunction)
:	CUSBHIDDevice (pFunction, REPORT_SIZE),
	m_pMouseDevice (0),
	m_pHIDReportDescriptor (0)
{
}

CUSBMouseDevice::~CUSBMouseDevice (void)
{
	delete m_pMouseDevice;
	m_pMouseDevice = 0;

	delete [] m_pHIDReportDescriptor;
	m_pHIDReportDescriptor = 0;
}

boolean CUSBMouseDevice::Configure (void)
{
	TUSBHIDDescriptor *pHIDDesc = (TUSBHIDDescriptor *) GetDescriptor (DESCRIPTOR_HID);
	if (   pHIDDesc == 0
	    || pHIDDesc->wReportDescriptorLength == 0)
	{
		ConfigurationError (FromUSBMouse);

		return FALSE;
	}
    //CLogger::Get ()->Write (FromUSBMouse, LogDebug, "HID descriptor");
	//debug_hexdump (pHIDDesc, sizeof *pHIDDesc, FromUSBMouse);

	m_usReportDescriptorLength = pHIDDesc->wReportDescriptorLength;
	m_pHIDReportDescriptor = new u8[m_usReportDescriptorLength];
	assert (m_pHIDReportDescriptor != 0);

	if (   GetHost ()->GetDescriptor (GetEndpoint0 (),
					  pHIDDesc->bReportDescriptorType, DESCRIPTOR_INDEX_DEFAULT,
					  m_pHIDReportDescriptor, m_usReportDescriptorLength,
					  REQUEST_IN | REQUEST_TO_INTERFACE, GetInterfaceNumber ())
	    != m_usReportDescriptorLength)
	{
		CLogger::Get ()->Write (FromUSBMouse, LogError, "Cannot get HID report descriptor");

		return FALSE;
	}
    //CLogger::Get ()->Write (FromUSBMouse, LogDebug, "Report descriptor");
    //debug_hexdump (m_pHIDReportDescriptor, m_usReportDescriptorLength, FromUSBMouse);

	if (!CUSBHIDDevice::Configure ())
	{
		CLogger::Get ()->Write (FromUSBMouse, LogError, "Cannot configure HID device");

		return FALSE;
	}

	m_pMouseDevice = new CMouseDevice;
	assert (m_pMouseDevice != 0);

	return StartRequest ();
}

void CUSBMouseDevice::ReportHandler (const u8 *pReport, unsigned nReportSize)
{
    //debug_hexdump (pReport, nReportSize, FromUSBMouse);

    if (   pReport != 0
	    && nReportSize == REPORT_SIZE)
	{
        //CLogger::Get ()->Write (FromUSBMouse, LogDebug, "%02X %3d %3d %3d %3d %3d %3d",
        //                        pReport[0], pReport[1], pReport[2], pReport[3], pReport[4], pReport[5], pReport[6]);

        // in boot protocol the 3 bytes are:
        // 0   1 - left button, 2 - right button, 4 - middle button
        // 1   X displacement (+/- 127 max)
        // 2   Y displacement (+/- 127 max)
        // in report protocol the 5 bytes are:
        // 0   0x01 constant?
        // 1   1 - left button, 2 - right button, 4 - middle button
        // 2   X displacement (+/- 127 max)
        // 3   Y displacement (+/- 127 max)
        // 4   1 - wheel up, -1 wheel down
		u8 ucHIDButtons = pReport[1];

		unsigned nButtons = 0;
		if (ucHIDButtons & USBHID_BUTTON1)
		{
			nButtons |= MOUSE_BUTTON_LEFT;
		}
		if (ucHIDButtons & USBHID_BUTTON2)
		{
			nButtons |= MOUSE_BUTTON_RIGHT;
		}
		if (ucHIDButtons & USBHID_BUTTON3)
		{
			nButtons |= MOUSE_BUTTON_MIDDLE;
		}

		if (m_pMouseDevice != 0)
		{
			m_pMouseDevice->ReportHandler (nButtons, (char) pReport[2],
						       (char) pReport[3], (char) pReport[4]);
		}
	}
}
