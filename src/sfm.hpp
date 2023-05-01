/*!
 * @file sfm.hpp
 *
 * SFM-V1.7 Fingerprint Module Driver by Matrixchung
 * https://github.com/Matrixchung
 * Note: This driver currently doesn't support characteristic value extract (return ~8206 bytes) and image receive (return more).
 */
#ifndef _SFM_HPP__
#define _SFM_HPP__

#include "Arduino.h"
#if defined(ESP32)
#include "HardwareSerial.h"
#else
#include "SoftwareSerial.h"
#endif
#include "string.h"

#define SFM_SERIAL_TIMEOUT    8000  // serial timeout (ms)
#define SFM_DEFAULT_USERROLE  0x03  // Default user role for register

#define SFM_ACK_SUCCESS       0x00  // Command successful
#define SFM_ACK_FAIL          0x01  // Command failed
#define SFM_ACK_FULL          0x04  // Database full
#define SFM_ACK_NOUSER        0x05  // User does not exist
#define SFM_ACK_USER_EXIST    0x07  // User exists
#define SFM_ACK_TIMEOUT       0x08  // Image collection timeout
#define SFM_ACK_HWERROR       0x0A  // Hardware error
#define SFM_ACK_IMGERROR      0x10  // Image error
#define SFM_ACK_BREAK         0x18  // Stop current cmd
#define SFM_ACK_ALGORITHMFAIL 0x11  // Film/Mask attack detected
#define SFM_ACK_HOMOLOGYFAIL  0x12  // Homology check fail
#define SFM_ACK_SERIALTIMEOUT 0x13  // Serial receive time exceeds SFM_SERIAL_TIMEOUT
#define SFM_ACK_IDLE          0x14  // Module idle

#define SFM_RING_OFF          0x07  // Ring LED Off
#define SFM_RING_RED          0x03  // Ring Color Red
#define SFM_RING_GREEN        0x05  // Ring Color Green
#define SFM_RING_BLUE         0x06  // Ring Color Blue
#define SFM_RING_YELLOW       0x01  // Ring Color Yellow
#define SFM_RING_PURPLE       0x02  // Ring Color Purple
#define SFM_RING_CYAN         0x04  // Ring Color Cyan

/*!
 * @brief Main class for SFM-V1.7 Fingerprint Module
 */
class SFM_Module{
public:
#if defined(ARDUINO_AVR_PROMICRO16)
  SFM_Module(uint8_t vccPin, uint8_t irqPin, HardwareSerial &hs);
#else
  SFM_Module(uint8_t vccPin, uint8_t irqPin, uint8_t rxPin, uint8_t txPin, uint8_t uartIndex = 1);
#endif
  ~SFM_Module();
  void enable();
  void disable();
  void setPinInterrupt(void (*pinInt)(void));
  #if defined(ESP32)
  void IRAM_ATTR pinInterrupt();
  #else
  void pinInterrupt();
  #endif
  bool isTouched();
  bool isConnected();
  uint16_t getUserCount();
  String getUuid();
  // functions below return SFM_ACK_XX
  uint8_t sendCmd(uint8_t cmdType, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t &ackType, uint8_t &q1, uint8_t &q2); // only when debugging can be called directly!
  uint8_t setRingColor(uint8_t startColor, int8_t endColor = -1, uint16_t period = 500);
  uint8_t register_3c3r_1st(uint16_t uid = 0);
  uint8_t register_3c3r_2nd();
  uint8_t register_3c3r_3rd(uint16_t &returnUid);
  uint8_t deleteUser(uint16_t uid);
  uint8_t deleteAllUser();
  uint8_t recognition_1v1(uint16_t uid);
  uint8_t recognition_1vN(uint16_t &returnUid);
  uint8_t stopAll();

protected:
  uint8_t _getCheckSum(uint8_t *buffer);
  uint8_t _getDataPackage(String &package);
  uint8_t _getCmdReturn(uint8_t cmdType, uint8_t p1 = 0x00, uint8_t p2 = 0x00, uint8_t p3 = 0x00);
#if defined(ESP32)
  HardwareSerial sfmSerial;
#elif defined(ARDUINO_AVR_PROMICRO16)
  HardwareSerial &sfmSerial;
#else
  SoftwareSerial sfmSerial;
#endif
  uint8_t cmdBuffer[8] = {0};
  uint8_t ackBuffer[8] = {0};
  bool touched = false;
  String uuid = "";
  uint16_t userCount = 0;
  uint8_t vcc_pin;
  uint8_t irq_pin;
#if not defined(ARDUINO_AVR_PROMICRO16)
  uint8_t rx_pin;
  uint8_t tx_pin;
#endif
};

#endif