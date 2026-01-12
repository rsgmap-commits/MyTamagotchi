#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <conio.h>
#include <windows.h>

// --- 遊戲平衡設定 ---
#define WIN_AGE 15      // 目標年齡
#define TICK_RATE 800   // 遊戲速度 (毫秒)

typedef struct {
    char name[20];
    int hunger;     // 0-100 (越低越好)
    int happiness;  // 0-100 (越高越好)
    int energy;     // 0-100 (越高越好)
    int age;        // 年齡
    int isAlive;
    int isGrowing;  // 是否正在生長
} Pet;

// --- 視窗工具 ---
void gotoxy(int x, int y) {
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
}

void setColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void hideCursor() {
    HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO info;
    info.dwSize = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo(consoleHandle, &info);
}

// --- 繪圖工具 ---
void drawBar(int x, int y, char* label, int value, int type) {
    // type: 0=越高越好(綠), 1=越低越好(紅)
    gotoxy(x, y);
    setColor(15);
    printf("%s", label);
    
    int barColor;
    if (type == 0) { // 快樂、體力 (越高越好)
        if (value > 70) barColor = 10;      // 綠 (好)
        else if (value > 40) barColor = 14; // 黃 (普)
        else barColor = 12;                 // 紅 (差)
    } else { // 飢餓 (越低越好)
        if (value < 30) barColor = 10;      // 綠 (好)
        else if (value < 60) barColor = 14; // 黃 (普)
        else barColor = 12;                 // 紅 (差)
    }

    gotoxy(x + 12, y);
    setColor(barColor);
    printf("[");
    for (int i = 0; i < 10; i++) {
        if (i < value / 10) printf("#");
        else printf(" ");
    }
    printf("] ");
    setColor(15);
    printf("%3d%%", value);
}

void drawPet(Pet p) {
    gotoxy(15, 6);
    if (p.isGrowing) {
        setColor(10); // 綠色
        printf("★ 狀態完美！生長中 (Age: %d/%d) ★   ", p.age, WIN_AGE);
    } else {
        setColor(12); // 紅色
        printf("⚠ 狀態普通，停止生長 (Age: %d/%d) ⚠   ", p.age, WIN_AGE);
    }

    setColor(15);
    // 根據狀態畫圖
    if (!p.isAlive) {
        // 死亡圖會在結尾處理，這裡畫暈倒
        gotoxy(15, 8);  printf("   ( X . X )            ");
        gotoxy(15, 9);  printf("    =======             ");
        gotoxy(15, 10); printf("                        ");
    }
    else if (p.energy < 20) {
        gotoxy(15, 8);  printf("   ( - . - ) Zzz...     ");
        gotoxy(15, 9);  printf("   ( >   < )            ");
        gotoxy(15, 10); printf("    UU---UU             ");
    } else if (p.hunger > 60 || p.happiness < 40) { 
        gotoxy(15, 8);  printf("   /\\_ /\\               ");
        gotoxy(15, 9);  printf("  ( T . T )  嗚嗚...    ");
        gotoxy(15, 10); printf("   >  ^  <              ");
    } else {
        gotoxy(15, 8);  printf("   /\\_ /\\               ");
        gotoxy(15, 9);  printf("  ( ^ . ^ )  喵！       ");
        gotoxy(15, 10); printf("   >  ^  <              ");
    }
}

void showMessage(char* msg) {
    gotoxy(5, 18);
    setColor(14);
    printf("%-50s", msg); 
}

int clamp(int val) {
    if (val > 100) return 100;
    if (val < 0) return 0;
    return val;
}

// --- 主程式 ---
int main() {
    system("chcp 65001 > nul"); 
    system("cls");
    hideCursor();

    // === 關鍵修改 1：初始數值 ===
    // 飢餓 55 (有點餓), 快樂 45 (有點悶)
    // 玩家如果不動作，這些數值會馬上變得更糟，絕對不會長大
    Pet myPet = {"Kitty", 55, 45, 80, 0, 1, 0}; 
    
    clock_t lastUpdate = clock();
    int running = 1;
    int gameResult = 0; // 0=進行, 1=死, 2=贏
    char message[60] = "快點照顧牠！不然長不大喔！";

    while (running && myPet.isAlive) {
        
        // --- 邏輯更新 ---
        if (clock() - lastUpdate > TICK_RATE) { 
            // 自然衰退
            myPet.hunger += 5;      
            myPet.energy -= 2;
            myPet.happiness -= 4;

            // === 關鍵修改 2：嚴格的成長條件 ===
            // 必須：飢餓 < 40 (吃飽飽) 且 快樂 > 60 (心情好) 且 體力 > 30
            // 初始值是 (55, 45)，所以一開始絕對不會長大！
            if (myPet.hunger < 40 && myPet.happiness > 60 && myPet.energy > 30) {
                myPet.age += 1;
                myPet.isGrowing = 1;
            } else {
                myPet.isGrowing = 0;
            }

            // 判斷勝利
            if (myPet.age >= WIN_AGE) {
                gameResult = 2;
                running = 0;
            }

            // 判斷死亡 (掛機懲罰：太餓或太憂鬱直接死)
            if (myPet.hunger >= 100 || myPet.happiness <= 0 || myPet.energy <= 0) {
                myPet.isAlive = 0;
                gameResult = 1;
                if(myPet.hunger >= 100) sprintf(message, "餓死了...");
                else if(myPet.happiness <= 0) sprintf(message, "憂鬱而終...");
                else sprintf(message, "過勞死了...");
            }
            
            lastUpdate = clock();
        }

        // --- 繪圖 ---
        drawBar(5, 2, "飢餓度", myPet.hunger, 1);     // 1=越低越好
        drawBar(5, 3, "快樂度", myPet.happiness, 0);  // 0=越高越好
        drawBar(5, 4, "精力度", myPet.energy, 0);
        drawPet(myPet);
        
        gotoxy(5, 15); setColor(11); printf("---------------------------------------");
        gotoxy(5, 16); printf("[F]餵食  [P]玩耍  [S]睡覺  [Q]離開");
        
        showMessage(message);

        // --- 輸入 ---
        if (_kbhit()) { 
            char key = _getch(); 
            if (key >= 'a' && key <= 'z') key -= 32;

            switch (key) {
                case 'F': 
                    if (myPet.hunger > 0) { 
                        myPet.hunger -= 15; 
                        myPet.happiness += 2; 
                        sprintf(message, "餵食！(飢餓-15)"); 
                    }
                    break;
                case 'P': 
                    if (myPet.energy > 15) { 
                        myPet.happiness += 15; 
                        myPet.energy -= 10; 
                        myPet.hunger += 5; 
                        sprintf(message, "玩耍！(快樂+15, 體力-10)"); 
                    } else sprintf(message, "太累了，不想玩...");
                    break;
                case 'S': 
                    myPet.energy = 100; 
                    myPet.hunger += 15; // 睡覺會變很餓
                    sprintf(message, "睡飽了！(精力全滿，肚子好餓)");
                    break;
                case 'Q':
                    running = 0;
                    break;
            }
            myPet.hunger = clamp(myPet.hunger);
            myPet.happiness = clamp(myPet.happiness);
            myPet.energy = clamp(myPet.energy);
        }
        Sleep(50);
    }

    // --- 結局 ---
    system("cls");
    gotoxy(0, 5);
    if (gameResult == 2) {
        setColor(14);
        printf("      ★ ★ ★ 恭喜通關！ ★ ★ ★\n");
        printf("      %s 成功長大，進化成貓皇！\n\n", myPet.name);
        printf("             /\\_____/\\ \n");
        printf("            /  o   o  \\ \n");
        printf("           ( ==  ^  == ) \n");
        printf("            )         ( \n");
        printf("           (           ) \n");
    } else if (gameResult == 1) {
        setColor(12);
        printf("      === GAME OVER ===\n");
        printf("      死因: %s\n\n", message);
        printf("           ( X . X )\n");
    }
    
    setColor(7);
    gotoxy(0, 20);
    system("pause");
    return 0;
}