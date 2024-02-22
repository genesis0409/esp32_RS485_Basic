/*
 * https://github.com/CMB27/ModbusRTUMaster/tree/main

 * Function Code: 0x01; readCoils(): modbus.readCoils(slaveId, startAddress, buffer, quantity); 1비트 읽기; 싱글 코일 읽기
 * Function Code: 0x02; readDiscreteInputs(): modbus.readDiscreteInputs(slaveId, startAddress, buffer, quantity); 1비트 읽기; 이산 입력 읽기
 * Function Code: 0x03; readHoldingRegisters(): modbus.readHoldingRegisters(slaveId, startAddress, buffer, quantity); 여러 워드 읽기; 레지스터의 연속 블록 내용 읽기
 * Function Code: 0x04; readInputRegisters(): modbus.readInputRegisters(slaveId, startAddress, buffer, quantity); 여러 워드 읽기; 입력 레지스터 읽기
 *
 * Function Code: 0x05; writeSingleCoil(): modbus.writeSingleCoil(slaveId, address, value); 1비트 쓰기; 원격 장치의 ON/OFF에 단일 출력 쓰기; FF00:ON; 0000:OFF
 * Function Code: 0x06; writeSingleHoldingRegister(): modbus.writeSingleHoldingRegister(slaveId, address, value); 1워드 쓰기; 싱글 홀딩 레지스터 쓰기
 * Function Code: 0x0F; writeMultipleCoils(): modbus.writeMultipleCoils(slaveId, startingAddress, buffer, quantity); 여러 비트 쓰기; 다수 코일 ON/OFF 지정
 * Function Code: 0x10; writeMultipleHoldingRegisters(): modbus.writeMultipleHoldingRegisters(slaveId, startingAddress, buffer, quantity); 여러 워드 쓰기; 연속 레지스터 블록 쓰기
*/
#include <ModbusRTUMaster.h>

#define SLAVE_ID 1
#define START_ADDRESS 0
#define QUANTITY 2
#define SCAN_RATE 30000

// **************** Soil Moisture + Temp + EC ****************
#define EC_SENSING_MODE
// ***********************************************************

unsigned long currentMillis = 0;
unsigned long previousMillis = 0; // Stores last time using Reference time

unsigned int messageID = 0;
#if defined(EC_SENSING_MODE)
float t;
float soil_m; // Soil Moisture
float ec;     // 전기 전도도
#else
float t;
float h;
#endif

const uint8_t rxPin = 33; // RX-RO
const uint8_t txPin = 32; // TX-DI
const uint8_t dePin = 25; // DE+RE

ModbusRTUMaster modbus(Serial1, dePin); // serial port, driver enable pin for rs-485 (optional)

#if defined(EC_SENSING_MODE)
uint16_t holdingRegisters[3] = {0xFF, 0xFF, 0xFF};
#else
uint16_t holdingRegisters[2] = {0xFF, 0xFF};
#endif

void setup()
{
    Serial.begin(115200);

    modbus.setTimeout(12000);
    modbus.begin(9600, SERIAL_8N1, rxPin, txPin); // 직렬 전송 설정 (baud, config, rxPin, txPin, invert)
                                                  // default config : SERIAL_8N1; { 데이터비트 8, 패리티 없음, 1 정지 비트}; E: 짝수 패리티, O: 홀수 패리티
                                                  // rxPin: 직렬 데이터 수신 핀; txPin: 직렬  데이터 전송 핀 (uint8_t)
}

void loop()
{
    currentMillis = millis();
    if (currentMillis - previousMillis >= SCAN_RATE)
    {
        Serial.print("Prieod: ");
        Serial.println(currentMillis - previousMillis);

#if defined(EC_SENSING_MODE)
        modbus.readHoldingRegisters(1, 0, holdingRegisters, 3);
        t = holdingRegisters[0] / 10.0;
        soil_m = holdingRegisters[1] / 10.0;
        ec = holdingRegisters[2] / 1000.0;
        Serial.printf("RK520-02 [messageID]: %d | [TEMP]: %.1f, [Moisture]: %.1f, [EC]: %.2f,\n", messageID, t, soil_m, ec);

#else
        modbus.readHoldingRegisters(1, 0, holdingRegisters, 2);
        t = holdingRegisters[0] / 10.0;
        h = holdingRegisters[1] / 10.0;
        Serial.printf("TZ-THT02 [messageID]: %d | [TEMP]: %.1f, [HUMI]: %.1f\n", messageID, t, h);
#endif
        previousMillis = currentMillis;
        messageID++;
    }
}