#include <UnbufferedSerial.h>
#include <mbed.h>
#include <stdio.h>

#define BAUD 115200
static UnbufferedSerial esp(PA_9, PA_10);
DigitalOut led(LED1);

char buf[2];
char str[64];
asm(".global _printf_float"); // printfでfloatを使えるようにする

struct Gamepad
{
    // データを扱う構造体
    bool button[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int dpad = 0;
    float rstick[2] = {0.0, 0.0};
    float lstick[2] = {0.0, 0.0};
} joy;

void read_esp()
{
    // 割り込み関数
    static int i = 0;
    esp.read(buf, 2);
    // 1文字ずつ読む
    if (i >= 64)
    {
        i = 0;
        memset(str, '\0', sizeof(str));
    }
    if (buf[0] == '\n')
    { // 改行コードが来たら
        i = 0;
        // printf("recv: %s\n", str);
        // 受信したデータを表示
        int data[6];
        sscanf(str, "%d %d %d %d %d %d\n", &data[0], &data[1], &data[2], &data[3], &data[4], &data[5]);
        joy.rstick[0] = data[0] / 512.0;
        joy.rstick[1] = data[1] / 512.0;
        joy.lstick[0] = data[2] / 512.0;
        joy.lstick[1] = data[3] / 512.0;
        joy.dpad = data[4];
        for (int j = 0; j < 12; j++)
        {
            joy.button[j] = (data[5] & (1 << j)) >> j;
        }
        memset(str, '\0', sizeof(str));
    }
    else
    { // 改行コードが来るまで
        str[i] = buf[0];
        i++;
    }
}

void init()
{

    esp.baud(BAUD);                                 // 通信速度の指定
    esp.attach(&read_esp, UnbufferedSerial::RxIrq); // 割り込み関数の割当
}

int main(void)
{
    init();
    while (1)
    {
        printf("%5.2f %5.2f %5.2f %5.2f %d %d %d %d %d %d %d %d %d\n",
               joy.rstick[0],  // 左スティックX(左-1.0~1.0右)
               joy.rstick[1],  // 左スティックY(上-1.0~1.0下)
               joy.lstick[0],  // 右スティックX(左-1.0~1.0右)
               joy.lstick[1],  // 右スティックY(上-1.0~1.0下)
               joy.dpad,       // 十字キー(1:上, 2:下, 4:右, 8:左)
               joy.button[0],  // ×
               joy.button[1],  // ○
               joy.button[2],  // ⬜︎
               joy.button[3],  // △
               joy.button[4],  // L1
               joy.button[5],  // R1
               joy.button[6],  // L2
               joy.button[7]); // R2
        wait_us(10 * 1000);
    }
    return 0;
}
