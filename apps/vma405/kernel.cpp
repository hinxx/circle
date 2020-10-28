//
// kernel.cpp
//
// MFRC522 - Mifare RFID reader/writer
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
#include <circle/timer.h>

#include "MFRC522.h"

#define SPI_MASTER_DEVICE	0		// 0, 4, 5, 6 on Raspberry Pi 4; 0 otherwise

#define SPI_CLOCK_SPEED		100000		// Hz
#define SPI_CPOL		0
#define SPI_CPHA		0

#define SPI_CHIP_SELECT		0		// 0 or 1, or 2 (for SPI1)

#define TEST_DATA_LENGTH	128		// number of data bytes transfered

static const char FromKernel[] = "kernel";

CKernel::CKernel (void)
:	m_Screen (m_Options.GetWidth (), m_Options.GetHeight ()),
	m_Timer (&m_Interrupt),
	m_Logger (m_Options.GetLogLevel (), &m_Timer)
//#ifndef USE_SPI_MASTER_AUX
//	m_SPIMaster (SPI_CLOCK_SPEED, SPI_CPOL, SPI_CPHA, SPI_MASTER_DEVICE)
//#else
//	m_SPIMaster (SPI_CLOCK_SPEED)
//#endif
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

//	if (bOK)
//	{
//		bOK = m_SPIMaster.Initialize ();
//	}

	return bOK;
}

TShutdownMode CKernel::Run (void)
{
	m_Logger.Write (FromKernel, LogNotice, "Compile time: " __DATE__ " " __TIME__);

	m_Logger.Write (FromKernel, LogNotice, "VMA405 RFID over SPI");

//	u8 TxData[TEST_DATA_LENGTH];
//	u8 RxBuffer[TEST_DATA_LENGTH];
//	for (unsigned i = 0; i < TEST_DATA_LENGTH; i++)
//	{
//		TxData[i] = (u8) i;
//		RxBuffer[i] = 0x55;
//	}

//	if (m_SPIMaster.WriteRead (SPI_CHIP_SELECT, TxData, RxBuffer, TEST_DATA_LENGTH) != TEST_DATA_LENGTH)
//	{
//		CLogger::Get ()->Write (FromKernel, LogPanic, "SPI write error");
//	}

//	CLogger::Get ()->Write (FromKernel, LogNotice, "%u bytes transfered", TEST_DATA_LENGTH);

//#ifndef NDEBUG
//	CLogger::Get ()->Write (FromKernel, LogDebug, "Dumping received data:");

//	debug_hexdump (RxBuffer, TEST_DATA_LENGTH, FromKernel);
//#endif
/*
    u8 TxData[TEST_DATA_LENGTH];
	u8 RxBuffer[TEST_DATA_LENGTH];
	for (unsigned i = 0; i < TEST_DATA_LENGTH; i++)
	{
		TxData[i] = 0;
		RxBuffer[i] = 0;
	}

    CGPIOPin m_RST;
    m_RST.AssignPin(24);
    m_RST.SetMode(GPIOModeOutput, TRUE);
    m_RST.Write(HIGH);
    CLogger::Get()->Write(FromKernel, LogDebug, "doing hard reset ..");
    m_RST.Write(HIGH);
    m_RST.Write(LOW);
    CTimer::SimpleMsDelay(2);
    m_RST.Write(HIGH);
    CTimer::SimpleMsDelay(50);

    //byte data[2] = {0x37 << 1, 0};
    TxData[0] = 0x80 | (0x37 << 1);
    if (m_SPIMaster.WriteRead(SPI_CHIP_SELECT, TxData, RxBuffer, 2) != 2) {
        CLogger::Get()->Write(FromKernel, LogPanic, "SPI write error");
    }
    CLogger::Get()->Write(FromKernel, LogDebug, "SPI read %X %X", RxBuffer[0], RxBuffer[1]);
*/

    MFRC522 mfrc522;
    // Init MFRC522
    mfrc522.PCD_Init(8, 24);
    CLogger::Get()->Write(FromKernel, LogDebug, "PCD init done!");

    // Optional delay. Some board do need more time after init to be ready, see Readme
	CTimer::SimpleMsDelay(4);
	mfrc522.PCD_DumpVersionToSerial();	// Show details of PCD - MFRC522 Card Reader details


    CLogger::Get ()->Write (FromKernel, LogDebug, "Scan PICC to see UID, SAK, type, sector data...");
    // Reset the loop if no new card present on the sensor/reader. This saves the entire process when idle.
	if (! mfrc522.PICC_IsNewCardPresent()) {
		CLogger::Get ()->Write (FromKernel, LogError, "PICC_IsNewCardPresent() failed!");
	}

	// Select one of the cards
	if (! mfrc522.PICC_ReadCardSerial()) {
        CLogger::Get ()->Write (FromKernel, LogError, "PICC_ReadCardSerial() failed!");
	}
    // debug_hexdump(mfrc522.uid.uidByte, mfrc522.uid.size, FromKernel);

    // Dump debug info about the card; PICC_HaltA() is automatically called
	mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

    CLogger::Get()->Write(FromKernel, LogDebug, "Rebooting..");
    return ShutdownReboot;
}
