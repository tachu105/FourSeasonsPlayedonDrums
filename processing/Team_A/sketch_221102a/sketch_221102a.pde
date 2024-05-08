import processing.video.*; //ライブラリをインポート
import processing.serial.*;
import ddf.minim.*;

Minim minim;
AudioSample taiko1,taiko2,taiko3,kettei;

Movie myMovie1;//Movie変数「myMovie」を宣言
Movie myMovie2;
Movie myMovie3;
Movie myMovie4;

int M1State = 0;
int M2State= 0;
int M3State=0;
int M4State=0;


Serial serial;
int IntData;  //シリアル通信の受信値を代入する変数

int full_Input_Num = 5;  //最大入力回数
int inputCounter = 0;
int array[] = new int[full_Input_Num + 1];
int hitPattern = 0;
int changeMovie = 0;


void setup() {
  fullScreen();
  
  myMovie1 = new Movie(this, "akiloop2.mp4");
  myMovie2 = new Movie(this, "compoloop2.mp4");
  myMovie3 = new Movie(this, "fuyuloop2.mp4");
  myMovie4 = new Movie(this, "sakuraloop.mp4");

  //バックグラウンドで再生//
   myMovie1.play();
  myMovie2.play();
  myMovie3.play();
  myMovie4.play();

  serial = new Serial(this, "COM4", 9600);  //シリアル通信開始
  
  minim = new Minim(this);  //minim開始
  
  //音楽ファイル取得//
  taiko1 = minim.loadSample("get.mp3");
  taiko2 = minim.loadSample("get3.mp3");
  taiko3 = minim.loadSample("get5.mp3");
  kettei = minim.loadSample("koto.mp3");
  
  //音量調整 +6～-80//
  taiko1.setGain(0);
  taiko2.setGain(0);
  taiko3.setGain(6);
  kettei.setGain(0);
}

void draw() {
  background(0);
  //特定の入力値の場合，指定の動画でフレームを更新//
  if (changeMovie == 2211) {
    myMovie1.play();
    myMovie2.stop();
    myMovie3.stop();
    myMovie4.stop();
    image(myMovie1, 30, 230);
  } else if (changeMovie == 1221) {
    myMovie1.stop();
    myMovie2.play();
    myMovie3.stop();
    myMovie4.stop();
    image(myMovie2, 30, 230);
  } else if (changeMovie == 1121) {
    myMovie1.stop();
    myMovie2.stop();
    myMovie3.play();
    myMovie4.stop();
    image(myMovie3, 30, 230);
  } else if (changeMovie == 2122) {
    myMovie1.stop();
    myMovie4.play();
    myMovie3.stop();
    myMovie2.stop();
    image(myMovie4, 30, 230);
  }
  else if (changeMovie == 21111) {
    myMovie1.stop();
    myMovie4.stop();
    myMovie3.stop();
    myMovie2.stop();
  }
}


void movieEvent(Movie m) {
  m.read();
}


void serialEvent(Serial p) {
  if (serial.available() >0) 
  {
    IntData = p.read();

    if (IntData == 1) {
      ArrayInput(IntData);
      taiko1.trigger();
      println(IntData);
    } else if (IntData == 2) {
      ArrayInput(IntData);
      taiko2.trigger();
      println(IntData);
    } else if (IntData == 3) {
      MakePattern();
      taiko3.trigger();
      if ((hitPattern == 2211 || hitPattern == 1221 || hitPattern == 1121 || hitPattern == 2122 || hitPattern == 21111) && changeMovie != hitPattern) {  
        changeMovie = hitPattern;  //  特定のパターンの場合のみ，動画変更用変数に代入
        kettei.trigger();
    }
      println(IntData);
      println(hitPattern);
      ArrayReset();
    }
  }
}


void ArrayInput(int Data) {
  array[inputCounter] = Data;
  inputCounter ++;

  if (inputCounter > full_Input_Num) {
    ArrayReset();
  }
}

void ArrayReset() {
  inputCounter = 0;
  hitPattern = 0;
  for (int i = 0; i < full_Input_Num + 1; i++) {
    array[i] = 0;
  }
}

void MakePattern() {
  int digit = 1;
  for (int j = 0; j < full_Input_Num + 1; j++) {
    hitPattern = hitPattern + (array[j] * digit);
    digit *= 10;
  }
}


void stop(){  //minim終了
  taiko1.close();
  taiko2.close();
  taiko3.close();
  kettei.close();
  
  minim.stop();
  super.stop();
}
