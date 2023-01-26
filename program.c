#include <pm2008_i2c.h>
#include <SoftwareSerial.h>
#include <DFPlayer_Mini_Mp3.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
PM2008_I2C pm2008_i2c;
SoftwareSerial dfp(10, 11); // RX, TX -> UART 통신
#define VELOCITY_TEMP(temp) ( ( 331.5 +0.6 * (float)( temp ) ) *100 /1000000.0 ) // 온도로 보정된 초음파 속도(cm/us)

//RGB LED 포트
int redPin =2;
int bluePin =3;
int greenPin =4;

//초음파 센서 포트와 변수
int16_t trigPin =8;
int16_t echoPin =9;
uint16_t distance;
uint32_t pulseWidthUs;

//미세먼지에 따라 LED 색 바꿔주기 위한 변수
int switchLed =0;

//LCD 사용시간 변수
int sec =0;
int _min =0;
int hour =0;
int counter =0;

//초기 설정
void setup(){  

  //LCD
  lcd.begin();      // LCD 초기화
  lcd.backlight();  // LCD 백라이트 ON
  
  //미세먼지 센서
  pm2008_i2c.begin();
  Serial.begin(9600);        // 시리얼 속도 설정
  pm2008_i2c.command();
  delay(1000);

  //초음파 센서
  pinMode(trigPin,OUTPUT);
  digitalWrite(trigPin,LOW);
  pinMode(echoPin,INPUT);

  //RGB LED
  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  //MP3 모듈
  dfp.begin (9600);
  mp3_set_serial (dfp);      // Mp3 module 통신 세팅
  delay(1);                  // 볼륨값 적용을 위한 delay
  mp3_set_volume (30);       // 볼륨조절 값 0~30

  //시작 소리 출력
  dfp.listen();
  mp3_play(6);
  delay(5000);
}

//시작
void loop(){

  //LCD 사용시간(분까지)
  sec = millis()/1000-60*counter;
  if(sec >60) {
    counter++;
    _min++;
  }
  if(_min ==60){
    _min =0;
    hour++;

    //1시간 마다 사용시간 알림
    switch(hour) {
      case 1 : {
        lcd.setCursor(12,0);
        lcd.print(hour);
        lcd.print(":00");
        mp3_play(1);
        delay(3000);
        break;
      }
      case 2 : {
        lcd.setCursor(12,0);
        lcd.print(hour);
        lcd.print(":00");
        mp3_play(2);
        delay(3000);
        break;
      }
      case 3 : {
        lcd.setCursor(12,0);
        lcd.print(hour);
        lcd.print(":00");
        mp3_play(3);
        delay(3000);
        break;
      }
      case 4 : {
        lcd.setCursor(12,0);
        lcd.print(hour);
        lcd.print(":00");
        mp3_play(4);
        delay(3000);
        break;
      }
      case 5 : {
        lcd.setCursor(12,0);
        lcd.print(hour);
        lcd.print(":00");
        mp3_play(5);
        delay(7000);
        break;
      }
      default : break;
    }
  }
  lcd.setCursor(0,0);
  lcd.print("USAGE TIME ");
  if(hour<10) lcd.print("0");
  lcd.print(hour);lcd.print(":");
  if(_min<10) lcd.print("0");
  lcd.print(_min);

  //초음파 센서
  int16_t  dist, temp;
  digitalWrite(trigPin,HIGH);//Tirg Pin을 HIGH로 설정합니다.
  delayMicroseconds(50);     //Delay of 50 microseconds
  digitalWrite(trigPin,LOW); //Tirg Pin을 LOW로 설정합니다.
  pulseWidthUs = pulseIn(echoPin,HIGH); //에코 HIGH 레벨 시간 측정, 출력 HIGH 레벨 시간은 초음파 비행 시간(단위: us)을 나타냅니다.
  distance = pulseWidthUs * VELOCITY_TEMP(33) /2.0; //초음파 비행시간에 따라 거리 계산 가능, 초음파 음속은 실제 주변 온도에 따라 보정할 수 있습니다.

  //초음파 시리얼 통신
  Serial.print(distance, DEC);
  Serial.println("cm");

  //초음파 LCD
  lcd.setCursor(11,1);
  lcd.print((float)distance/100);
  lcd.print("M");
  
  //"거리가 너무 가깝습니다" 출력
  dfp.listen();
  if(distance <70) {
    mp3_play(7);
    delay(2000);
  }

  //미세먼지 센서
  uint8_t ret = pm2008_i2c.read();
  uint8_t pm1p0_grade =0;
  uint8_t pm2p5_grade =0;
  uint8_t pm10p_grade =0;
  uint8_t total_grade =0;
  if (ret ==0) {
    // PM 1.0
    if (pm2008_i2c.pm1p0_grimm <16) {
      pm1p0_grade =1;
    } else if (pm2008_i2c.pm1p0_grimm <51) {
      pm1p0_grade =2;
    } else {
      pm1p0_grade =3;
    }
    // PM 2.5
    if (pm2008_i2c.pm2p5_grimm <16) {
      pm2p5_grade =1;
    } else if (pm2008_i2c.pm2p5_grimm <51) {
      pm2p5_grade =2;
    } else {
      pm2p5_grade =3;
    }
    // PM 10
    if (pm2008_i2c.pm10_grimm <31) {
      pm10p_grade =1;
    } else if (pm2008_i2c.pm10_grimm <81) {
      pm10p_grade =2;
    } else {
      pm10p_grade =3;
    }

    // 가장 높은 미세먼지 추출
    total_grade = max(pm1p0_grade, pm2p5_grade);
    total_grade = max(total_grade, pm10p_grade);
    
    dfp.listen();
    
    switch (total_grade) {
      case 1: {
        setColor(0, 0, 255); // blue
        lcd.setCursor(0,1);  // LCD 위치
        lcd.print("PM:GOOD  "); // LCD 출력
        if(switchLed !=1) {
          mp3_play(10);
          delay(3000);
        }
        switchLed =1;
        break;
      }
      case 2: {
        setColor(0, 255, 0); // green
        lcd.setCursor(0,1);  // LCD 위치
        lcd.print("PM:NORMAL"); // LCD 출력
        if(switchLed !=2) {
          mp3_play(9);
          delay(3000);
        }
        switchLed =2;
        break;
      }
      case 3: {
        setColor(255, 0, 0); // red
        lcd.setCursor(0,1);  // LCD 위치
        lcd.print("PM:WORST "); // LCD 출력
        if(switchLed !=3) {
          mp3_play(8);
          delay(3000);
        }
        switchLed =3;
        break;
      }
      default:
      break;
    }
  }
  delay(500);
}

//RGB LED 색깔 만들기 함수
void setColor(int red, int green, int blue)
{
  analogWrite(redPin, red);
  analogWrite(greenPin, green);
  analogWrite(bluePin, blue); 
}