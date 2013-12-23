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
