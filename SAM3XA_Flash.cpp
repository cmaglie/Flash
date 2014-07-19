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
#include <SAM3XA_Flash.h>




void SAM3XA_Flash::begin()
{
	for (int i=0; eefcs[i]!=NULL; i++)
		eefcs[i]->begin();
}

boolean SAM3XA_Flash::writePage(void *data, const void *flash)
{
	for (int i=0; eefcs[i]!=NULL; i++)
		if (eefcs[i]->containsAddress(flash))
			return eefcs[i]->writePage(data, flash);
	return false;
}

boolean SAM3XA_Flash::writeData(void *data, uint32_t len, const void *_flash)
{
	const uint8_t *flash = reinterpret_cast<const uint8_t *>(_flash);
	const uint8_t *end; // formula has been moved down...as len can be updated
	Serial.println("Writing Data");
	
	while (true) {
		int i;
		for (i=0; eefcs[i]!=NULL; i++)
			if (eefcs[i]->containsAddress(flash))
			{
				Serial.println("Address Found");
				break;
			}
		// No flash controllers found for the requested area
		if (eefcs[i]==NULL)
		{
			Serial.println("NO FLASH CTRL");
			return false;
		}
		
		// M.Hefny: align length by taking extra unnecessary bytes.
		if (!eefcs[i]->isAligned(len))
		{

			len = len + (0x4- (len & 0x03)); 	// Global Works		
			Serial.println("to ");
			Serial.println(len);			
		}
		end = flash + len; // new location.
		
		if (eefcs[i]->containsAddress(end)) {
  			// Write last block
			 Serial.println((int)data,HEX);
			 Serial.println(len);
			 Serial.println((int)flash,HEX);
			 Serial.println((int)end,HEX);
			return eefcs[i]->writeData(data, len, flash);
		}
		
		uint8_t *stop = reinterpret_cast<uint8_t *>(eefcs[i]->getEndAddress());
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
