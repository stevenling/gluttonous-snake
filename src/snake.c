#include "LPC17xx.h"                                                  
#include "stdlib.h"
#include "time.h"

// 宏定义
// -------------------
#define CCLK 100000000                                                              
#define PCLK0 CCLK/4
#define BEEP            ( 0xFF<< 0)  
#define BPEE            ( 0xFF<< 4)
#define MAX_length 10

// 函数
// -------------------------------------
void Show();
void Clear();
void Delay (uint32_t ulTime);
void GPIOInit(void);
void EintAllInit(void);
void Time0Init (void);
void CreateSnake();
void CreateFood();
void ShowScore(uint8_t a[8][8]);
void ShowLevel(uint8_t a[8][8]);
void SnakeDie();
int  IsMove(int n);
void SnakeMove();
void Clear();

// 全局变量
// -----------------
int snakeLength = 3; // 蛇的长度
int snakeFront = 0;  // 蛇头
int snakeRear = 2;   // 蛇尾
int Die = 0;
int score = 0;
int count = 0;
int dieCount = 0; // 死亡次数
char dirs = 'n';

// ---------------------------
void Delay (uint32_t ulTime)
{
	uint32_t i;
	i = 0;
	while (ulTime--) 
	{
		for (i = 0; i < 1000; i++);
	}
}

// -----------------
void GPIOInit(void)
{
	LPC_GPIO0->FIODIR  = LPC_GPIO0->FIODIR | BPEE; // C1~C7 P0.4 - P0.11
	LPC_GPIO2->FIODIR  = LPC_GPIO2->FIODIR| BEEP;  // R1~R7 P0.0 - P0.7
}

// ------------------------------------
void EintAllInit(void)
{
	LPC_PINCON->PINSEL4 =(LPC_PINCON->PINSEL4 & ~(0xFF << 20))|(0x55<<20);
	LPC_SC->EXTMODE     |=(1<<0);                               
	LPC_SC->EXTPOLAR    &=~(1<<0);                                 
	LPC_SC->EXTINT |=(1<<0);
	NVIC_EnableIRQ(EINT0_IRQn);

	LPC_SC->EXTMODE     |=(1<<1);                                    
	LPC_SC->EXTPOLAR    &=~(1<<1);                                      
	LPC_SC->EXTINT |=(1<<1) ;
	NVIC_EnableIRQ(EINT1_IRQn);

	LPC_SC->EXTMODE     |=(1<<2);                                      
	LPC_SC->EXTPOLAR    &=~(1<<2);                                      
	LPC_SC->EXTINT |=(1<<2) ;
	NVIC_EnableIRQ(EINT2_IRQn);

	LPC_SC->EXTMODE     |=(1<<3);                                      
	LPC_SC->EXTPOLAR    &=~(1<<3);                                    
	LPC_SC->EXTINT |=(1<<3)	;
	NVIC_EnableIRQ(EINT3_IRQn);
}

// --------------------
void Time0Init (void)
{
	LPC_TIM0->CTCR = 0;
	LPC_TIM0->PR = 0;
	LPC_TIM0->TC = 0;
	LPC_TIM0->MR0 = 0.001 * PCLK0 - 1;
	LPC_TIM0->MCR = 0x03;
	LPC_TIM0->TCR = 1;
	NVIC_EnableIRQ(TIMER0_IRQn);
}

// --------------------
uint8_t SnakeMap[8][8]=
{								
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,0,0},
};

// ---------
struct Snake        
{
	int x[MAX_length];
	int y[MAX_length];

}snake;

// 初始化蛇
// ------------
void CreateSnake()
{
	int i;
	Die = 0;
	dirs = 'n';
	snakeLength = 3;
	snakeFront = 0;
	snakeRear = 2;
	score = 0;
	for(i = 0; i < 10; i++)
	{
		snake.x[i] = 0;
		snake.y[i] = 0;
	}
	snake.x[0] = 3;
	snake.x[1] = 3;
	snake.x[2] = 3;
	snake.y[0] = 3;
	snake.y[1] = 2;
	snake.y[2] = 1;
	count = 0;
	Show();
}
struct Food
{
	int x;
	int y;
}food;

// 生成食物
// ------------
void CreateFood()
{
	SnakeMap[0][0] = 0;
	// 如果是 1 表明是亮的，必须继续找
	do
	{
		food.x = rand() % 8;
		food.y = rand() % 8;
	}while(SnakeMap[food.x][food.y] == 1); 

	SnakeMap[food.x][food.y] = 1;
	count++;
}

// ---------------------
uint8_t SnakeDieMap[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,1,0,0,0,0,1,0},
	{0,0,1,0,0,1,0,0},
	{0,0,0,1,1,0,0,0},
	{0,0,0,1,1,0,0,0},
	{0,0,1,0,0,1,0,0},
	{0,1,0,0,0,0,1,0},
	{0,0,0,0,0,0,0,0},
};

// -------------------
uint8_t Level_1[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,1,0,0},
	{0,0,0,0,1,1,0,0},
	{0,0,0,0,0,1,0,0},
	{0,0,0,0,0,1,0,0},
	{0,0,0,0,0,1,0,0},
	{0,0,0,0,0,1,0,0},
	{0,0,0,0,1,1,1,0},
};

// -------------------
uint8_t Level_2[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,0,0,1,1,1,0,0},
	{0,0,1,0,0,0,1,0},
	{0,0,0,0,0,0,1,0},
	{0,0,0,0,0,0,1,0},
	{0,0,1,1,1,1,0,0},
	{0,0,1,0,0,0,0,0},
	{0,0,1,1,1,1,1,0},
};

// -------------------
uint8_t Level_3[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,0,0,1,1,1,0,0},
	{0,0,1,0,0,0,1,0},
	{0,0,0,0,0,0,1,0},
	{0,0,0,1,1,1,0,0},
	{0,0,0,0,0,0,1,0},
	{0,0,1,0,0,0,1,0},
	{0,0,0,1,1,1,0,0},
};

// -------------------
uint8_t Level_4[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,0,0,0,0,0,1,0},
	{0,0,0,0,0,1,1,0},
	{0,0,0,0,1,0,1,0},
	{0,0,0,1,0,0,1,0},
	{0,0,0,1,1,1,1,1},
	{0,0,0,0,0,0,1,0},
	{0,0,0,0,0,0,1,0},
};

// -------------------
uint8_t Level_5[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,0,1,1,1,1,1,0},
	{0,0,1,0,0,0,0,0},
	{0,0,1,1,1,1,0,0},
	{0,0,0,0,0,0,1,0},
	{0,0,0,0,0,0,1,0},
	{0,0,1,0,0,0,1,0},
	{0,0,0,1,1,1,0,0},
};

// -------------------
uint8_t Level_6[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,0,0,1,1,1,0,0},
	{0,0,1,0,0,0,1,0},
	{0,0,1,0,0,0,0,0},
	{0,0,1,1,1,1,0,0},
	{0,0,1,0,0,0,1,0},
	{0,0,1,0,0,0,1,0},
	{0,0,0,1,1,1,0,0},
};

// -------------------
uint8_t Level_7[8][8]=
{									
	{0,0,0,0,0,0,0,0},
	{0,0,1,1,1,1,1,0},
	{0,0,0,0,0,0,1,0},
	{0,0,0,0,0,0,1,0},
	{0,0,0,0,0,1,0,0},
	{0,0,0,0,1,0,0,0},
	{0,0,0,0,1,0,0,0},
	{0,0,0,0,1,0,0,0},
};

// -----------------------
uint8_t ScoreEmptyMap[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,0,0,0,0,0,0,1},
	{1,1,1,1,1,1,1,1}
};

// -------------------
uint8_t Score_1[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,0,0,0,1,0,0,1},
	{1,0,0,1,1,0,0,1},
	{1,0,0,0,1,0,0,1},
	{1,0,0,0,1,0,0,1},
	{1,0,0,0,1,0,0,1},
	{1,0,0,1,1,1,0,1},
	{1,1,1,1,1,1,1,1}
};

// -------------------
uint8_t Score_0[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,0,0,1,1,0,0,1},
	{1,0,1,0,0,1,0,1},
	{1,0,1,0,0,1,0,1},
	{1,0,1,0,0,1,0,1},
	{1,0,1,0,0,1,0,1},
	{1,0,0,1,1,0,0,1},
	{1,1,1,1,1,1,1,1},
};

// --------------------
uint8_t Score_25[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
	{1,0,0,1,1,0,0,1},
	{1,1,1,1,1,1,1,1},
	{1,1,0,0,0,0,1,1},
	{1,1,0,0,0,0,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
};

// -------------------
uint8_t Score_7[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,0,1,1,1,1,0,1},
	{1,0,0,0,0,1,0,1},
	{1,0,0,0,1,0,0,1},
	{1,0,0,0,1,0,0,1},
	{1,0,0,0,1,0,0,1},
	{1,0,0,0,1,0,0,1},
	{1,1,1,1,1,1,1,1},
};

// --------------------
uint8_t Score_12[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,0,1,0,1,1,1,1},
	{1,1,1,0,0,0,1,1},
	{1,0,1,0,1,1,1,1},
	{1,0,1,0,1,0,0,1},
	{1,0,1,0,1,0,0,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
};

// --------------------
uint8_t Score_18[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,0,1,0,1,1,1,1},
	{1,1,1,0,1,0,1,1},
	{1,0,1,0,1,1,1,1},
	{1,0,1,0,1,0,1,1},
	{1,0,1,0,1,0,1,1},
	{1,1,1,1,1,1,1,1},
	{1,1,1,1,1,1,1,1},
};

// -------------------
uint8_t Score_3[8][8]=
{         
	{1,1,1,1,1,1,1,1},
	{1,0,1,1,1,1,0,1},
	{1,0,0,0,0,0,1,1},
	{1,0,0,1,1,1,0,1},
	{1,0,0,0,0,0,1,1},
	{1,0,0,0,0,0,1,1},
	{1,0,1,1,1,1,0,1},
	{1,1,1,1,1,1,1,1},
};

// 显示分数
// ----------------------------
void ShowScore(uint8_t a[8][8])
{
	int li,lj;
	Clear();
	for (li = 0; li < 8; li++)
	{
		for (lj = 0; lj < 8; lj++)
		{
			SnakeMap[li][lj] = a[li][lj];
		}
	}
	Delay(10000);
}

// 显示关卡
// ----------------------------------------
void ShowLevel(uint8_t SnakeLevelMap[8][8])
{
	int i, j;
	Clear();
	for (i = 0; i < 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			SnakeMap[i][j] = SnakeLevelMap[i][j];
		}
	}
	Delay(100000);
	Clear();
}	

// ------------
void SnakeDie() 
{
	int li, lj;
	Clear();
	for (li = 0; li < 8; li++)
	{
		for (lj = 0; lj < 8; lj++)
		{
			SnakeMap[li][lj] = SnakeDieMap[li][lj];
		}
	}
	Die = 1;
	dieCount++; // 死亡次数加 1 
	count = 0;
	Delay(20000);
}

// -----------------
int IsMove(int n)
{
	if(n <= -1 || n >= 8) // 蛇越界，让蛇死亡
	{
		SnakeDie();	
	}
	return n;
}

// --------
void Show()
{
	int l, k;
	k = snakeFront;
	for (l = snakeRear; l != k - 1; l--)
	{
		if (k == 0)
		{
			k = 9;
		}
		if ( l<= -1)
		{
			l = 9;
		}
		SnakeMap[snake.x[l]][snake.y[l]] = 1;
	}
}

// 蛇的移动
// --------------
void SnakeMove()
{
	int x, y;
	// 获取蛇头坐标
	x = snake.x[snakeFront];
	y = nake.y[snakeFront];

	if(dirs =='w') // 方向向上
	{
		x--; // 蛇头行减一
		x = IsMove(x);
	}
	else if(dirs =='s') 
	{
		x++;
		x = IsMove(x);
	}
	else if(dirs =='a') 
	{
		y--;
		y = IsMove(y);
	}
	else if(dirs =='d') 
	{
		y++;
		y = IsMove(y);
	}
	if(x != snake.x[snakeFront] || y != snake.y[snakeFront]) // 发生变化了
	{
		snakeFront--;
		if(snakeFront <= -1)
		{
			snakeFront = 9;
		}
		// 新的蛇头位置
		snake.x[snakeFront] = x;
		snake.y[snakeFront] = y;

		if(SnakeMap[x][y] == 1) // 如果当前点是亮的
		{
			
			if(snakeLength >= MAX_length - 1 && snake.x[snakeFront] == food.x && snake.y[snakeFront] == food.y)
			{
				SnakeMap[x][y] = 1;
				SnakeMap[snake.x[snakeRear]][snake.y[snakeRear]] = 0;
				snakeRear--;
				score++;
				CreateFood(); 
			}
			// 是食物，说明吃到食物了
			else if(snake.x[snakeFront] == food.x && snake.y [snakeFront] == food.y)
			{
				snakeLength++;
				score++;
				CreateFood(); // 生成新的食物
			}
			else
			{
				SnakeDie(); // 撞到了自己的身体，死亡
			}		
		}
		else
		{
			// 正常移动
			SnakeMap[x][y] = 1;
			SnakeMap[snake.x[snakeRear]][snake.y[snakeRear]] = 0;
			snakeRear--;
		}
		if (snakeRear <= -1)
		{
			snakeRear = 9;
		}
	}
}

// 清空蛇的地图
// ---------
void Clear()
{
	int o, p;
	for(o = 0; o < 8; o++)
	{
		for(p = 0; p < 8; p++)
		{
			SnakeMap[o][p] = 0;
		}		
	}		
}

// ------------
int main (void)
{
	int speed = 10000; // 速度
	int level = 1;
	int lighting = 0;

	SystemInit();
	GPIOInit();
	Time0Init ();
	EintAllInit();
	Clear();

	if(level == 1)
	{
		ShowLevel(Level_1); // 显示第一关
	}

	while(1)
	{
		Delay(1000);
		CreateSnake();
		CreateFood();
		count = 0;
		Delay(10000);
		while(Die == 0)
		{
			SnakeMove();
			Delay(speed);
			if(dieCount >= 3)
				{
					if(level == 1)
				{
					lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
				ShowScore(Score_0);
						Delay(10000);
						ShowScore(ScoreEmptyMap);
					}
				}
				else if(level == 2)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
						ShowScore(Score_1);
						Delay(10000);
						ShowScore(ScoreEmptyMap);
					}
				}
				else if(level == 3)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_3);
						Delay(10000);			
						ShowScore(ScoreEmptyMap);
						
					}
				}
				else if(level == 4)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_7);
						Delay(10000);			
						ShowScore(ScoreEmptyMap);
						
					}
				}
				else if(level == 5)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_12);
						Delay(10000);			
						ShowScore(ScoreEmptyMap);
						
					}
				}
				else if(level == 6)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_18);
						Delay(10000);			
						ShowScore(ScoreEmptyMap);
						
					}
				}
				else if(level == 7)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_25);
						Delay(10000);			
						ShowScore(ScoreEmptyMap);
						
					}
				}
					speed = 10000;
					count = 0;
					dieCount = 0;
					level = 1;
				Clear();
				if(level == 1)
				{
					dieCount = 0;
				ShowLevel(Level_1);
				}
				}
			if(count >= 4)
			{	
				level++;
				Clear();
				
				if(level == 1)
				{
					dieCount = 0;
				ShowLevel(Level_1);
				}
				else if(level == 2)
				{
				  dieCount = 0;
				ShowLevel(Level_2);
				}
				else if(level == 3)
				{
					dieCount = 0;
				ShowLevel(Level_3);
				}
				else if(level == 4)
				{
					dieCount = 0;
				ShowLevel(Level_4);
				}
				else if(level == 5)
				{
					dieCount = 0;
				ShowLevel(Level_5);
				}
				else if(level == 6)
				{
					dieCount = 0;
				ShowLevel(Level_6);
				}
				else if(level == 7)
				{
					dieCount = 0;
				ShowLevel(Level_7);
				}
				CreateSnake();	
		    CreateFood();				
				count = 0;
				speed = speed - 1500;
				
				Delay(10000);
			}

		}		
		Clear();
	}
}

// -------------------------
void EINT0_IRQHandler (void)
{ 
	char ch = 'a';
	LPC_SC->EXTINT |=(1<<0) ;
	if(0 == (LPC_GPIO2->FIOPIN & (1<<10)))
	{	
		if(dirs =='d') 
		{
			return;
		}		
		else
		{
			dirs = ch;
		}	
	}		
}

// -------------------------
void EINT1_IRQHandler (void)
{	
	char ch = 'w';
	LPC_SC->EXTINT |=(1<<1) ;
	if(0 ==(LPC_GPIO2->FIOPIN & (1<<11)))
	{	
		if(dirs=='s') 
		{
			return;
		}
		else
		{
			dirs = ch;
		}	
	}		
}

// -------------------------
void EINT2_IRQHandler (void)
{	
	char ch = 'd';
	LPC_SC->EXTINT |=(1<<2) ;
	if(0 == (LPC_GPIO2->FIOPIN & (1<<12)))
	{	
		if(dirs =='a')
		{
			return;
		}
		else
		{
			dirs = ch;
		}
	}		
}

// -------------------------
void EINT3_IRQHandler (void)
{	
	char ch = 's';
	LPC_SC->EXTINT |=(1<<3) ;
	if(0 == (LPC_GPIO2->FIOPIN & (1<<13)))
	{	
		if(dirs =='w')
		{
			return;
		}
		else
		{
			dirs = ch;
		}
	}		
}

uint8_t i = 0;
uint8_t const BUFFER[8]={0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80};

// ---------------------
void TIMER0_IRQHandler()
{
	int j;
	LPC_TIM0->IR |=(1<<0);
	LPC_GPIO2->FIOCLR  |= BEEP; // P0.4 - P0.11
	LPC_GPIO0->FIOSET  |= BPEE; // R1~R7 P0.0 - P0.7

	for(j = 0; j < 8; j++)  
	{
		LPC_GPIO2->FIOSET |= (SnakeMap[j][i]<<j);
	}
	LPC_GPIO0->FIOCLR |= (BUFFER[i]<<4);
	i++;
	if(i == 8)
	{
		i = 0;
	}
}
