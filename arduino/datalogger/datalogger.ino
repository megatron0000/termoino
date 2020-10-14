// ██╗███╗   ██╗ ██████╗██╗     ██╗   ██╗██████╗ ███████╗███████╗
// ██║████╗  ██║██╔════╝██║     ██║   ██║██╔══██╗██╔════╝██╔════╝
// ██║██╔██╗ ██║██║     ██║     ██║   ██║██║  ██║█████╗  ███████╗
// ██║██║╚██╗██║██║     ██║     ██║   ██║██║  ██║██╔══╝  ╚════██║
// ██║██║ ╚████║╚██████╗███████╗╚██████╔╝██████╔╝███████╗███████║
// ╚═╝╚═╝  ╚═══╝ ╚═════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝╚══════╝

#include <avr/sleep.h>

#include <Adafruit_Sensor.h>
#include <DHT_U.h>
#include <DHT.h>

#include <TimeLib.h>
#include <Time.h>

#include <DS3232RTC.h>
#include <AT24Cxx.h>

//#include <SPI.h>
//#include <SD.h>

//  ██████╗ ██████╗ ███╗   ███╗███╗   ███╗██╗   ██╗███╗   ██╗██╗ ██████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
// ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██║   ██║████╗  ██║██║██╔════╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║     ██║   ██║██╔████╔██║██╔████╔██║██║   ██║██╔██╗ ██║██║██║     ███████║   ██║   ██║██║   ██║██╔██╗ ██║
// ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██║   ██║██║╚██╗██║██║██║     ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
// ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║╚██████╗██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
//  ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

// ██╗███╗   ██╗██████╗  ██████╗ ██╗   ██╗███╗   ██╗██████╗
// ██║████╗  ██║██╔══██╗██╔═══██╗██║   ██║████╗  ██║██╔══██╗
// ██║██╔██╗ ██║██████╔╝██║   ██║██║   ██║██╔██╗ ██║██║  ██║
// ██║██║╚██╗██║██╔══██╗██║   ██║██║   ██║██║╚██╗██║██║  ██║
// ██║██║ ╚████║██████╔╝╚██████╔╝╚██████╔╝██║ ╚████║██████╔╝
// ╚═╝╚═╝  ╚═══╝╚═════╝  ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚═════╝

// These must be kept in sync: computer client code <=> arduino code <=> esp32 code

//
// ─── COMPUTER ───────────────────────────────────────────────────────────────────
//

#define COMPUTER_CONNECTED 0
#define LOG_DATA_REQUEST 1
#define LOG_MODE_REQUEST 2
#define DELETE_DATA_REQUEST 3
#define CLOSE_REQUEST 4
#define KEEPALIVE_REQUEST 5
#define USELESS_REQUEST 6
#define RECORD_COUNT_REQUEST 7
#define REALTIME_MEASUREMENT_REQUEST 8
#define TEST_WHOLE_EEPROM_REQUEST 9

// ────────────────────────────────────────────────────────────────────────────────

//
// ─── ESP 32 ─────────────────────────────────────────────────────────────────────
//

#define ESP32_CONNECTED 10
#define TEMPERATURE_HUMIDITY_NOW_REQUEST 11
#define ARDUINO_OPERATION_MODE_REQUEST 12

// ────────────────────────────────────────────────────────────────────────────────

//  ██████╗ ██████╗ ███╗   ███╗███╗   ███╗██╗   ██╗███╗   ██╗██╗ ██████╗ █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
// ██╔════╝██╔═══██╗████╗ ████║████╗ ████║██║   ██║████╗  ██║██║██╔════╝██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║     ██║   ██║██╔████╔██║██╔████╔██║██║   ██║██╔██╗ ██║██║██║     ███████║   ██║   ██║██║   ██║██╔██╗ ██║
// ██║     ██║   ██║██║╚██╔╝██║██║╚██╔╝██║██║   ██║██║╚██╗██║██║██║     ██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
// ╚██████╗╚██████╔╝██║ ╚═╝ ██║██║ ╚═╝ ██║╚██████╔╝██║ ╚████║██║╚██████╗██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
//  ╚═════╝ ╚═════╝ ╚═╝     ╚═╝╚═╝     ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚═╝ ╚═════╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

//  ██████╗ ██╗   ██╗████████╗██████╗  ██████╗ ██╗   ██╗███╗   ██╗██████╗
// ██╔═══██╗██║   ██║╚══██╔══╝██╔══██╗██╔═══██╗██║   ██║████╗  ██║██╔══██╗
// ██║   ██║██║   ██║   ██║   ██████╔╝██║   ██║██║   ██║██╔██╗ ██║██║  ██║
// ██║   ██║██║   ██║   ██║   ██╔══██╗██║   ██║██║   ██║██║╚██╗██║██║  ██║
// ╚██████╔╝╚██████╔╝   ██║   ██████╔╝╚██████╔╝╚██████╔╝██║ ╚████║██████╔╝
//  ╚═════╝  ╚═════╝    ╚═╝   ╚═════╝  ╚═════╝  ╚═════╝ ╚═╝  ╚═══╝╚═════╝

// These must be kept in sync: computer client code <=> arduino code <=> esp32 code

#define ARDUINO_CONNECTED "arduino connected"
#define LOG_DATA_START "log data start"
#define LOG_DATA_END "log data end"
#define CLOSE_ACKNOWLEDGE "close acknowledge"
#define KEEPALIVE_ACKNOWLEDGE "keepalive acknowledge"
#define DELETE_DATA_ACKNOWLEDGE "delete data acknowledge"

//  ██████╗ ██████╗ ███████╗██████╗  █████╗ ████████╗██╗ ██████╗ ███╗   ██╗
// ██╔═══██╗██╔══██╗██╔════╝██╔══██╗██╔══██╗╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║   ██║██████╔╝█████╗  ██████╔╝███████║   ██║   ██║██║   ██║██╔██╗ ██║
// ██║   ██║██╔═══╝ ██╔══╝  ██╔══██╗██╔══██║   ██║   ██║██║   ██║██║╚██╗██║
// ╚██████╔╝██║     ███████╗██║  ██║██║  ██║   ██║   ██║╚██████╔╝██║ ╚████║
//  ╚═════╝ ╚═╝     ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

// ███╗   ███╗ ██████╗ ██████╗ ███████╗███████╗
// ████╗ ████║██╔═══██╗██╔══██╗██╔════╝██╔════╝
// ██╔████╔██║██║   ██║██║  ██║█████╗  ███████╗
// ██║╚██╔╝██║██║   ██║██║  ██║██╔══╝  ╚════██║
// ██║ ╚═╝ ██║╚██████╔╝██████╔╝███████╗███████║
// ╚═╝     ╚═╝ ╚═════╝ ╚═════╝ ╚══════╝╚══════╝

#define STANDALONE_MODE 0
#define COMPUTER_MODE 1
#define ESP32_MODE 2
#define UNKNOWN_MODE 3

//  ██████╗ ██╗      ██████╗ ██████╗  █████╗ ██╗     ███████╗
// ██╔════╝ ██║     ██╔═══██╗██╔══██╗██╔══██╗██║     ██╔════╝
// ██║  ███╗██║     ██║   ██║██████╔╝███████║██║     ███████╗
// ██║   ██║██║     ██║   ██║██╔══██╗██╔══██║██║     ╚════██║
// ╚██████╔╝███████╗╚██████╔╝██████╔╝██║  ██║███████╗███████║
//  ╚═════╝ ╚══════╝ ╚═════╝ ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝

#define time_interval_minutes 10
#define led_pin 9
#define dht_pin 5
#define rtc_interrupt_pin 2
//#define sd_chip_select_pin 4

const int interrupt_number = digitalPinToInterrupt(rtc_interrupt_pin);
DHT dht(dht_pin, DHT22);
int operation_mode;
AT24Cxx at24cxx(0x57, 32); // 32Kbytes (EEPROM size)

// ███████╗███████╗████████╗██╗   ██╗██████╗
// ██╔════╝██╔════╝╚══██╔══╝██║   ██║██╔══██╗
// ███████╗█████╗     ██║   ██║   ██║██████╔╝
// ╚════██║██╔══╝     ██║   ██║   ██║██╔═══╝
// ███████║███████╗   ██║   ╚██████╔╝██║
// ╚══════╝╚══════╝   ╚═╝    ╚═════╝ ╚═╝

void setup()
{
  Serial.begin(115200);

  pinMode(led_pin, OUTPUT);

  RTC.setAlarm(ALM1_MATCH_DATE, 0, 0, 0, 1);
  RTC.setAlarm(ALM2_MATCH_DATE, 0, 0, 0, 1);
  RTC.alarm(ALARM_1);
  RTC.alarm(ALARM_2);
  RTC.alarmInterrupt(ALARM_1, false);
  RTC.alarmInterrupt(ALARM_2, false);
  RTC.squareWave(SQWAVE_NONE);

  //  reset_rtc();

  pinMode(rtc_interrupt_pin, INPUT_PULLUP);
  RTC.squareWave(SQWAVE_NONE);
  RTC.alarmInterrupt(ALARM_1, true);
  //  set_next_alarm();

  dht.begin();

  operation_mode = detect_operation_mode();

  //  SD.begin(sd_chip_select_pin);

  //  print_logged_data();
  //  while (1) {
  //    ;
  //  }

  //  mem_write(0, 0);
}

// ███╗   ███╗ ██████╗ ██████╗ ███████╗
// ████╗ ████║██╔═══██╗██╔══██╗██╔════╝
// ██╔████╔██║██║   ██║██║  ██║█████╗
// ██║╚██╔╝██║██║   ██║██║  ██║██╔══╝
// ██║ ╚═╝ ██║╚██████╔╝██████╔╝███████╗
// ╚═╝     ╚═╝ ╚═════╝ ╚═════╝ ╚══════╝

// ██████╗ ███████╗████████╗███████╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗
// ██╔══██╗██╔════╝╚══██╔══╝██╔════╝██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║
// ██║  ██║█████╗     ██║   █████╗  ██║        ██║   ██║██║   ██║██╔██╗ ██║
// ██║  ██║██╔══╝     ██║   ██╔══╝  ██║        ██║   ██║██║   ██║██║╚██╗██║
// ██████╔╝███████╗   ██║   ███████╗╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║
// ╚═════╝ ╚══════╝   ╚═╝   ╚══════╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝

int detect_operation_mode()
{
  Serial.println(ARDUINO_CONNECTED);
  while (Serial.available() == 0 && millis() < 4000)
  {
  }
  char buffer[1];
  if (Serial.available() == 0)
  {
    return STANDALONE_MODE;
  }
  Serial.readBytes(buffer, 1);
  switch (buffer[0])
  {
  case COMPUTER_CONNECTED:
    return COMPUTER_MODE;

  case ESP32_CONNECTED:
    return ESP32_MODE;

  default:
    return UNKNOWN_MODE;
  }

  return UNKNOWN_MODE;
}

// ██╗      ██████╗  ██████╗ ██████╗
// ██║     ██╔═══██╗██╔═══██╗██╔══██╗
// ██║     ██║   ██║██║   ██║██████╔╝
// ██║     ██║   ██║██║   ██║██╔═══╝
// ███████╗╚██████╔╝╚██████╔╝██║
// ╚══════╝ ╚═════╝  ╚═════╝ ╚═╝

// ███████╗██╗   ██╗███╗   ██╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
// ██╔════╝██║   ██║████╗  ██║██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
// █████╗  ██║   ██║██╔██╗ ██║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
// ██╔══╝  ██║   ██║██║╚██╗██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
// ██║     ╚██████╔╝██║ ╚████║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
// ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝

void loop()
{
  switch (operation_mode)
  {
  case STANDALONE_MODE:
    logger_mode_loop();
    break;

  case COMPUTER_MODE:
    computer_mode_loop();
    break;

  case ESP32_MODE:
    esp32_mode_loop();
    break;

  default:
    unknown_mode_loop();
    break;
  }
}

void logger_mode_loop()
{
  set_next_alarm();

  sleep_enable();
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);
  sleep_cpu();

  log_data();
}

void computer_mode_loop()
{
  if (Serial.available() == 0)
  {
    return;
  }
  byte buffer[1];
  Serial.readBytes(buffer, 1);
  switch (buffer[0])
  {
  case LOG_DATA_REQUEST:
    print_logged_data();
    break;
  case LOG_MODE_REQUEST:
    operation_mode = STANDALONE_MODE;
    Serial.println("computer mode off");
    break;
  case DELETE_DATA_REQUEST:
    mem_write(0, 0);
    Serial.println(DELETE_DATA_ACKNOWLEDGE);
    break;
  case CLOSE_REQUEST:
    Serial.println(CLOSE_ACKNOWLEDGE);
    break;
  case KEEPALIVE_REQUEST:
    Serial.println(KEEPALIVE_ACKNOWLEDGE);
    break;
  case USELESS_REQUEST:
    break;
  case RECORD_COUNT_REQUEST:
    Serial.println(mem_read(0));
    break;
  case REALTIME_MEASUREMENT_REQUEST:
    realtime_measurement();
    break;
  case TEST_WHOLE_EEPROM_REQUEST:
    test_whole_eeprom();
    break;
  default:
    Serial.println("unrecognized command");
    break;
  }
}

void esp32_mode_loop()
{
  if (Serial.available() == 0)
  {
    return;
  }
  byte buffer[1];
  Serial.readBytes(buffer, 1);
  switch (buffer[0])
  {
  case TEMPERATURE_HUMIDITY_NOW_REQUEST:
    Serial.println(dht.readTemperature());
    Serial.println(dht.readHumidity());
    break;

  case ARDUINO_OPERATION_MODE_REQUEST:
    Serial.write((byte)operation_mode);
    break;

  default:
    Serial.println("unrecognized command");
    break;
  }
}

void unknown_mode_loop()
{
  analogWrite(led_pin, 100);
  delay(1000);
  analogWrite(led_pin, 0);
  delay(1000);
}

// ██████╗ ████████╗ ██████╗
// ██╔══██╗╚══██╔══╝██╔════╝
// ██████╔╝   ██║   ██║
// ██╔══██╗   ██║   ██║
// ██║  ██║   ██║   ╚██████╗
// ╚═╝  ╚═╝   ╚═╝    ╚═════╝

//  █████╗ ███╗   ██╗██████╗
// ██╔══██╗████╗  ██║██╔══██╗
// ███████║██╔██╗ ██║██║  ██║
// ██╔══██║██║╚██╗██║██║  ██║
// ██║  ██║██║ ╚████║██████╔╝
// ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝

// ██╗███╗   ██╗████████╗███████╗██████╗ ██████╗ ██╗   ██╗██████╗ ████████╗███████╗
// ██║████╗  ██║╚══██╔══╝██╔════╝██╔══██╗██╔══██╗██║   ██║██╔══██╗╚══██╔══╝██╔════╝
// ██║██╔██╗ ██║   ██║   █████╗  ██████╔╝██████╔╝██║   ██║██████╔╝   ██║   ███████╗
// ██║██║╚██╗██║   ██║   ██╔══╝  ██╔══██╗██╔══██╗██║   ██║██╔═══╝    ██║   ╚════██║
// ██║██║ ╚████║   ██║   ███████╗██║  ██║██║  ██║╚██████╔╝██║        ██║   ███████║
// ╚═╝╚═╝  ╚═══╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝  ╚═╝ ╚═════╝ ╚═╝        ╚═╝   ╚══════╝

void set_next_alarm()
{
  time_t t = RTC.get();
  RTC.setAlarm(ALM1_MATCH_MINUTES, 0, (minute(t) + time_interval_minutes) % 60, 0, 0);
  // RTC.setAlarm(ALM1_MATCH_SECONDS, (second(t) + 4) % 60, 0, 0, 0);
  RTC.alarm(ALARM_1);
  attachInterrupt(interrupt_number, wakeup_interrupt, LOW);
}

void wakeup_interrupt()
{
  sleep_disable();
  detachInterrupt(interrupt_number);
}

void reset_rtc()
{
  tmElements_t tm;
  tm.Hour = 18; // set the RTC to an arbitrary time
  tm.Minute = 5;
  tm.Second = 30;
  tm.Day = 17;
  tm.Month = 1;
  tm.Year = 2019 - 1970; // tmElements_t.Year is the offset from 1970
  RTC.write(tm);         // set the RTC from the tm structure
}

/* 
███████╗███████╗██████╗ ██████╗  ██████╗ ███╗   ███╗                                            
██╔════╝██╔════╝██╔══██╗██╔══██╗██╔═══██╗████╗ ████║                                            
█████╗  █████╗  ██████╔╝██████╔╝██║   ██║██╔████╔██║                                            
██╔══╝  ██╔══╝  ██╔═══╝ ██╔══██╗██║   ██║██║╚██╔╝██║                                            
███████╗███████╗██║     ██║  ██║╚██████╔╝██║ ╚═╝ ██║                                            
╚══════╝╚══════╝╚═╝     ╚═╝  ╚═╝ ╚═════╝ ╚═╝     ╚═╝                                            
                                                                                                
███╗   ███╗ █████╗ ███╗   ██╗ █████╗  ██████╗ ███████╗███╗   ███╗███████╗███╗   ██╗████████╗    
████╗ ████║██╔══██╗████╗  ██║██╔══██╗██╔════╝ ██╔════╝████╗ ████║██╔════╝████╗  ██║╚══██╔══╝    
██╔████╔██║███████║██╔██╗ ██║███████║██║  ███╗█████╗  ██╔████╔██║█████╗  ██╔██╗ ██║   ██║       
██║╚██╔╝██║██╔══██║██║╚██╗██║██╔══██║██║   ██║██╔══╝  ██║╚██╔╝██║██╔══╝  ██║╚██╗██║   ██║       
██║ ╚═╝ ██║██║  ██║██║ ╚████║██║  ██║╚██████╔╝███████╗██║ ╚═╝ ██║███████╗██║ ╚████║   ██║       
╚═╝     ╚═╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝ ╚══════╝╚═╝     ╚═╝╚══════╝╚═╝  ╚═══╝   ╚═╝       
                                                                                                
███╗   ███╗ ██████╗ ██████╗ ██╗   ██╗██╗     ███████╗                                           
████╗ ████║██╔═══██╗██╔══██╗██║   ██║██║     ██╔════╝                                           
██╔████╔██║██║   ██║██║  ██║██║   ██║██║     █████╗                                             
██║╚██╔╝██║██║   ██║██║  ██║██║   ██║██║     ██╔══╝                                             
██║ ╚═╝ ██║╚██████╔╝██████╔╝╚██████╔╝███████╗███████╗                                           
╚═╝     ╚═╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚══════╝╚══════╝                                           
                                                                                                
*/
// reads 4 bytes. LSB at lowest address
uint32_t mem_read(uint16_t address)
{
  // uint8_t buffer[4];
  // AT24Cxx::ReadMem(address, buffer, 4);
  // uint32_t result = 0;
  // for (uint8_t i = 0; i < 4; i++)
  // {
  //   uint32_t partial = buffer[i];
  //   partial = (partial << (8 * i));
  //   result |= partial;
  // }
  // return result;

  uint32_t result = 0;
  for (uint16_t i = 0; i < 4; i++)
  {
    result |= (((uint32_t)at24cxx.read(address + i)) << (8 * i));
  }
  return result;
}

// writes 4 bytes. LSB at lowest address
void mem_write(uint16_t address, uint32_t value)
{
  // uint8_t buffer[4];
  // buffer[0] = ((uint32_t)(value & 0x000000FF));
  // buffer[1] = (((uint32_t)(value & 0x0000FF00)) >> 8);
  // buffer[2] = (((uint32_t)(value & 0x00FF0000)) >> 16);
  // buffer[3] = (((uint32_t)(value & 0xFF000000)) >> 24);
  // AT24Cxx::WriteMem(address, buffer, 4);

  for (uint16_t i = 0; i < 4; i++)
  {
    at24cxx.write(address + i, (uint8_t)((value >> (8 * i)) & 0x000000FF));
  }
}

void test_whole_eeprom()
{
  Serial.println("Starting 32Kbyte EEPROM test");
  uint16_t address;
  for (address = 0; address < at24cxx.length(); address++)
  {
    at24cxx.write(address, 0xFF);
    if (at24cxx.read(address) != 0xFF)
    {
      Serial.print("Error at address ");
      Serial.println(address);
      continue;
    }

    at24cxx.write(address, 0x00);
    if (at24cxx.read(address) != 0x00)
    {
      Serial.print("Error at address ");
      Serial.println(address);
    }
  }

  Serial.println("End of EEPROM test. All errors (if any) have been reported");
}

// ██╗      ██████╗  ██████╗         ██████╗  █████╗ ████████╗ █████╗
// ██║     ██╔═══██╗██╔════╝         ██╔══██╗██╔══██╗╚══██╔══╝██╔══██╗
// ██║     ██║   ██║██║  ███╗        ██║  ██║███████║   ██║   ███████║
// ██║     ██║   ██║██║   ██║        ██║  ██║██╔══██║   ██║   ██╔══██║
// ███████╗╚██████╔╝╚██████╔╝███████╗██████╔╝██║  ██║   ██║   ██║  ██║
// ╚══════╝ ╚═════╝  ╚═════╝ ╚══════╝╚═════╝ ╚═╝  ╚═╝   ╚═╝   ╚═╝  ╚═╝

// ███████╗██╗   ██╗███╗   ██╗ ██████╗████████╗██╗ ██████╗ ███╗   ██╗███████╗
// ██╔════╝██║   ██║████╗  ██║██╔════╝╚══██╔══╝██║██╔═══██╗████╗  ██║██╔════╝
// █████╗  ██║   ██║██╔██╗ ██║██║        ██║   ██║██║   ██║██╔██╗ ██║███████╗
// ██╔══╝  ██║   ██║██║╚██╗██║██║        ██║   ██║██║   ██║██║╚██╗██║╚════██║
// ██║     ╚██████╔╝██║ ╚████║╚██████╗   ██║   ██║╚██████╔╝██║ ╚████║███████║
// ╚═╝      ╚═════╝ ╚═╝  ╚═══╝ ╚═════╝   ╚═╝   ╚═╝ ╚═════╝ ╚═╝  ╚═══╝╚══════╝

void log_data()
{
  time_t t = RTC.get();
  float temperature = dht.readTemperature(), humidity = dht.readHumidity();

  analogWrite(led_pin, 100);

  delay(5000); // "hold time" after led on and before writing

  log_data_eeprom(t, temperature, humidity);

  delay(5000); // delay for completing  write

  analogWrite(led_pin, 0);
}

void log_data_eeprom(time_t t, float temperature, float humidity)
{
  uint32_t temp = (uint32_t)(10 * temperature);
  uint32_t hum = (uint32_t)(10 * humidity);
  uint32_t record_count = mem_read(0);
  uint16_t base_address = 4 + 12 * record_count;
  mem_write(base_address, t);
  mem_write(base_address + 4, temp);
  mem_write(base_address + 8, hum);
  mem_write(0, record_count + 1);

  int now_month = month(t), now_day = day(t), now_year = year(t), now_hour = hour(t), now_minute = minute(t), now_second = second(t);
  Serial.print("S");
  Serial.print(now_month);
  Serial.print("/");
  Serial.print(now_day);
  Serial.print("/");
  Serial.print(now_year);
  Serial.print(" ");
  Serial.print(now_hour);
  Serial.print(":");
  Serial.print(now_minute);
  Serial.print(":");
  Serial.print(now_second);
  Serial.print(" UTC-3");
  Serial.print(",");
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(humidity);
  Serial.println("E");
}

//void log_data_sd(time_t t, float temperature, float humidity) {
//  int now_month = month(t), now_day = day(t), now_year = year(t), now_hour = hour(t), now_minute = minute(t), now_second = second(t);
//  File sd_log = SD.open("log.txt", FILE_WRITE);
//
//  if (sd_log) {
//    String log_string = "";
//    log_string += now_month;
//    log_string += "/";
//    log_string += now_day;
//    log_string += "/";
//    log_string += now_year;
//    log_string += " ";
//    log_string += now_hour;
//    log_string += ":";
//    log_string += now_minute;
//    log_string += ":";
//    log_string += now_second;
//    log_string += " UTC-3";
//    log_string += ",";
//    log_string += temperature;
//    log_string += ",";
//    log_string += humidity;
//    sd_log.println(log_string);
//    sd_log.close();
//  }
//}

void print_logged_data()
{
  uint32_t record_count = mem_read(0);
  Serial.println(LOG_DATA_START);
  for (uint32_t i = 0; i < record_count; i++)
  {
    uint16_t base_address = 4 + i * 12;
    time_t t = mem_read(base_address);
    float temperature = 0.1 * ((float)(mem_read(base_address + 4)));
    float humidity = 0.1 * ((float)(mem_read(base_address + 8)));
    int now_month = month(t), now_day = day(t), now_year = year(t), now_hour = hour(t), now_minute = minute(t), now_second = second(t);
    //    Serial.print(now_month);
    //    Serial.print("/");
    //    Serial.print(now_day);
    //    Serial.print("/");
    //    Serial.print(now_year);
    //    Serial.print(" ");
    //    Serial.print(now_hour);
    //    Serial.print(":");
    //    Serial.print(now_minute);
    //    Serial.print(":");
    //    Serial.print(now_second);
    //    Serial.print(" UTC-3");
    Serial.print(t);
    Serial.print(",");
    Serial.print(temperature);
    Serial.print(",");
    Serial.println(humidity);
  }
  Serial.println(LOG_DATA_END);
}

void print_date()
{
  time_t t = RTC.get();
  int now_month = month(t), now_day = day(t), now_year = year(t), now_hour = hour(t), now_minute = minute(t), now_second = second(t);
  Serial.print(now_month);
  Serial.print("/");
  Serial.print(now_day);
  Serial.print("/");
  Serial.print(now_year);
  Serial.print(" ");
  Serial.print(now_hour);
  Serial.print(":");
  Serial.print(now_minute);
  Serial.print(":");
  Serial.print(now_second);
  Serial.println(" UTC-3");
}

void realtime_measurement()
{
  time_t t = RTC.get();
  float temperature = dht.readTemperature(), humidity = dht.readHumidity();

  int now_month = month(t), now_day = day(t), now_year = year(t), now_hour = hour(t), now_minute = minute(t), now_second = second(t);
  Serial.print("S");
  Serial.print(now_month);
  Serial.print("/");
  Serial.print(now_day);
  Serial.print("/");
  Serial.print(now_year);
  Serial.print(" ");
  Serial.print(now_hour);
  Serial.print(":");
  Serial.print(now_minute);
  Serial.print(":");
  Serial.print(now_second);
  Serial.print(" UTC-3");
  Serial.print(",");
  Serial.print(temperature);
  Serial.print(",");
  Serial.print(humidity);
  Serial.println("E");
}
