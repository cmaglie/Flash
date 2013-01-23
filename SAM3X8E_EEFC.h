
#include <Arduino.h>

#ifndef SAM3X8E_EEFC_INCLUDED
#define SAM3X8E_EEFC_INCLUDED

class SAM3X8E_EEFC {
public:
	SAM3X8E_EEFC(Efc *_efc, int start);
	void begin();
	boolean writePage(uint8_t *data, const uint8_t *flash);
	boolean writeData(uint8_t *data, uint32_t len, const uint8_t *flash);
	boolean getDescriptor(uint32_t *buff, int size);

	uint8_t *getStartAddress() { return start; }
	uint8_t *getEndAddress() { return start+size; }
	uint32_t getSize() { return size; }
	boolean containsAddress(const uint8_t *addr) { return (start<=addr) && (addr<start+size); }
	boolean isAligned(const uint8_t *addr) { return isAligned(reinterpret_cast<uint32_t>(addr)); }
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
