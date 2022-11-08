#include <SoftwareSerial.h>
#include <Adafruit_GPS.h>
#include "Adafruit_CCS811.h"

/*
 * 각 단계별 할 일
 * 0단계: 장시간 운전 (v)
 * 1단계: 환기 (v), 퀴즈 (했나?), 
 * 2단계: 차간거리 (v), 전화 (TODO), 
 */
//핀 사용 2,3,4,5,6,7,8,9 , 11,12  사용 중   || 아날로그 핀 A4, A5 사용 중 

// 블루투스 > 2, 3번 
// gps > 6, 7번 
// 스위치 > 8, 9 번 
// 공기 측정 > 아날로그 핀 A4, A5
// 초음파 > 11, 12번 
// 졸음 처리 > 4, 5번

// 블루투스 신호 번호
// 

SoftwareSerial S_bluetooth(2,3);
SoftwareSerial S_GPS(6,7);  // (tx,rx)
Adafruit_GPS GPS(&S_GPS);
Adafruit_CCS811 ccs;

//블루투스 전처리 
#define GPSECHO  true
int buttonState1 = 0;
int buttonState2 = 0;
int lastButtonState = 0;
int buttonPushCounter = 0;

//초음파 거리측정 전처리 
#define echoPin 11
#define trigPin 12

// 라즈베리 파이 입력
// 졸음 상태는 0(안 졸고 있음) ~ 3(심하게 졸고 있음)의 4단계로 되어 있음
// rpi 11 - arduino 4, rpi- 13 - arduino 5
// 0단계에서는 4, 5번 핀 모두 off
// 1단계에서는 4번 핀만 on
// 2단계에서는 5번 핀만 on
// 3단계에서는 4, 5번 핀 모두 on
#define SLEEPING_PIN_1 4
#define SLEEPING_PIN_2 5
long duration;
float distance;
unsigned long startTime, lastWarningTime;

int sleepLevel = 0;
int prevSleepLevel = 0;

void setup(){
  pinMode(8,INPUT); // 스위치.전화
  pinMode(9,INPUT); // 스위치.음악
  pinMode(4,INPUT);
  pinMode(5,INPUT);
  Serial.begin(9600); // pc-아두이노간 통신
  S_bluetooth.begin(9600); //블루투스 통신 시작

  GPS.begin(9600);
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);

  if(!ccs.begin()) {
    Serial.println("Failed to start sensor! Please check your wiring.");
    while(1);
  }
  while(!ccs.available());
  float temp = ccs.calculateTemperature();
  ccs.setTempOffset(temp - 25.0);

  //초음파 
  pinMode(trigPin, OUTPUT); 
  pinMode(echoPin, INPUT); 

  startTime = millis();
  lastWarningTime = millis() - 60000;
  
}

uint32_t timer = millis();

void loop() {
  prevSleepLevel = sleepLevel;
  sleepLevel = (int)digitalRead(SLEEPING_PIN_1) + (int)digitalRead(SLEEPING_PIN_2);
  
  S_bluetooth.listen();
  

  buttonState1 = digitalRead(8);
  buttonState2 = digitalRead(9);

  if(buttonState1 != lastButtonState) {
    delay(10);
  }

  if(buttonState1 == 1) {
    S_bluetooth.print(3);
  }
  
  if(buttonState2 != lastButtonState) {
    if(buttonState2 == 1) {
      buttonPushCounter += 1; //buttonPushCounter = 1....
    }
  }
  lastButtonState = buttonState2;
  
  
  if(buttonPushCounter % 2 == 0) {
    S_bluetooth.print(2);
  } // 음악 정지
  else if(buttonPushCounter %2 == 1) {
    S_bluetooth.print(1);
  } // 음악 재생

  //Serial.println(lastButtonState);
  Serial.println(buttonState1);
  //Serial.println(buttonPushCounter);

  // gps 모듈 loop

  // 1) 모듈로부터 속도값 받아오기 
  S_GPS.listen();
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) {
      return;  
    }
  }

  if (millis() - timer > 1000) {
    timer = millis(); // reset the timer
    Serial.print("Date: ");
    Serial.print(GPS.day, DEC); Serial.print('/');
    Serial.print(GPS.month, DEC); Serial.print("/20");
    Serial.println(GPS.year, DEC);
    if (GPS.fix) {
      Serial.print("Speed (km/h): "); Serial.println((GPS.speed)*1.852); // 1knot = 1.852km/h 이다. 
    }
  }

  // 2) 초음파 모듈 거리 측정 Loop 
  digitalWrite(trigPin,LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigPin,HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin,LOW);

  duration = pulseIn(echoPin,HIGH);
  distance = ((float)(340*duration)/10000)/2;
  
  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.print(" cm"); 

  // 3) 속도와 거리로 안전거리 경보 (예를들어 > 100km/h 일 때, 100m가 안전거리임) 
  // 만약 속도가 50이고 안전거리가 100 ok
  // 즉, 속도가 거리보다 작으면 ok 
  // 속도가 거리보다 크면 알람
  // TODO 이거 잘못하면 0.1초마다 계속 신호 날려댈수도 있음
  // 최소한의 딜레이를 걸어야 함
  // + 이거는 2단계 졸음부터 넣기
  if (sleepLevel >= 2) {
    if (GPS.fix) {
      // distance*0.01 = m 단위
      if ( (GPS.speed)*1.852 > (distance*0.01) ) {
        S_bluetooth.print(5);  // 안전리 음성 
      }  
    }
  }

  // 공기 측정 모드 loop
  // 이거는 1단계부터
  if (sleepLevel >= 1) {
    if(ccs.available()) { // co2와, tvoc 위험 농도 이상일 시 환기 알림 
      float temp = ccs.calculateTemperature();
      if(!ccs.readData()) {
        Serial.print("CO2: ");
        Serial.print(ccs.geteCO2());
        Serial.print("ppm, TVOC: ");
        Serial.print(ccs.getTVOC());
        Serial.print("ppb Temp:");
        Serial.println(temp);
    
        if (ccs.geteCO2() > 1000 && ccs.getTVOC() > 500 && sleepLevel >= 1) {
          S_bluetooth.print(4);  // 환기 권고 음성
        }
      }
    }
    
    else {
      Serial.println("ERROR!");
    }
  }

  if (sleepLevel == 1 && millis() >= lastWarningTime + 60000) {
    S_bluetooth.print(1); // 졸음 경보 음성 + 아재개그
    lastWarningTime = millis();
  }

  

  else if (sleepLevel == 2 && millis() >= lastWarningTime + 60000) {
    S_bluetooth.print(8); // 졸지 말라고 경보하는 음성 2단계 == 전화 권고 음성
    lastWarningTime = millis();
  }

  

  // 장시간 운전 시간 체크 loop 
  if (millis() - startTime >= 7200000) {  // 운전시간 2시간 후 장시간 운전 경고 알림
    S_bluetooth.print(6);  //장시간 운전 경고 알림장시간 운전 경고 알림
    startTime += 7200000;
  }
  
  delay(10);
} // loop 닫는 바 



