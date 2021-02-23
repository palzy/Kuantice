#include <CircularBuffer.h>
#include <Wire.h>
#include <Adafruit_RGBLCDShield.h>

CircularBuffer<byte,128> random_buffer;
CircularBuffer<byte,16> shaped_buffer;
Adafruit_RGBLCDShield lcd = Adafruit_RGBLCDShield();

unsigned int tm1 = 0, tm2, final_number, result_number;
boolean flg = false;
String result_bits, password;
int k = 1;
int max_random_number = 10;
char letter_array[63] = {
  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
  'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l',
  'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x',
  'y', 'z', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
  'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V',
  'W', 'X', 'Y', 'Z', '\0'};

void signal_check() {
  if (digitalRead(2) == HIGH) {
    flg = !flg;
  }
}

void write_buffer() {
  shaped_buffer.clear();
  while(!shaped_buffer.isFull()) {
    random_buffer.clear();
    while(!random_buffer.isFull()) {
      //単位期間に信号があるかチェック
      while(1) {
        signal_check();
        tm2 = micros(); //1000で1ms
        if ((tm2 - tm1) > 800) {
          tm1 = tm2;
          break;
        }
      }
      //信号の有無をバッファに追記
      if(flg == true) {
        random_buffer.push(1);
        flg = !flg;
      }
      else {
        random_buffer.push(0);
      }
    }
    //ノイマン処理
    for (decltype(random_buffer)::index_t i = 0; i < random_buffer.size(); i=i+2) {
      if (random_buffer[i] + random_buffer[i+1] == 1) {
        if(random_buffer[i] == 0) {
          shaped_buffer.push(0);
        }
        else {
          shaped_buffer.push(1);
        }
      }
    }
  }
  //文字列に変換
  result_bits = "";
  for (decltype(shaped_buffer)::index_t i = 0; i < shaped_buffer.size(); i++) {
    result_bits = result_bits + (String)shaped_buffer[i];
  }
  //ビット毎に計算して数字を求める
  k = 1;
  result_number = 0;
  for (uint8_t b = 0; b < result_bits.length(); b++) {
    if (result_bits[b] != '0') {
      result_number = k + result_number;
    }
    k = k * 2;
  }
}

void setup() {
  pinMode(2, INPUT_PULLUP);
  lcd.begin(16, 2);
  lcd.setCursor(0, 0);
  lcd.print("Kuantice");
  lcd.setCursor(0, 1);
  lcd.print("L:pw D:4n U:1n");
  
}

void loop() {
  uint8_t buttons = lcd.readButtons();
  if (buttons) {
    lcd.clear();
    password = "";
    lcd.setCursor(0, 0);
    if (buttons & BUTTON_LEFT) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Password");
      lcd.setBacklight(3);
      for (int b = 0; b < 12; b++) {
        write_buffer();
        final_number = result_number % 62;
        password = password + letter_array[final_number];
      }
      lcd.setCursor(0, 1);
      lcd.print(password);
    }
    if (buttons & BUTTON_DOWN) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Four Numbers");
      lcd.setCursor(0, 1);
      lcd.setBacklight(5);
      for (int i = 0; i < 4; i++) {
        write_buffer();
        final_number = result_number % max_random_number;
        lcd.print(final_number);
        lcd.print(' ');
      }
    }
    if (buttons & BUTTON_RIGHT) {
      lcd.setCursor(0, 0);
      lcd.print("No Function");
      lcd.setBacklight(7);
    }
    if (buttons & BUTTON_UP) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("One Number");
      lcd.setBacklight(2);
      write_buffer();
      final_number = result_number % max_random_number;
      if (final_number == 0) {
        final_number = max_random_number;
      }
      lcd.setCursor(0, 1);
      lcd.print(final_number);
    }
  }
}
