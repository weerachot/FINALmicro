#include <TM1638plus.h>

#define STB 18
#define CLK 19
#define DIO 21
#define CancelSW 0

TM1638plus tm(STB, CLK, DIO, true);

int money = 0;
const int COUPON_PRICE = 275;
// ลำดับค่าเงินทอนจากซ้ายไปขวา: 1000, 500, 100, 50, 20, 10, 5, 1
int note[8] = {1000, 500, 100, 50, 20, 10, 5, 1};

void showChange(int change) {
  int n[8];
  uint8_t ledMask = 0; // ตัวแปรสำหรับเก็บสถานะ LED ทั้ง 8 ดวง (Bitmask)

  // คำนวณจำนวนแบงก์/เหรียญ
  for (int i = 0; i < 8; i++) {
    n[i] = change / note[i];
    change %= note[i];
    
    // ถ้าหลักนั้นมีการทอน (n[i] > 0) ให้สั่งเปิด LED ประจำหลักนั้น
    if (n[i] > 0) {
      ledMask |= (1 << i); // ตั้งค่าบิตที่ i ให้เป็น 1
    }
  }

  // แสดงผลบนหน้าจอและเปิด LED
  for (int i = 0; i < 8; i++) {
    // แสดงตัวเลขจำนวนแบงก์/เหรียญบน Segment
    tm.displayASCII(i, n[i] + '0', CommonData::DecPointOff);
  }
  
  // สั่งให้ LED ติดสว่างตามหลักที่มีการทอนเงิน
  tm.setLEDs(ledMask); 
}

void setup() {
  pinMode(CancelSW, INPUT_PULLUP);
  tm.displayBegin();
  tm.reset();
  tm.brightness(4);

  // Step 1: แสดงรหัสนักศึกษา (6 คะแนน)
  tm.displayText("b6732493 "); 
  delay(3000);
  tm.reset();
}

void loop() {
  // แสดงจำนวนเงินที่ป้อนเข้ามาปัจจุบัน
  tm.displayIntNum(money);

  byte b = tm.readButtons();
  if (b > 0) {
    // Step 2: ป้อนเงินจากสวิตช์ S1-S8 (5 คะแนน)
    if (b & 0x01) money += 1000;
    if (b & 0x02) money += 500;
    if (b & 0x04) money += 100;
    if (b & 0x08) money += 50;
    if (b & 0x10) money += 20;
    if (b & 0x20) money += 10;
    if (b & 0x40) money += 5;
    if (b & 0x80) money += 1;
    delay(250); 
  }

  // ระบบ Cancel
  if (digitalRead(CancelSW) == LOW) {
    tm.displayText("CANCEL  ");
    delay(1000);
    money = 0;
    tm.setLEDs(0x00); // ปิด LED ทั้งหมด
    tm.reset();
  }

  // Step 3: ระบบทอนเงินสมบูรณ์ (5 คะแนน)
  if (money >= COUPON_PRICE) {
    int change = money - COUPON_PRICE;

    tm.displayText("COUPON  "); 
    delay(2000);

    tm.displayIntNum(change); // แสดงยอดเงินทอนรวม
    delay(2000);

    showChange(change); // แสดงจำนวนแบงก์ และเปิด LED ประจำหลัก
    delay(5000);

    money = 0;
    tm.setLEDs(0x00); // รีเซ็ต LED หลังจบการทำงาน
    tm.reset();
  }
}