/*
  Copyright (c) 2013 Cristian Maglie <c.maglie@bug.st>
  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/

#include <SAM3X8E_EEFC.h>
#include <Arduino.h>

SAM3X8E_EEFC::SAM3X8E_EEFC(Efc *_efc, int _start) :
	efc(_efc), start(reinterpret_cast<uint8_t *>(_start)), hasInternalError(false)
{
	// Empty
}

void SAM3X8E_EEFC::begin()
{
	// Retrieve flash size and page size directly from EEFC
	uint32_t buff[8];
	getDescriptor(buff, 8);
	size = buff[1];
	pageSize = buff[2];

	// Set Flash Wait State to 6
	// (SAM3X bug, see datasheet errata for details)
	setFlashWaitStates(6);
}

__attribute__ ((long_call, section (".ramfunc")))
boolean SAM3X8E_EEFC::writePage(void *data, const void *_flash)
{
	const uint8_t *flash = reinterpret_cast<const uint8_t *>(_flash);

	// Range check
	if (!containsAddress(flash)) {
		hasInternalError = true;
		internalError = OUT_OF_RANGE_ERROR;
		return false;
	}

	// 32-bit Align check
	if (!isAligned(flash) || !isAligned(data)) {
		hasInternalError = true;
		internalError = ALIGNMENT_ERROR;
		return false;
	}

	hasInternalError = false;

	// Calculate page
	uint32_t page = (flash - start) / pageSize;

	// Run page write
	// (page buffering should be done 32bit at a time, otherwise
	// strange things happens...)
	uint32_t *flashP = reinterpret_cast<uint32_t *>(const_cast<uint8_t *>(flash));
	uint32_t *dataP  = reinterpret_cast<uint32_t *>(data);
	for (int i=0; i<pageSize/4; i++)
		flashP[i] = dataP[i];
	__disable_irq();
	command(ERASE_PAGE_AND_WRITE_PAGE, page);
	waitReady();
	__enable_irq();

	if (getError() != SUCCESS)
		return false;
	return true;
}

__attribute__ ((long_call, section (".ramfunc")))
boolean SAM3X8E_EEFC::writeData(void *data, uint32_t len, const void *_flash)
{
	const uint8_t *flash = reinterpret_cast<const uint8_t *>(_flash);

	// Range check
	if (!containsAddress(flash)) {
		hasInternalError = true;
		internalError = OUT_OF_RANGE_ERROR;
		return false;
	}

	// 32-bit Align check
	if (!isAligned(flash) || !isAligned(data) || !isAligned(len)) {
		hasInternalError = true;
		internalError = ALIGNMENT_ERROR;
		return false;
	}

	// Address calculations
	uint8_t *startFlash  =  const_cast<uint8_t *>(flash);
	uint32_t startPage   = (flash - start) / pageSize;
	uint32_t startOffset = (flash - start) % pageSize;
	// uint8_t *endFlash    =  const_cast<uint8_t *>(flash + len - 1);
	uint32_t endPage     = (flash + len - start - 1) / pageSize;
	uint32_t endOffset   = (flash + len - start - 1) % pageSize;

	// Serial.print("startFlash ="); Serial.println((int)startFlash,HEX);
	// Serial.print("startPage  ="); Serial.println(startPage);
	// Serial.print("startOffset="); Serial.println(startOffset);
	// Serial.print("  endFlash ="); Serial.println((int)endFlash,HEX);
	// Serial.print("  endPage  ="); Serial.println(endPage);
	// Serial.print("  endOffset="); Serial.println(endOffset);

	// Run page write
	// (page buffering should be done 32bit at a time,
	// otherwise strange things happens...)
	volatile uint32_t *flashP  = reinterpret_cast<uint32_t *>(start + (startPage * pageSize));
	volatile uint32_t *dataP   = reinterpret_cast<uint32_t *>(data);
	for (int page=startPage; page<=endPage; page++)
	{
		// Serial.print("Writing page ");
		// Serial.println(page);

		int start = (page==startPage) ? startOffset/4 : 0;
		int end   = (page==endPage)   ? endOffset/4   : pageSize/4;

		// Buffer pre
		for (int off=0; off<=start; off++) {
			// Serial.print("Pre  BUFF: ");
			// Serial.println((int)(flashP+off), HEX);
			flashP[off] = flashP[off];
		}
		// Buffer post
		for (int off=end; off<pageSize/4; off++) {
			// Serial.print("Post BUFF: ");
			// Serial.println((int)(flashP+off), HEX);
			flashP[off] = flashP[off];
		}
		// Copy data over
		for (int off=start; off<=end; off++, dataP++) {
			// Serial.print("Data     : ");
			// Serial.print((int)(flashP+off), HEX);
			// Serial.print(" ");
			// Serial.println(*dataP, HEX);
			flashP[off] = *dataP;
		}

		flashP += pageSize / 4;

		__disable_irq();
		command(ERASE_PAGE_AND_WRITE_PAGE, page);
		waitReady();
		__enable_irq();
		if (getError() != SUCCESS)
			return false;
	}

	return true;
}

boolean SAM3X8E_EEFC::getDescriptor(uint32_t *buff, int size) {
	command(GET_FLASH_DESCRIPTOR);
	waitReady();
	if (getError() != SUCCESS)
		return false;
	for (int i=0; i<size; i++)
		buff[i] = getResult();
	return true;
}

SAM3X8E_EEFC::Error SAM3X8E_EEFC::getError() {
	if (hasInternalError) {
		hasInternalError = false;
		return internalError;
	}
	uint32_t status = FSR();
	if ((status & EEFC_FSR_FCMDE) == EEFC_FSR_FCMDE)
		return COMMAND_ERROR;
	if ((status & EEFC_FSR_FLOCKE) == EEFC_FSR_FLOCKE)
		return LOCK_ERROR;
	return SUCCESS;
}

SAM3X8E_EEFC Flash0(EFC0, IFLASH0_ADDR);
#if defined (EFC1)
SAM3X8E_EEFC Flash1(EFC1, IFLASH1_ADDR);
#endif


