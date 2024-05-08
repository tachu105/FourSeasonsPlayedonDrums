#include <Adafruit_NeoPixel.h>

//LEDPin定義//
#define led_R1 13//右から順番
#define led_B1 23
#define led_R2 12
#define led_B2 25
#define led_R3 11
#define led_B3 27

//タタコンPin定義//
#define sensor_D1 9  //右から順番
#define sensor_C1 31
#define sensor_D2 8
#define sensor_C2 33
#define sensor_D3 7
#define sensor_C3 35

#define delayTime 150

#define full_Input_Num 5  //入力配列数

#define pixel_Num 16  //片側合計ピクセル数
#define pixel_Pin_R 5  //ピクセル信号ピン定義  
#define pixel_Pin_L 4

Adafruit_NeoPixel pixelsR(pixel_Num, pixel_Pin_R, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsL(pixel_Num, pixel_Pin_L, NEO_GRB + NEO_KHZ800);


//変数宣言//
int sensorState_D1; //タタコンセンサ入力ピン
int sensorState_C1;
int sensorState_D2;
int sensorState_C2;
int sensorState_D3;
int sensorState_C3;
int hitPattern; //入力結果
int array[full_Input_Num];  //入力パターン代入配列
int inputCounter = 0; //配列代入位置変更変数
int baseTime = 0;
int movieTime = 0;
int curTime = 0;
int isReset = 1;


void setup() {
  //LEDPin設定//
  pinMode(led_R1, OUTPUT);
  pinMode(led_B1, OUTPUT);
  pinMode(led_R2, OUTPUT);
  pinMode(led_B2, OUTPUT);
  pinMode(led_R3, OUTPUT);
  pinMode(led_B3, OUTPUT);

  //タタコンPin設定//
  pinMode(sensor_D1, INPUT);
  pinMode(sensor_C1, INPUT);
  pinMode(sensor_D2, INPUT);
  pinMode(sensor_C2, INPUT);
  pinMode(sensor_D3, INPUT);
  pinMode(sensor_C3, INPUT);

  Serial.begin(9600);

  //NeoPixel初期化//
  pixelsR.begin();
  pixelsL.begin();
  pixelsR.clear();
  pixelsL.clear();
  pixelsR.show();
  pixelsL.show();

}

void loop() {
  //タタコンデータ取得//
  sensorState_D1 = digitalRead(sensor_D1);
  sensorState_C1 = digitalRead(sensor_C1);
  sensorState_D2 = digitalRead(sensor_D2);
  sensorState_C2 = digitalRead(sensor_C2);
  sensorState_D3 = digitalRead(sensor_D3);
  sensorState_C3 = digitalRead(sensor_C3);


  if (sensorState_D1 == HIGH) {     //選択センサ1
    digitalWrite(led_R1, LOW);  // If the sensorState is HIGH, Nothing happens.
  } else {
    digitalWrite(led_R1, HIGH); // If the sensorState is LOW, Taiko is triggered.
    Serial.write(1);  //processingにデータ送信
    ArrayInput(1);  //NeoPixel操作用情報入力
    delay(delayTime);
    while (1) {
      sensorState_D1 = digitalRead(sensor_D1);
      sensorState_C1 = digitalRead(sensor_C1);
      sensorState_D2 = digitalRead(sensor_D2);
      sensorState_C2 = digitalRead(sensor_C2);
      sensorState_D3 = digitalRead(sensor_D3);
      sensorState_C3 = digitalRead(sensor_C3);
      if (sensorState_D1 == HIGH) break;
    }
    delay(50);
    baseTime = millis()/1000;
  }

  if (sensorState_D2 == HIGH) {     //選択センサ2
    digitalWrite(led_R2, LOW);  // If the sensoq
  } else {
    digitalWrite(led_R2, HIGH); // If the sensorState is LOW, Taiko is triggered.
    Serial.write(2);  //processingにデータ送信
    ArrayInput(2);  //NeoPixel操作用情報入力
    delay(delayTime);
    while (1) {
      sensorState_D1 = digitalRead(sensor_D1);
      sensorState_C1 = digitalRead(sensor_C1);
      sensorState_D2 = digitalRead(sensor_D2);
      sensorState_C2 = digitalRead(sensor_C2);
      sensorState_D3 = digitalRead(sensor_D3);
      sensorState_C3 = digitalRead(sensor_C3);
      if (sensorState_D2 == HIGH) break;
    }
    delay(50);
    baseTime = millis()/1000;
  }


  if (sensorState_D3 == HIGH) {    //決定センサ
    digitalWrite(led_R3, LOW);  // If the sensorState is HIGH, Nothing happens.
  } else {
    digitalWrite(led_R3, HIGH); // If the sensorState is LOW, Taiko is triggered.
    MakePattern();  //叩いた結果を算出
    Serial.write(3);  //processingにデータ送信
    ChangePixels(); //提灯カラー変更
    ArrayReset(); //配列初期化
    delay(delayTime);
    while (1) {
      sensorState_D1 = digitalRead(sensor_D1);
      sensorState_C1 = digitalRead(sensor_C1);
      sensorState_D2 = digitalRead(sensor_D2);
      sensorState_C2 = digitalRead(sensor_C2);
      sensorState_D3 = digitalRead(sensor_D3);
      sensorState_C3 = digitalRead(sensor_C3);
      if (sensorState_D3 == HIGH) break;
    }
    delay(50);
    baseTime = millis()/1000;
  }

  curTime = millis()/1000;

  if (((curTime - movieTime >= 60) && (curTime - baseTime >= 20)) && isReset == 0) {
    ArrayReset();
    Serial.write(3);
    
    ArrayInput(1);
    ArrayInput(1);
    ArrayInput(1);
    ArrayInput(1);
    ArrayInput(2);
    Serial.write(1);
    Serial.write(1);
    Serial.write(1);
    Serial.write(1);
    Serial.write(2);
    Serial.write(3);

    MakePattern();
    ArrayReset();

    pixelsR.clear();
    pixelsL.clear();
    pixelsR.show();
    pixelsL.show();

    isReset = 1;
  }
}




void ArrayInput (int sensor_num) {  //取得データを配列に保存
  array[inputCounter] = sensor_num;
  inputCounter ++;

  if (inputCounter > full_Input_Num) {
    ArrayReset();
  }
}

void ArrayReset() { //配列を空にする
  inputCounter = 0;
  hitPattern = 0;
  for (int i = 0; i < full_Input_Num; i++) {
    array[i] = 0;
  }
}

void MakePattern() {  //配列の集計結果を各桁に変換し，一つの数値にする．
  int digit = 1;
  for (int j = 0; j < full_Input_Num; j++) {
    hitPattern = hitPattern + (array[j] * digit);
    digit *= 10;
  }
}

void ChangePixels() {
  if (hitPattern == 2211) {
    for (int k = 0; k < 4; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 150, 0));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 150, 0));
    }
    for (int k = 4; k < 8; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 50, 5));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 50, 5));
    }
    for (int k = 8; k < 12; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 150, 0));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 150, 0));
    }
    for (int k = 12; k < 16; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 50, 5));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 50, 5));
    }

    pixelsR.show();
    pixelsL.show();

    movieTime = millis()/1000;
    isReset = 0;

  } else if (hitPattern == 1221) {
    for (int k = 0; k < 4; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 30, 0));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 30, 0));
    }
    for (int k = 4; k < 8; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 10, 30));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 10, 30));
    }
    for (int k = 8; k < 12; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 30, 0));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 30, 0));
    }
    for (int k = 12; k < 16; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 10, 30));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 10, 30));
    }

    pixelsR.show();
    pixelsL.show();

    movieTime = millis()/1000;
    isReset = 0;

  } else if (hitPattern == 1121) {
    for (int k = 0; k < 4; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(215, 232, 243));
      pixelsL.setPixelColor(k, pixelsL.Color(215, 232, 243));
    }
    for (int k = 4; k < 8; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(37, 66, 110));
      pixelsL.setPixelColor(k, pixelsL.Color(37, 66, 110));
    }
    for (int k = 8; k < 12; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(215, 232, 243));
      pixelsL.setPixelColor(k, pixelsL.Color(215, 232, 243));
    }
    for (int k = 12; k < 16; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(37, 66, 110));
      pixelsL.setPixelColor(k, pixelsL.Color(37, 66, 110));
    }

    pixelsR.show();
    pixelsL.show();

    movieTime = millis()/1000;
    isReset = 0;

  } else if (hitPattern == 2122) {
    for (int k = 0; k < 4; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 30, 100));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 30, 100));
    }
    for (int k = 4; k < 8; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 105, 180));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 105, 180));
    }
    for (int k = 8; k < 12; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 30, 100));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 30, 100));
    }
    for (int k = 12; k < 16; k++) {
      pixelsR.setPixelColor(k, pixelsR.Color(255, 50, 100));
      pixelsL.setPixelColor(k, pixelsL.Color(255, 105, 180));
    }

    pixelsR.show();
    pixelsL.show();

    movieTime = millis()/1000;
    isReset = 0;

  } else if (hitPattern == 21111) {

    pixelsR.clear();
    pixelsL.clear();
    pixelsR.show();
    pixelsL.show();
  } else {
    return;
  }
}
