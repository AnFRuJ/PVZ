#include<stdio.h>
#include<graphics.h>
#include<time.h>
#include<math.h>
#include"tools.h"

#include<mmsystem.h>
#pragma comment(lib, "winmm.lib")

#define WIN_WIDTH	900
#define WIN_HEIGHT	600
enum { WAN_DOU, XIANG_RI_KUI, ZHI_WU_COUNT };

IMAGE imgBg; //背景图片
IMAGE imgBar;
IMAGE imgCards[ZHI_WU_COUNT];
IMAGE *imgZhiWu[ZHI_WU_COUNT][20];

int curX, curY; //当前鼠标点击后，卡牌在移动过程中的位置
int curZhiWu;//当前鼠标选中的植物,0代表没有选中植物

struct zhiwu {
	int type;//0：没有植物。1：第一种植物 
	int FrameIndex;//序列帧的序列号 
};
struct zhiwu map[3][9];//草坪可以种植植物的map 

struct sunshineBall{
	int x, y;//飘落过程中的位置 
	int frameIndex;//动画帧的帧序号 
	int destY;//飘落的目标位置的y坐标 
	bool used;//是否在使用 
	int timer;//掉落到指定位置后存活的时间
	
	float xoff;//阳光球被拾起后每帧x减少的量 
	float yoff; 
}; 

struct sunshineBall balls[10];
IMAGE imgSunshineBall[29];
int sunshine;

struct zm{
	int x, y;
	int frameIndex;
	int row;
	bool used;
	int speed;
	int blood; 
};
struct zm zms[10];
IMAGE imgZM[18];

struct bullet{
	int x, y;
	int row;
	bool used;
	int speed;
	bool blast;//是否发生爆炸
	int frameIndex; 
};
struct bullet bullets[30];
IMAGE imgBulletNormal;
IMAGE imgBullBlast[4];

bool fileExist(const char* name2) {//检查植物动画一共有多少图片
	//errno_t err;//fopen_s的前置定义
	FILE* fp = fopen(name2, "r");
	if (fp == NULL) {
		return false;
	}
	else {
		fclose(fp);
		return true;
	}
}


void gameInit() {
	loadimage(&imgBg, "res/bg.jpg"); //加载背景图片
	loadimage(&imgBar, "res/bar5.png");

	memset(imgZhiWu, 0, sizeof(imgZhiWu));//对植物数组中的值全部赋为0
	memset(map, 0, sizeof(map));//可种植植物的草坪 
	memset(balls, 0, sizeof(balls));//阳光球
	
	//配置随机种子
	srand(time(NULL));	
	
	//初始化植物卡牌
	char name[64];
	char name2[64];
	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		//生成植物卡牌的文件名
		sprintf(name, "res/Cards/card_%d.png", i + 1);
		loadimage(&imgCards[i], name);

		//生成植物的文件名
		for (int j = 0; j < 20; j++) {
			sprintf(name2, "res/zhiwu/%d/%d.png", i, j+1);
			if (fileExist(name2)) {//判断文件是否存在
				imgZhiWu [i][j] = new IMAGE;
				loadimage(imgZhiWu[i][j], name2);
			}
			else {
				break;
			}
		}
	}
	curZhiWu = 0;
	sunshine = 50;
	
	for(int i = 0; i < 29; i++){
		sprintf(name, "res/sunshine/%d.png", i+1);
		loadimage(&imgSunshineBall[i], name);
	}
	
	//创建游戏图形窗口
	initgraph(WIN_WIDTH, WIN_HEIGHT);

	//设置字体
	LOGFONT f;
	gettextstyle(&f);
	f.lfHeight = 30;
	f.lfWeight = 15;
	strcpy(f.lfFaceName, "Segoe UI Black");
	f.lfQuality = ANTIALIASED_QUALITY;//抗锯齿
	settextstyle(&f);
	setbkmode(TRANSPARENT);
	setcolor(BLACK);
	
	//初始化僵尸数据
	memset(zms, 0, sizeof(zms));
	for(int i = 0; i < 18; i++){
		sprintf(name, "res/zm/%d.png", i+1);
		loadimage(&imgZM[i], name);
	} 
	
	loadimage(&imgBulletNormal, "res/bullets/bullet_normal.png");
	memset(bullets, 0, sizeof(bullets));
	
	//初始化豌豆子弹帧图片数组
	loadimage(&imgBullBlast[3], "res/bullets/bullet_blast.png");
	for(int i = 0; i < 3; i++){
		float k = (i + 1) * 0.2;
		loadimage(&imgBullBlast[i], "res/bullets/bullet_blast.png",
		imgBullBlast[3].getwidth() * k,
		imgBullBlast[3].getheight() *k, true);
	} 
}

void drawZM(){
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	for(int i = 0; i < zmCount; i++){
		if(zms[i].used){
			IMAGE *img = &imgZM[zms[i].frameIndex];
			putimagePNG(zms[i].x, zms[i].y - img->getheight(), img);
		}
	}
}

void updateWindow() {
	BeginBatchDraw();//开始缓冲

	putimage(0, 0, &imgBg);
	//putimage(235, 0, &imgBar);
	putimagePNG(235, 0, &imgBar);//去处PNG图片的黑边

	for (int i = 0; i < ZHI_WU_COUNT; i++) {
		int x = 323 + i * 65;
		int y = 6;
		putimage(x, y, &imgCards[i]);
	}

	//渲染草坪植物 
	for(int i =0; i<3; i++){
		for(int j =0; j<9; j++){
			if(map[i][j].type > 0){
				int x = 256 + j*81;
				int y = 179 + i*102 + 13;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].FrameIndex;//当前植物的动画帧 
				putimagePNG(x,y,imgZhiWu[zhiWuType][index]);
			}
		}
	}
			//渲染拖动卡牌过程
	if (curZhiWu > 0) {
		IMAGE* img = imgZhiWu[curZhiWu - 1][0];
		putimagePNG(curX - img->getwidth()/2, curY - img->getheight()/2, img);
	}
	
	//渲染阳光 
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for(int i = 0; i < ballMax; i++){
		if(balls[i].used || balls[i].xoff){
			IMAGE* img = &imgSunshineBall[balls[i].frameIndex];
			putimagePNG(balls[i].x, balls[i].y, img);
		}
	}
	
	//输出阳光
	char scoreText[8];
	sprintf(scoreText, "%d",sunshine);
	if(sunshine == 0){
		outtextxy(274, 67, scoreText);
	}else if(sunshine < 100){
		outtextxy(268, 67, scoreText); 
	}else if(sunshine < 1000){
		outtextxy(264, 67, scoreText);
	}else{
		outtextxy(258, 67, scoreText);
	}

	drawZM();//渲染僵尸 
	
	//渲染豌豆子弹 
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	for(int i = 0; i < bulletMax; i++){
		if(bullets[i].used){		
			if(bullets[i].blast){
				IMAGE *img = &imgBullBlast[bullets[i].frameIndex];
				putimagePNG(bullets[i].x, bullets[i].y, img);
			}else{
				putimagePNG(bullets[i].x, bullets[i].y, &imgBulletNormal);
			}
		}
	}
	
	EndBatchDraw();
}

void collectSunshine(ExMessage* msg){
	int count = sizeof(balls) / sizeof(balls[0]);
	int w = imgSunshineBall[0].getwidth();
	int h = imgSunshineBall[0].getheight();
	for(int i = 0; i < count; i++){
		if(balls[i].used){
			int x = balls[i].x;
			int y = balls[i].y;
			if(msg->x > x && msg->x < x + w &&
			   msg->y > y && msg->y < y +h){
			   	balls[i].used = false;
			   	mciSendString("play res/sunshine.mp3", 0, 0, 0);
			   	//设置阳光球的偏移量
				float destY = 0;
				float destX = 260;
				float angle = atan((y - destY) / (x - destX));
				balls[i].xoff = 32 * cos(angle);
				balls[i].yoff = 32 * sin(angle);
			}
		}
	}
}

void userClick() {
	ExMessage msg;
	static int AnXia = 0;//是否选中植物 
	if (peekmessage(&msg)) {
		if (msg.message == WM_LBUTTONDOWN) {
			if (msg.x > 323 && msg.x < 323 + 65 * ZHI_WU_COUNT && msg.y > 6 && msg.y < 96) {
				int index = (msg.x - 323) / 65;//第几个植物卡槽，0代表第一个 
				AnXia = 1;
				curZhiWu = index + 1;
				curX = msg.x;
				curY = msg.y;
			}
			if(AnXia == 1 && msg.x > 256 && msg.x < 990 && msg.y > 179 && msg.y < 489){//按下种植物是否在草坪上 
			int row = (msg.y - 179) / 102;
			int col = (msg.x - 256) / 81;
			
				if(map[row][col].type == 0){
					map[row][col].type = curZhiWu;
					map[row][col].FrameIndex = 0;
					AnXia = 0;
					curZhiWu = 0;
				}
			}
			//鼠标是否在收集阳光 
			else{
				collectSunshine(&msg);
			}
		}
		else if(msg.message == WM_MOUSEMOVE && AnXia == 1) {
			curX = msg.x;
			curY = msg.y;
		}
		//右键松开释放植物 
		else if (msg.message == WM_RBUTTONDOWN && AnXia == 1){
			curZhiWu = 0;
			AnXia = 0;
		}
	}
}

void createSunshine(){
	static int count = 0;
	static int fre = 50;
	count++;
	if(count >= fre){
		//阳光在200帧到400帧内随机掉落 
		fre = 100 + rand() % 200;
		count = 0;
	
		//从阳光池中选一个可以使用的阳光
		int ballMax = sizeof(balls) / sizeof(balls[0]);
		int i;
		for(i = 0; i < ballMax && balls[i].used; i++);
		if(i >= ballMax)return;
	
		balls[i].used = true;
		balls[i].frameIndex = 0;
		balls[i].x = 260 + rand() % (900 - 260);
		balls[i].y = 60;
		balls[i].destY = 200+ (rand()%4) * 90;
		balls[i].xoff = 0;
		balls[i].yoff = 0;
	}
}

void updateSunshine(){
	int ballMax = sizeof(balls) / sizeof(balls[0]);
	for(int i = 0; i < ballMax; i++){
		if(balls[i].used){
			balls[i].frameIndex = (balls[i].frameIndex + 1) % 29;
			if(balls[i].timer == 0){
				balls[i].y += 4;
			}
			if(balls[i].y > balls[i].destY){
				balls[i].timer ++;
				if(balls[i].timer > 100){
					balls[i].used = false;
					balls[i].timer = 0;
				}
			}
		}else if(balls[i].xoff){
			//调整弹道 
			float destY = 0;
			float destX = 260;
			float angle = atan((balls[i].y - destY) / (balls[i].x - destX));
			balls[i].xoff = 32 * cos(angle);
			balls[i].yoff = 32 * sin(angle);
			
			balls[i].x -= balls[i].xoff;
			balls[i].y -= balls[i].yoff;
			if(balls[i].y < 0 || balls[i].x < 262){
				balls[i].xoff = 0;
				balls[i].yoff = 0;
				sunshine +=25;
			}
		}
	}
}

void createZM(){
	static int zmFre = 10;
	static int count = 0;
	count++;
	if(count > zmFre){
		count = 0;
		zmFre = rand() % 50 + 300;
		int i;
		int zmMax = sizeof(zms) / sizeof(zms[0]);
		for(i = 0; i < zmMax && zms[i].used; i++);
		if(i < zmMax){
			zms[i].used = true;
			zms[i].x = WIN_WIDTH;
			zms[i].row = rand() % 3;
			zms[i].y = 172 + (1 + zms[i].row) * 100;
			zms[i].speed = 4;
			zms[i].blood = 10;
		}
	}
}

void updateZM(){
	//更新僵尸的位置
	int zmMax = sizeof(zms) / sizeof(zms[0]);
		for(int i = 0; i < zmMax; i++){
	 		if(zms[i].used){
	 			//尝试僵尸走路动作更加流畅
	 			if(zms[i].frameIndex < 2){
	 				zms[i].speed = 0;
				}else if(zms[i].frameIndex < 7){
					zms[i].speed = 1;
				}else if(zms[i].frameIndex < 10){
	 				zms[i].speed = 0;
	 			}else if(zms[i].frameIndex < 11){
	 				zms[i].speed = 1;
				}else if(zms[i].frameIndex < 13){
				 	zms[i].speed = 3;
	 			}else if(zms[i].frameIndex < 14){
				 	zms[i].speed = 2;	
				}else if(zms[i].frameIndex < 15){
				 	zms[i].speed = 1;
				}else{
					zms[i].speed = 0;
				}
				 
	 			zms[i].x -= zms[i].speed;
	 			if(zms[i].x < 120){
	 				//GG
	 				MessageBox(NULL, "over", "over", 0);
	 				exit(0);
				}
			}
		}

	//更新僵尸动画 
	static int count2 = 0;
	count2++;
	if(count2 > 2){
		count2 = 0;
		for(int i = 0; i < zmMax; i++){
			if(zms[i].used){
				zms[i].frameIndex = (zms[i].frameIndex +1 ) % 18;
			}
		}
	}
}

void shoot(){
	int lines[3] = {0};//这一行是否有僵尸，有设置为1 
	int zmCount = sizeof(zms) / sizeof(zms[0]);
	int bulletMax = sizeof(bullets) / sizeof(bullets[0]);
	int dangerX = WIN_WIDTH - imgZM[0].getwidth() + 50;//僵尸走多远后植物开始攻击 
	for(int i =0; i < zmCount; i++){
		if(zms[i].used && zms [i].x < dangerX){
			lines[zms[i].row] = 1;
		}
	}
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 9; j++){
			if(map[i][j].type == WAN_DOU +1 && lines[i]){
				static int count = 0;
				count++;
				if(count > 30){
					count = 0;
					
					int k;
					for(k = 0; k < bulletMax && bullets[k].used; k++);
					if(k < bulletMax){
						bullets[k].used = true;
						bullets[k].row = i;
						bullets[k].speed = 12;
						bullets[k].blast = false;
						bullets[k].frameIndex = 0;
						
						int zwX = 256 + j * 81;
						int zwY = 179 + i * 102 + 14;
						bullets[k].x = zwX + imgZhiWu[map[i][j].type -1][0]->getwidth() - 28;
						bullets[k].y = zwY + 5;
					}
				}
			}
		}
	}
}

void updateBullets(){
	int countMax = sizeof(bullets) / sizeof (bullets[0]);
	for(int i = 0; i < countMax; i++){
		if(bullets[i].used){
			bullets[i].x += bullets[i].speed;
			if(bullets[i].x > WIN_WIDTH){
				bullets[i].used = false;
			}
			if(bullets[i].blast){
				bullets[i].frameIndex++;
				if(bullets[i].frameIndex > 3){
					bullets[i].used = false;
				}
			}
			
		}
	}
}

void collisionCheck(){
	int bCount = sizeof(bullets) / sizeof(bullets[0]);
	int zCount = sizeof(zms) / sizeof(zms[0]);
	for(int i = 0; i < bCount; i++){
		if(bullets[i].used == false ||bullets[i].blast)continue;
		
		for(int k = 0; k < zCount; k++){
			if(zms[k].used == false)continue;
			
			int x1 = zms[k].x + 80;
			int x2 = zms[k].x + 110;
			int x = bullets[i].x;
			if(bullets[i].row == zms[k].row && x > x1 && x < x2){
				zms[k].blood -=1;
				bullets[i].blast = true;
				bullets[i].speed = 0;
				
			}
		}
	}
} 

void updateGame(){
	//渲染草坪植物的动画 
	for(int i = 0; i < 3; i++){
		for(int j = 0; j < 9; j++){
			if(map[i][j].type > 0){
				map[i][j].FrameIndex++;
				int zhiWuType = map[i][j].type - 1;
				int index = map[i][j].FrameIndex;
				if(imgZhiWu[zhiWuType][index] == NULL){
					map[i][j].FrameIndex = 0;
				}
			}
		}
	}
	
	createSunshine();//创建阳光 
	updateSunshine();
	
	createZM();//创建僵尸 
	updateZM();
	
	shoot();//发射豌豆子弹
	updateBullets(); 
	
	collisionCheck();//实现豌豆子弹和僵尸碰撞检测 
}

void startUI(){
	IMAGE imgBg, imgMenu1, imgMenu2;
	loadimage(&imgBg, "res/menu.png");
	loadimage(&imgMenu1, "res/menu2.png");
	loadimage(&imgMenu2, "res/menu1.png");
	int flag = 0;
	while(1){
		BeginBatchDraw();
		putimage(0, 0, &imgBg);
		putimagePNG(474, 75, flag ? &imgMenu2 : &imgMenu1);
		
		ExMessage msg;
		if(peekmessage(&msg)){
			if(msg.message == WM_LBUTTONDOWN &&
				   msg.x > 474 && msg.x < 474 + 300 &&
				   msg.y > 75 && msg.y < 75 + 140){//是否按下 
				flag = 1;
			}
			if(flag){
				if(msg.x < 474 || msg.x > 474 + 300 ||
				   msg.y < 75 || msg.y > 75 + 140){//按下后是否移开开始区域 
					if(msg.message == WM_LBUTTONUP){
						flag = 0;
					}
				}else if (msg.message == WM_LBUTTONUP){
					return;
				}
			}
		}
		

	EndBatchDraw();
	}
	
}

int main(void) {
	gameInit();
	
	startUI();
	
	int timer = 0;
	bool flag = true;
	while (1) {
		userClick();
		timer += getDelay();
		if(timer > 72){
			flag = true;
			timer = 0;
		}
		updateWindow();
		if(flag){
			flag = false;
			updateGame();
		}
	}


	system("pause");
	return 0;
}





