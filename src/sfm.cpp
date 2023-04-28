/*!
 * @file sfm.cpp
 * @author Matrixchung
 */
#include "sfm.hpp"
char hex_char[17] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};
/*!
    @brief Constructor for SFM-V1.7 Module
    @param vccPin
           Pin number for VCC
    @param irqPin
           Pin number for TOUCH_OUT
    @param rxPin
           Pin number for UART_TX
    @param txPin
           Pin number for UART_RX
    @param uartIndex (only useful for ESP32 platform)
           HardwareSerial uart_nr, default 1 means UART 1 (UART 0 is preconfigured and can't be used)
    @return SFM_Module object
    @note Remember to call SFM_Module::setPinInterrupt() before loop()! (see example for more information)
 */
#if defined(ESP32)
SFM_Module::SFM_Module(uint8_t vccPin, uint8_t irqPin, uint8_t rxPin, uint8_t txPin, uint8_t uartIndex):sfmSerial(uartIndex), vcc_pin(vccPin), irq_pin(irqPin), rx_pin(rxPin), tx_pin(txPin){
  pinMode(irq_pin, INPUT_PULLDOWN);
  pinMode(vcc_pin, OUTPUT);
  digitalWrite(vcc_pin, HIGH); // Enable sensor vcc
  cmdBuffer[0] = 0xF5;
  cmdBuffer[7] = 0xF5;
  sfmSerial.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
}
#elif defined(ARDUINO_AVR_PROMICRO16)
SFM_Module::SFM_Module(uint8_t vccPin, uint8_t irqPin, HardwareSerial &hs):sfmSerial(hs), vcc_pin(vccPin), irq_pin(irqPin){
  pinMode(irq_pin, INPUT);
  pinMode(vcc_pin, OUTPUT);
  digitalWrite(vcc_pin, HIGH); // Enable sensor vcc
  cmdBuffer[0] = 0xF5;
  cmdBuffer[7] = 0xF5;
  sfmSerial.begin(115200, SERIAL_8N1);
  while (!Serial1)
    delay(100);
}
#else
SFM_Module::SFM_Module(uint8_t vccPin, uint8_t irqPin, uint8_t rxPin, uint8_t txPin, uint8_t uartIndex):sfmSerial(rxPin, txPin), vcc_pin(vccPin), irq_pin(irqPin), rx_pin(rxPin), tx_pin(txPin){
  pinMode(irq_pin, INPUT);
  pinMode(vcc_pin, OUTPUT);
  digitalWrite(vcc_pin, HIGH); // Enable sensor vcc
  cmdBuffer[0] = 0xF5;
  cmdBuffer[7] = 0xF5;
  sfmSerial.begin(115200);
}
#endif
SFM_Module::~SFM_Module(){
  detachInterrupt(irq_pin);
}
/*!
    @brief Set outside ring's LED color
    @param startColor
           LED's start color. Only SFM_RING_XXX can be chosen.
    @param endColor
           LED's end color. Only SFM_RING_XXX can be chosen.
    @param period
           LED will fade from startColor to endColor in given period. Measured in ms. range: 300-2000
    @return SFM_ACK_XXX
 */
uint8_t SFM_Module::setRingColor(uint8_t startColor, int8_t endColor, uint16_t period){
  uint8_t ackType, q1, q2, q3;
  period /= 10;
  period = constrain(period, 30, 200);
  if(endColor == -1) endColor = startColor;
  return _getCmdReturn(0xC3, startColor, endColor, period);
}
/*!
    @brief 3C3R Register step #1
    @param uid
           Specify the UID you want to add a new user. Default 0 means add to the end
    @return SFM_ACK_XXX
    @note Please check example for how to register using 3C3R method.
 */
uint8_t SFM_Module::register_3c3r_1st(uint16_t uid){
  return _getCmdReturn(0x01, (uid >> 8) & 0xFF, uid & 0xFF, SFM_DEFAULT_USERROLE);
}
/*!
    @brief 3C3R Register step #2
    @return SFM_ACK_XXX
    @note It must be called after SFM_Module::register_3c3r_1st() is successful, otherwise restart from #1
 */
uint8_t SFM_Module::register_3c3r_2nd(){
  return _getCmdReturn(0x02);
}
/*!
    @brief 3C3R Register step #3
    @param &returnUid
           Get the registered uid if success, else 0
    @return SFM_ACK_XXX
    @note It must be called after step #1 and #2 with both of them successful, otherwise you need to re-register from step #1
 */
uint8_t SFM_Module::register_3c3r_3rd(uint16_t &returnUid){
  uint8_t ackType, q1, q2, q3;
  q3 = sendCmd(0x03, 0x00, 0x00, 0x00, ackType, q1, q2);
  if(ackType == 0x03 && q3 == SFM_ACK_SUCCESS){
    returnUid = (q1 << 8) | q2;
    return SFM_ACK_SUCCESS;
  }
  returnUid = 0;
  return SFM_ACK_FAIL;
}
/*!
    @brief Delete specific user by uid
    @param uid
           UID which is going to be deleted
    @return SFM_ACK_XXX
 */
uint8_t SFM_Module::deleteUser(uint16_t uid){
  return _getCmdReturn(0x04, (uid >> 8) & 0xFF, uid & 0xFF);
}
/*!
    @brief Delete ALL user(s)
    @return SFM_ACK_XXX
 */
uint8_t SFM_Module::deleteAllUser(){
  return _getCmdReturn(0x05);
}
/*!
    @brief Match current fingerprint with specific uid
    @param uid
           Target uid
    @return SFM_ACK_SUCCESS if matched
 */
uint8_t SFM_Module::recognition_1v1(uint16_t uid){
  return _getCmdReturn(0x0B, (uid >> 8) & 0xFF, uid & 0xFF);
}
/*!
    @brief Search the database and find matched uid
    @param &returnUid
           Matched uid (0 if not matched)
    @return SFM_ACK_SUCCESS if matched
 */
uint8_t SFM_Module::recognition_1vN(uint16_t &returnUid){
  uint8_t ackType, q1, q2, q3;
  q3 = sendCmd(0x0C, 0x00, 0x00, 0x00, ackType, q1, q2);
  if(ackType == 0x0C){
    returnUid = (q1 << 8) | q2;
    return (returnUid == 0 && q3 != SFM_ACK_SUCCESS) ? SFM_ACK_FAIL : SFM_ACK_SUCCESS;
  }
  return SFM_ACK_FAIL;
}
/*!
    @brief Stop all current actions
    @return SFM_ACK_SUCCESS if successfully stopped
            SFM_ACK_IDLE if module is idle
            SFM_ACK_FAIL if failed
 */
uint8_t SFM_Module::stopAll(){
  uint8_t ackType, q1, q2, q3;
  q3 = sendCmd(0xFE, 0x00, 0x00, 0x00, ackType, q1, q2);
  if(ackType == 0x0C) return q3;
  if(ackType == 0xFE && q3 == SFM_ACK_FAIL) return SFM_ACK_IDLE;
  return SFM_ACK_FAIL;
}
/*!
    @brief Get the module's UUID
    @return 18-character hex string
 */
String SFM_Module::getUuid(){
  return (_getCmdReturn(0x60) == SFM_ACK_SUCCESS && _getDataPackage(uuid) == SFM_ACK_SUCCESS) ? uuid : "";
}
/*!
    @brief Get user count in database
    @return user count
 */
uint16_t SFM_Module::getUserCount(){
  uint8_t ackType, q1, q2, q3;
  q3 = sendCmd(0x09, 0x00, 0x00, 0x00, ackType, q1, q2);
  if(ackType == 0x09 && q3 == SFM_ACK_SUCCESS) userCount = (q1 << 8) | q2;
  else userCount = 0;
  return userCount;
}
uint8_t SFM_Module::sendCmd(uint8_t cmdType, uint8_t p1, uint8_t p2, uint8_t p3, uint8_t &ackType, uint8_t &q1, uint8_t &q2){
  while(sfmSerial.available()) sfmSerial.read(); // flush rx buffer
  cmdBuffer[1] = cmdType;
  cmdBuffer[2] = p1;
  cmdBuffer[3] = p2;
  cmdBuffer[4] = p3;
  cmdBuffer[6] = _getCheckSum(cmdBuffer);
  sfmSerial.write(cmdBuffer, 8);
  sfmSerial.flush(); // waiting for send complete
  unsigned int timer = SFM_SERIAL_TIMEOUT;
  while(timer--){
    if(sfmSerial.available() >= 8){
      unsigned int trimTimer = SFM_SERIAL_TIMEOUT;
      while(sfmSerial.peek() != 0xF5 && trimTimer--){
        sfmSerial.read(); // trim the cache to find first 0xF5 (ack start)
      }
      if(!trimTimer) {
        while(sfmSerial.available()) sfmSerial.read(); // flush buffer
        sfmSerial.flush();
        return SFM_ACK_SERIALTIMEOUT;
      }
      if(sfmSerial.available() >= 8){ // more than 8 bytes since the first 0xF5
        sfmSerial.readBytes(ackBuffer, 8);
        if(ackBuffer[6] == _getCheckSum(ackBuffer)){ // checksum matched, exit without flush buffer
          ackType = ackBuffer[1];
          q1 = ackBuffer[2];
          q2 = ackBuffer[3];
          return ackBuffer[4]; // return q3 as SFM_ACK
        }
        else {
          while(sfmSerial.available()) sfmSerial.read(); // flush buffer
          sfmSerial.flush();
          return SFM_ACK_FAIL;
        }
      }
    }
#if defined(ARDUINO_AVR_PROMICRO16)
    delay(2);
#else
    delay(1);
#endif
  }
  while(sfmSerial.available()) sfmSerial.read(); // flush buffer
  return SFM_ACK_SERIALTIMEOUT;
}
void SFM_Module::setPinInterrupt(void (*pinInt)(void)){
  attachInterrupt(digitalPinToInterrupt(irq_pin), pinInt, CHANGE);
}
#if defined(ESP32)
void IRAM_ATTR SFM_Module::pinInterrupt()
#else
void SFM_Module::pinInterrupt()
#endif
{
  touched = digitalRead(irq_pin);
}
bool SFM_Module::isTouched(){
  return touched;
}
bool SFM_Module::isConnected(){
  getUuid();
  return uuid.length() == 18;
}
void SFM_Module::enable(){
  digitalWrite(vcc_pin, HIGH);
}
void SFM_Module::disable(){
  digitalWrite(vcc_pin, LOW);
}
// result = xor checksum buffer[1:5]
uint8_t SFM_Module::_getCheckSum(uint8_t *buffer){
  uint8_t result = 0x00;
  for(uint8_t i = 1; i <= 5; i++) result^=(*(buffer+i));
  return result;
}
// get all bytes between two 0xF5 and convert to hex
uint8_t SFM_Module::_getDataPackage(String &package){
  package = "";
  char temp = '\0';
  unsigned int timer = SFM_SERIAL_TIMEOUT;
  while(timer--){
    if(sfmSerial.available() > 0){
      while(sfmSerial.read() != 0xF5); // trim the cache to find first 0xF5 and ignore it
      while(sfmSerial.peek() != 0xF5){
        // byte to hex char
        temp = sfmSerial.read();
        package += hex_char[temp / 16];
        package += hex_char[temp % 16];
      }
      if(package.length() > 0) return SFM_ACK_SUCCESS;
    }
    delay(1);
  }
  while(sfmSerial.available()) sfmSerial.read(); // flush buffer
  package = "";
  return SFM_ACK_SERIALTIMEOUT;
}
uint8_t SFM_Module::_getCmdReturn(uint8_t cmdType, uint8_t p1, uint8_t p2, uint8_t p3){
  uint8_t ackType, q1, q2, q3;
  q3 = sendCmd(cmdType, p1, p2, p3, ackType, q1, q2);
  if(ackType == cmdType) return q3;
  return SFM_ACK_FAIL;
}
