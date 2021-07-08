//- -----------------------------------------------------------------------------------------------------------------------
// AskSin++
// 2016-10-31 papa Creative Commons - http://creativecommons.org/licenses/by-nc-sa/3.0/de/
//- -----------------------------------------------------------------------------------------------------------------------

// define this to read the device id, serial and device type from bootloader section
// #define USE_OTA_BOOTLOADER
//#define NDEBUG
#define HIDE_IGNORE_MSG

#define EI_NOTEXTERNAL
#include <EnableInterrupt.h>
#include <AskSinPP.h>
#include <LowPower.h>

#include <Wire.h>
 #include <sensors/Opt3001.h>

#include <MultiChannelDevice.h>
#include <Motion.h>

// we use a Pro Mini
// Arduino pin for the LED
// D4 == PIN 4 on Pro Mini
#define LED1_PIN A0
#define LED2_PIN A1
// Arduino pin for the config button
// B0 == PIN 8 on Pro Mini
#define CONFIG_BUTTON_PIN 8
// Arduino pin for the PIR
// A0 == PIN 14 on Pro Mini
#define PIR_PIN A2

// number of available peers per channel
#define PEERS_PER_CHANNEL 6

// all library classes are placed in the namespace 'as'
using namespace as;

// define all device properties
const struct DeviceInfo PROGMEM devinfo = {
  {0x4A, 0xDE, 0x8E},       // Device ID
  "HB96868549",           // Device Serial
//  {0xB4, 0xB0, 0xF},       // Device ID
//  "HB91706977",           // Device Serial
  {0x00,0x4a},              // Device Model
  0x16,                     // Firmware Version
  as::DeviceType::MotionDetector, // Device Type
  {0x01,0x00}               // Info Bytes
};

/**
 * Configure the used hardware
 */
typedef AvrSPI<10,11,12,13> SPIType;
typedef Radio<SPIType,2> RadioType;
//typedef StatusLed<LED_PIN> LedType;
typedef DualStatusLed<LED2_PIN, LED1_PIN> LedType;
typedef AskSin<LedType,BatterySensor,RadioType> BaseHal;
class Hal : public BaseHal {
public:
  void init (const HMID& id) {
    BaseHal::init(id);
    // set low voltage to 2.2V
    // measure battery every 1h
    battery.init(seconds2ticks(60UL*60),sysclock);
    battery.low(22);
    battery.critical(19);
  }
} hal;


/**
* this is an overlay to the MotionChannel class; 
* it adds some functionallity to change the sensitivity of the
* AM612 motion detector with the help of an AD5247 100k I2C poti
*/
template <class HalType, int PeerCount, class List0Type = List0, class BrightnessSensor = Brightness>
class MyMotionChannel : public MotionChannel<HalType, PeerCount, List0Type, BrightnessSensor > { //Channel<HalType, MotionList1, EmptyList, DefList4, PeerCount, List0Type>, public Alarm {
  uint8_t _poti_present = false;
  int8_t readVal() {
    uint8_t	buf;
    Wire.requestFrom(0x2e, 2);

    uint8_t counter = 0;
    while (Wire.available() < 2) {
      counter++;
      delay(10);
      if (counter > 250) return -1;
    }

    Wire.readBytes(&buf, 1);
    return buf;
  }  
  int8_t writeVal(uint8_t val) {
    Wire.beginTransmission(0x2e);
    Wire.write(val);
    return ( Wire.endTransmission());
  }

public:
  typedef MotionChannel<HalType, PeerCount, List0Type, BrightnessSensor > MotionChannelType;

  MyMotionChannel() : MotionChannel<HalType, PeerCount, List0Type, BrightnessSensor > () {}

  virtual ~MyMotionChannel() {}

  void setup(Device<HalType, List0Type>* dev, uint8_t number, uint16_t addr) {
    // check for and init the AD5247 
    Wire.begin();
    Wire.beginTransmission(0x2E);
    uint8_t error = Wire.endTransmission();
    if (error == 0) {
      DPRINTLN(F("AD5247 poti found"));
      _poti_present = true;
    }
    MotionChannelType::setup(dev, number, addr);
  }

  void motionDetected() {
    //DPRINT('.');
    MotionChannelType::motionDetected();
  }

  void configChanged() {
    DPRINTLN(F("config changed"));
    if (!_poti_present) return;
    // write the sensitivity settings into the AD5247
    //writeVal(0);
  }




};



#if defined(_TSL2561_H_)
typedef MotionChannel<Hal,PEERS_PER_CHANNEL,List0,Tsl2561<TSL2561_ADDR_LOW> > MChannel;
#elif defined(__SENSORS_BH1750_h__)
typedef MyMotionChannel<Hal, PEERS_PER_CHANNEL, List0, Bh1750<> > MChannel;
#elif defined(__SENSORS_OPT3001_h__)
typedef MyMotionChannel<Hal, PEERS_PER_CHANNEL, List0, Opt3001<> > MChannel;
//typedef MotionChannel<Hal, PEERS_PER_CHANNEL, List0, Opt3001<> > MChannel;
#else
typedef MotionChannel<Hal,PEERS_PER_CHANNEL,List0> MChannel;
#endif

typedef MultiChannelDevice<Hal,MChannel,1> MotionType;
MotionType sdev(devinfo,0x20);

ConfigButton<MotionType> cfgBtn(sdev);

void setup () {
  DINIT(57600,ASKSIN_PLUS_PLUS_IDENTIFIER);
  sdev.init(hal);
  buttonISR(cfgBtn,CONFIG_BUTTON_PIN);
  motionISR(sdev,1,PIR_PIN);
  sdev.initDone();

  /*// scan i2c bus for devices
  DPRINTLN(F("I2C scan..."));
  for (uint8_t i = 1; i < 127; i++) {
    Wire.beginTransmission(i);
    uint8_t error = Wire.endTransmission();

    if (error == 0) {
      DPRINT(F("device at 0x")); DHEXLN(i);
    }
    else if (error == 4) {
      DPRINT(F("error at 0x")); DHEXLN(i);
    }
  }
  DPRINTLN(F("done"));
  // ------------------------- */
}

void loop() {
  //static uint32_t temp_millis = 0;
  //if (millis() - temp_millis > 2000) {
  //  temp_millis = millis();
  //  sdev.channel(1).status();
  //}
  bool worked = hal.runready();
  bool poll = sdev.pollRadio();
  if( worked == false && poll == false ) {
    // deep discharge protection
    // if we drop below critical battery level - switch off all and sleep forever
    if( hal.battery.critical() ) {
      // this call will never return
			DPRINTLN(F("sleep forever"));
			delay(500);
      hal.activity.sleepForever(hal);
    }
    // if nothing to do - go sleep
    hal.activity.savePower<Sleep<>>(hal);
  }
}
