/*
  Flash Example

  Counts the number of times the sketch is run.
  The counter is stored in flash memory.

  WARNING: Flash memory has a limited number of erase/write
  cycles (usually 10K). Running this sketch or writing to
  the flash will reduce its expected lifetime depending on
  the number of write cycles performed.

  This example code is in the public domain.
 */

#include <Flash.h>

// Flash space must be declared as "const"
// so the data is allocated into flash memory
const int flashBuffer[] = { 0 };

// We must pass through another auxiliary memory pointer
// to read the content of reserved flash memory
const int *flash = flashBuffer;

// if we use the original pointer doing something like
//
//     Serial.print(flashBuffer[0]);
//
// the compiler is able to optimize the memory read because
// it considers the flashBuffer area as constant, so the
// call is optimized as
//
//     Serial.print(0);
//
// and the flash memory is not read at all.

void setup() {

  // Read data from non volatile space
  int count = flash[0];

  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.print("You launched this sketch ");
  Serial.print(count);
  Serial.println(" times!");

  if (count > 3) {
    Serial.println("You should not write too much into Flash memory, this is not an EEPROM!");
  }
  if (count > 5) {
    Serial.println("NO, really, I'm not joking STOP DOING THAT, you can wear out the Flash");
    Serial.println("because it has a limited number of writes cycles (around 10K)");
  }

  // to update flash memory you must prepare a temporary buffer
  // of the same size.
  int tempBuffer[1];
  tempBuffer[0] = count + 1;

  Flash.begin();
  // Write the flash memory with the content of the tempBuffer
  bool result = Flash.writeData(tempBuffer, sizeof(tempBuffer), flash);
  if (result == true) {
    Serial.println("Counter updated");
  }  else {
    Serial.println("Failed to update counter");
  }
}

void loop() {
}

