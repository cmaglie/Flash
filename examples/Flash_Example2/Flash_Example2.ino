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

#define ARDUINO_ARCH_SAM
#define ARDUINO_DUE
#include "Flash.h"

enum box {
  BOXARM,
 
    BOXANGLE,
    BOXHORIZON,
    BOXBARO,
  #ifdef VARIOMETER
    BOXVARIO,
  #endif
    BOXMAG,
    BOXHEADFREE,
    BOXHEADADJ, // acquire heading for HEADFREE mode
  #if defined(SERVO_TILT) || defined(GIMBAL)  || defined(SERVO_MIX_TILT)
    BOXCAMSTAB,
  #endif
  #if defined(CAMTRIG)
    BOXCAMTRIG,
  #endif
  #if GPS
    BOXGPSHOME,
    BOXGPSHOLD,
  #endif
  #if defined(FIXEDWING) || defined(HELICOPTER)
    BOXPASSTHRU,
  #endif
  #if defined(BUZZER)
    BOXBEEPERON,
  #endif
  #if defined(LED_FLASHER)
    BOXLEDMAX, // we want maximum illumination
    BOXLEDLOW, // low/no lights
  #endif
  #if defined(LANDING_LIGHTS_DDR)
    BOXLLIGHTS, // enable landing lights at any altitude
  #endif
  #ifdef INFLIGHT_ACC_CALIBRATION
    BOXCALIB,
  #endif
  #ifdef GOVERNOR_P
    BOXGOV,
  #endif
  #ifdef OSD_SWITCH
    BOXOSD,
  #endif
  CHECKBOXITEMS
};

enum pid {
  PIDROLL,
  PIDPITCH,
  PIDYAW,
  PIDALT,
  PIDPOS,
  PIDPOSR,
  PIDNAVR,
  PIDLEVEL,
  PIDMAG,
  PIDVEL,     // not used currently
  PIDITEMS
};
 

typedef struct {
  uint8_t currentSet;
  int16_t accZero[3];
  int16_t magZero[3];
  uint16_t flashsum;
  uint8_t checksum;      // MUST BE ON LAST POSITION OF STRUCTURE !
} global_conf_t;

struct pid_ {
  uint8_t P8;
  uint8_t I8;
  uint8_t D8;
};

struct servo_conf_ {  // this is a generic way to configure a servo, every multi type with a servo should use it
  int16_t min;        // minimum value, must be more than 1020 with the current implementation
  int16_t max;        // maximum value, must be less than 2000 with the current implementation
  int16_t middle;     // default should be 1500
  int8_t  rate;       // range [-100;+100] ; can be used to ajust a rate 0-100% and a direction
};

typedef struct {
  pid_    pid[PIDITEMS];
  uint8_t rcRate8;
  uint8_t rcExpo8;
  uint8_t rollPitchRate;
  uint8_t yawRate;
  uint8_t dynThrPID;
  uint8_t thrMid8;
  uint8_t thrExpo8;
  int16_t angleTrim[2];
  uint16_t activate[CHECKBOXITEMS];
  uint8_t powerTrigger1;
  #if MAG
    int16_t mag_declination;
  #endif
  servo_conf_ servoConf[8];
  #if defined(GYRO_SMOOTHING)
    uint8_t Smoothing[3];
  #endif
  #if defined (FAILSAFE)
    int16_t failsafe_throttle;
  #endif
  #ifdef VBAT
    uint8_t vbatscale;
    uint8_t vbatlevel_warn1;
    uint8_t vbatlevel_warn2;
    uint8_t vbatlevel_crit;
  #endif
  #ifdef POWERMETER
    uint8_t pint2ma;
  #endif
  #ifdef POWERMETER_HARD
    uint16_t psensornull;
  #endif
  #ifdef MMGYRO
    uint8_t mmgyro;
  #endif
  #ifdef ARMEDTIMEWARNING
    uint16_t armedtimewarning;
  #endif
  int16_t minthrottle;
  #ifdef GOVERNOR_P
   int16_t governorP;
   int16_t governorD;
  #endif
  uint8_t  checksum;      // MUST BE ON LAST POSITION OF CONF STRUCTURE !
} conf_t;


global_conf_t global_conf;

conf_t conf;


const uint8_t Flashglobal_conf[sizeof(global_conf)+32] = { 0 };   //32 extra free space for padding as safe guard for the next data flash data, as len is updated to be multiple of 4 when saving. i.e. if length 14 it becomes 16
const uint8_t *pFlashglobal_conf = Flashglobal_conf;
const uint8_t Flash_conf[sizeof(conf)*4 + 16] = { 0 };
const uint8_t *pFlash_conf = Flash_conf;
void eeprom_read_block (void *__dst, const void *__src, unsigned int __n)
{
	int i=0;
	char before,after;
	for (i=0;i<__n; i++)
	{
		Serial.print(i);
		Serial.write(" : ");
		Serial.print(((char*)__dst)[i],HEX);
		Serial.write(" - ");
		Serial.print(((char*)__src)[i],HEX);
		Serial.write ("\r\n");
		delayMicroseconds(200);
		((char*)__dst)[i]= ((char*)__src)[i];
	}

} 

void eeprom_write_byte (uint8_t *__p, uint8_t __value)
{
	*__p = __value;
}
 
void eeprom_write_word (uint16_t *__p, uint16_t __value)
{
	*__p = (uint16_t)__value;
}  

void eeprom_write_block ( void *__src, void *__dst, unsigned int __n)
{
	Flash.begin();
	//Write the flash memory with the content of the tempBuffer
	bool result =  Flash.writeData(__src, __n, __dst);
   
	if (result)
	{
	Serial.println("Saved");
	}
	else
	{
	Serial.println("failed");
	}
	
}


void PressAnykey()
{
	Serial.println ("Press Any Key");
	while (Serial.available() <= 0)
	{
	}
	Serial.read();
}

void setup() {

  // Read data from non volatile space
 
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("You launched this sketch ....  ");
  PressAnykey();
 
  char *b = (char *) &global_conf; 
  int count;
 Serial.print("Updating global_conf");
 for (count =0; count < sizeof(global_conf); ++ count)
 {
	*b=count;
	Serial.print (count);
	Serial.write(" :");
	Serial.println (*b,HEX);
	
	b++;
 }
 
  PressAnykey();
  Serial.println("Writing global_conf");
  eeprom_write_block((void*)&global_conf, (void*)pFlashglobal_conf, sizeof(global_conf));
  
  
  PressAnykey();
  Serial.println("Reading global_conf");
  eeprom_read_block((void*)&global_conf, (void*)pFlashglobal_conf, sizeof(global_conf));
  
  
  
  b = (char *) &conf; 
 
 PressAnykey();
 Serial.print("Updating global_conf");
 for (count =0; count < sizeof(conf); ++ count)
 {
	*b=count;
	Serial.print (count);
	Serial.write(" :");
	Serial.println (*b,HEX);
	
	b++;
 }
 
  PressAnykey();
  Serial.println("Writing conf");
  eeprom_write_block((void*)&conf, (void*)(pFlash_conf ), sizeof(conf));
 
  
  
  PressAnykey();
  Serial.println("Reading conf");
  eeprom_read_block((void*)&conf, (void*)(pFlash_conf), sizeof(conf));
  
  
}

void loop() {
}

