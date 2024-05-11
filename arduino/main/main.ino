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

#define mishitPreventTime 150

#define maxInputNum 5  //最大入力回数

#define pixel_Num 16  //片側合計ピクセル数
#define pixel_Pin_R 5  //ピクセル信号ピン定義  
#define pixel_Pin_L 4

Adafruit_NeoPixel pixelsR(pixel_Num, pixel_Pin_R, NEO_GRB + NEO_KHZ800);
Adafruit_NeoPixel pixelsL(pixel_Num, pixel_Pin_L, NEO_GRB + NEO_KHZ800);


//変数宣言//
int sensorState_D1; //タタコンセンサ入力ピン状態（ドン）
int sensorState_C1; //タタコンセンサ入力ピン状態（カッ）
int sensorState_D2;
int sensorState_C2;
int sensorState_D3;
int sensorState_C3;

int hitPattern; //入力結果
int inputtedDatas[maxInputNum];  //入力パターン代入配列
int inputCounter = 0; //入力回数
int lastInputTime = 0; //最終入力時刻
int lastStateChangedTime = 0; //最終システムフェーズ変更時間
int curTime = 0;  //システム経過時間
bool isSleepModeActive = true;  //システムスリープ状態


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

  //タタコン1_ドン
  if (sensorState_D1 == HIGH) {   //入力なし
    digitalWrite(led_R1, LOW); 
  } else {                        //入力あり  
    digitalWrite(led_R1, HIGH); 
    Serial.write(1);  //processingにデータ送信
    ArrayInput(1);  //NeoPixel操作用情報入力
    
    lastInputTime = millis()/1000;

    //バチが離れるまで待機
    while (digitalRead(sensor_D1)) {}

    delay(mishitPreventTime);  //チャタリング防止
  }


  //タタコン2_ドン
  if (sensorState_D2 == HIGH) {     //入力なし
    digitalWrite(led_R2, LOW);  
  } else {                          //入力あり
    digitalWrite(led_R2, HIGH); // If the sensorState is LOW, Taiko is triggered.
    Serial.write(2);  //processingにデータ送信
    ArrayInput(2);  //NeoPixel操作用情報入力
    
    lastInputTime = millis()/1000;

    //バチが離れるまで待機
    while (digitalRead(sensor_D2)) {}

    delay(mishitPreventTime);  //チャタリング防止
  }


  //タタコン3_ドン（入力確定用）
  if (sensorState_D3 == HIGH) {    //入力なし
    digitalWrite(led_R3, LOW); 
  } else {                          //入力あり
    digitalWrite(led_R3, HIGH); 
    Serial.write(3);  //processingにデータ送信

    MakePattern();  //叩いた結果を算出
    ArrayReset(); //配列初期化
    int resultChange = TryChangePixels(); //提灯カラー変更

    if(resultChange == 1) {
      isSleepModeActive = false;
    }else if(resultChange == 0){
      isSleepModeActive = true;
    }

    lastStateChangedTime = millis()/1000;
    lastInputTime = millis()/1000;

    //バチが離れるまで待機
    while (digitalRead(sensor_D3)) {}

    delay(mishitPreventTime);  //チャタリング防止
  }

  curTime = millis()/1000;

  //指定時間経過時，システムスリープ処理を実行
  if (curTime - lastStateChangedTime >= 60 && curTime - lastInputTime >= 20 && !isSleepModeActive) {
    ArrayReset();
    Serial.write(3);
    
    //システムスリープコードを生成
    Serial.write(1);
    Serial.write(1);
    Serial.write(1);
    Serial.write(1);
    Serial.write(2);
    Serial.write(3);
    ArrayInput(1);
    ArrayInput(1);
    ArrayInput(1);
    ArrayInput(1);
    ArrayInput(2);

    MakePattern();
    ArrayReset();
    TryChangePixels();

    isSleepModeActive = true;
  }
}



//入力データを配列に保持するメソッド//
void ArrayInput (int sensor_num) {
  inputtedDatas[inputCounter] = sensor_num;
  inputCounter ++;

  //入力回数が最大入力回数を超えた場合に入力リセット
  if (inputCounter > maxInputNum) {
    ArrayReset();
  }
}


//入力データ保持用配列をリセットするメソッド//
void ArrayReset() { //配列を空にする
  inputCounter = 0;
  hitPattern = 0;
  for (int i = 0; i < maxInputNum; i++) {
    inputtedDatas[i] = 0;
  }
}

//入力データの集計結果を変数に変換して，入力パターンを生成するメソッド//
void MakePattern() {
  int digit = 1;
  for (int j = 0; j < maxInputNum; j++) {
    hitPattern = hitPattern + (inputtedDatas[j] * digit);
    digit *= 10;
  }
}

//pixelLEDの更新処理//
int TryChangePixels() {
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

    return 1;

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

    return 1;

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

    return 1;

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

   return 1;

  } else if (hitPattern == 21111) {
    pixelsR.clear();
    pixelsL.clear();
    pixelsR.show();
    pixelsL.show();

    return 0;

  } else {
    return -1;
  }
}
