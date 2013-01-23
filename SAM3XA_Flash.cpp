
#include <SAM3XA_Flash.h>

void SAM3XA_Flash::begin()
{
	for (int i=0; eefcs[i]!=NULL; i++)
		eefcs[i]->begin();
}

boolean SAM3XA_Flash::writePage(uint8_t *data, const uint8_t *flash)
{
	for (int i=0; eefcs[i]!=NULL; i++)
		if (eefcs[i]->containsAddress(flash))
			return eefcs[i]->writePage(data, flash);
	return false;
}

boolean SAM3XA_Flash::writeData(uint8_t *data, uint32_t len, const uint8_t *flash)
{
	uint8_t *end = const_cast<uint8_t *>(flash) + len;

	while (true) {
		int i;
		for (i=0; eefcs[i]!=NULL; i++)
			if (eefcs[i]->containsAddress(flash))
				break;

		// No flash controllers found for the requested area
		if (eefcs[i]==NULL)
			return false;

		if (eefcs[i]->containsAddress(end)) {
  			// Write last block
			return eefcs[i]->writeData(data, len, flash);
		}

		uint8_t *stop = eefcs[i]->getEndAddress();
		uint32_t size = stop - flash;
		boolean res = eefcs[i]->writeData(data, size, flash);
		if (!res)
			return false;
		data += size;
		flash += size;
		len -= size;
	}
}

static SAM3X8E_EEFC *df[] = {
		&Flash0,
#if defined (EFC1)
		&Flash1,
#endif
		NULL };

SAM3XA_Flash Flash(df);
