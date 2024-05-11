import processing.video.*; //動画再生用ライブラリ
import processing.serial.*;  //シリアル通信用ライブラリ
import ddf.minim.*;  //音声再生用ライブラリ

//音声再生クラス
Minim minim;

//音声用変数
AudioSample taiko1Sound,taiko2Sound,taiko3Sound;
AudioSample confirmSound;

//映像用変数
Movie myMovie1, myMovie2, myMovie3, myMovie4;
Movie currentMovie;

//シリアル通信クラス
Serial serial;  

int receivedData;  //シリアル通信の受信値を代入する変数
int maxInputNum = 5;  //最大入力回数
int inputCounter = 0;  //入力回数カウンター
int inputtedDataArray[] = new int[maxInputNum + 1];  //入力データ保持用配列
int newHitPattern = 0;  //新規入力パターン
int currentMoviePattern = 0;  //現在の入力パターン


void setup() {
  fullScreen();
  
  //映像データ取得//
  //映像データのパスは，Processing.exeからの相対パスを指定すること
  myMovie1 = new Movie(this, "datas/movies/akiloop2.mp4");
  myMovie2 = new Movie(this, "datas/movies/compoloop2.mp4");
  myMovie3 = new Movie(this, "datas/movies/fuyuloop2.mp4");
  myMovie4 = new Movie(this, "datas/movies/sakuraloop.mp4");

  serial = new Serial(this, "COM4", 9600);  //シリアル通信開始

  minim = new Minim(this);  //minim開始
  
  //音楽ファイル取得//
  taiko1Sound = minim.loadSample("datas/sounds/get.mp3");
  taiko2Sound = minim.loadSample("datas/sounds/get3.mp3");
  taiko3Sound = minim.loadSample("datas/sounds/get5.mp3");
  confirmSound = minim.loadSample("datas/sounds/koto.mp3");
  
  //音量調整 +6～-80//
  taiko1Sound.setGain(0);
  taiko2Sound.setGain(0);
  taiko3Sound.setGain(6);
  confirmSound.setGain(0);
}


void draw() {
  background(0);
  
  if(currentMovie != null){
    image(currentMovie, 30, 230);  //映像を表示
  }
}


//映像のフレームの更新処理//
void movieEvent(Movie m) {
  m.read();
}


//新規シリアル通信を受け取ったときの処理//
void serialEvent(Serial p) {
  if (serial.available() >0) 
  {
    receivedData = p.read();

    if (receivedData == 1) {  //太鼓1の入力を受信したとき
      ArrayInput(receivedData);  //受信データを記録用配列に保持
      taiko1Sound.trigger();  //太鼓1の音声を再生
      println(receivedData);
    } else if (receivedData == 2) {  //太鼓2の入力を受信したとき
      ArrayInput(receivedData);
      taiko2Sound.trigger();
      println(receivedData);
    } else if (receivedData == 3) {  //決定用太鼓の入力を受信したとき
      MakePattern();  //記録した入力データを変換
      taiko3Sound.trigger();
      
      //入力パターンに変更があるときのみ，動画を差し替え
      if ((newHitPattern == 2211 || newHitPattern == 1221 || newHitPattern == 1121 || newHitPattern == 2122 || newHitPattern == 21111) && currentMoviePattern != newHitPattern) {  
        currentMoviePattern = newHitPattern;  //動画変更用変数に代入
        confirmSound.trigger();
        SwitchMovie(currentMoviePattern);
      }
      
      println(receivedData);
      println(newHitPattern);
      ArrayReset();  //入力データをリセット
    }
  }
}



//投影する映像を切り替えるメソッド//
void SwitchMovie(int moviePattern){
  switch (moviePattern) {
    case 2211: // 秋映像再生
      myMovie1.play();
      myMovie2.stop();
      myMovie3.stop();
      myMovie4.stop();
      currentMovie = myMovie1;
      break;
  
    case 1221: // 夏映像再生
      myMovie1.stop();
      myMovie2.play();
      myMovie3.stop();
      myMovie4.stop();
      currentMovie = myMovie2;
      break;
  
    case 1121: // 冬映像再生
      myMovie1.stop();
      myMovie2.stop();
      myMovie3.play();
      myMovie4.stop();
      currentMovie = myMovie3;
      break;
  
    case 2122: // 春映像再生
      myMovie1.stop();
      myMovie2.stop();
      myMovie3.stop();
      myMovie4.play();
      currentMovie = myMovie4;
      break;
  
    case 21111: // システムスリープ
      myMovie1.stop();
      myMovie2.stop();
      myMovie3.stop();
      myMovie4.stop();
      currentMovie = null;
      break;
  
    default:
      break;
  }
}



//受信データ記録用配列に値を追加するメソッド//
void ArrayInput(int Data) {
  inputtedDataArray[inputCounter] = Data;
  inputCounter ++;

  //入力データ数が最大入力回数を超えた場合はリセット
  if (inputCounter > maxInputNum) {
    ArrayReset();
  }
}


//受信データ記録用配列をクリアするメソッド//
void ArrayReset() {
  inputCounter = 0;
  newHitPattern = 0;
  for (int i = 0; i < maxInputNum + 1; i++) {
    inputtedDataArray[i] = 0;
  }
}


//記録用配列のデータをintデータに変換するメソッド//
void MakePattern() {
  int digit = 1;
  for (int j = 0; j < maxInputNum + 1; j++) {
    newHitPattern = newHitPattern + (inputtedDataArray[j] * digit);
    digit *= 10;
  }
}


void stop(){  //minim終了
  taiko1Sound.close();
  taiko2Sound.close();
  taiko3Sound.close();
  confirmSound.close();
  
  minim.stop();
  super.stop();
}
