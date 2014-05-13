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

#include <Arduino.h>

#ifndef SAM3X8E_EEFC_INCLUDED
#define SAM3X8E_EEFC_INCLUDED


class SAM3X8E_EEFC {
public:
	SAM3X8E_EEFC(Efc *_efc, int start);
	void begin();
	boolean writePage(void *data, const void *flash);
	boolean writeData(void *data, uint32_t len, const void *flash);
	boolean getDescriptor(uint32_t *buff, int size);

	void *getStartAddress() { return start; }
	void *getEndAddress() { return start+size; }
	uint32_t getSize() { return size; }
	boolean containsAddress(const void *addr) { return (start<=addr) && (addr<start+size); }
	boolean isAligned(const void *addr) { return isAligned(reinterpret_cast<uint32_t>(addr)); }
	boolean isAligned(uint32_t addr) { return (addr & 0x03) == 0; }

	enum Commands {
		GET_FLASH_DESCRIPTOR,
		WRITE_PAGE,
		WRITE_PAGE_AND_LOCK,
		ERASE_PAGE_AND_WRITE_PAGE,
		ERASE_PAGE_AND_WRITE_PAGE_AND_LOCK,
		ERASE_ALL,
		SET_LOCK_BIT,
		CLEAR_LOCK_BIT,
		GET_LOCK_BIT,
		SET_GPNVM_BIT,
		CLEAR_GPNVM_BIT,
		GET_GPNVM_BIT,
		START_READ_UNIQUE_ID,
		STOP_READ_UNIQUE_ID,
		GET_CALIBRATION_BIT,
	};
	void command(Commands cmd) { command(cmd, 0); }
	void command(Commands cmd, uint16_t arg) { FCR(EEFC_FCR_FCMD(cmd) | EEFC_FCR_FARG(arg) | EEFC_FCR_FKEY(0x5A)); }
	boolean isBusy() { return (FSR() & EEFC_FSR_FRDY) == 0; }
	void waitReady() { while (isBusy()) ; }

	uint32_t getResult() { return FRR(); }
	void setFlashWaitStates(uint8_t v) { FMR_FWS(v); }

	enum Error {
		SUCCESS,
		COMMAND_ERROR,
		LOCK_ERROR,
		OUT_OF_RANGE_ERROR,
		ALIGNMENT_ERROR,
	};
	Error getError();

private:
	boolean hasInternalError;
	Error internalError;

	uint32_t pageSize;
	uint32_t size;
	uint8_t *start;

	// SAM3X low level registers access
	Efc *efc;
	void FMR(uint32_t v) { efc->EEFC_FMR = v; }
	void FMR_FWS(uint8_t v) { FMR((FMR() & ~EEFC_FMR_FWS_Msk) | EEFC_FMR_FWS(v)); }
	void FCR(uint32_t v) { efc->EEFC_FCR = v; }
	void FSR(uint32_t v) { efc->EEFC_FSR = v; }
	void FRR(uint32_t v) { efc->EEFC_FRR = v; }
	uint32_t FMR() { return efc->EEFC_FMR; }
	uint32_t FCR() { return efc->EEFC_FCR; }
	uint32_t FSR() { return efc->EEFC_FSR; }
	uint32_t FRR() { return efc->EEFC_FRR; }
};

extern SAM3X8E_EEFC Flash0;
extern SAM3X8E_EEFC Flash1;

#endif // SAM3X8E_EEFC_INCLUDED
