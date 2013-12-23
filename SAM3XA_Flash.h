
#include <SAM3X8E_EEFC.h>

#ifndef SAM3XA_FLASH_INCLUDED
#define SAM3XA_FLASH_INCLUDED

class SAM3XA_Flash {
public:
	SAM3XA_Flash(SAM3X8E_EEFC *_eefcs[]) : eefcs(_eefcs) { };

	void begin();
	boolean writePage(void *data, const void *flash);
	boolean writeData(void *data, uint32_t len, const void *flash);

private:
	SAM3X8E_EEFC **eefcs;
};

extern SAM3XA_Flash Flash;

#endif // SAM3XA_FLASH_INCLUDED
