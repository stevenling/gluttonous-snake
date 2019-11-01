#include "LPC17xx.h"                                                  
#include "stdlib.h"
#include "time.h"

#define CCLK 100000000                                                              
#define PCLK0 CCLK/4
#define BEEP            ( 0xFF<< 0)  
#define BPEE            ( 0xFF<< 4)
#define MAX_length 10

// 函数
// -------------------------------------
void show();
void Clear();
void myDelay (uint32_t ulTime);
void GPIO_Init(void);
void Eint0_Eint1_Eint2_Eint3_Init(void);
void TIMER0_Init (void);
void CreateSnake();//snake init
void CreateFood();
void ShowScore(uint8_t a[8][8]);
void ShowLevel(uint8_t a[8][8]);
void die();//s=die 
int oper_move(int n);
void show();//
void SnakeMove();
void Clear();

// 全局变量
int length=3;
char dirs='n';
int front=0,rear=2;
int Die=0;
int score=0;
int count = 0;
int dieCount = 0;

// ---------------------------
void myDelay (uint32_t ulTime)
{
	uint32_t i;
	i = 0;
	while (ulTime--) 
	{
		for (i = 0; i < 1000; i++);
	}
}

// -----------------
void GPIO_Init(void)
{
	LPC_GPIO0->FIODIR  = LPC_GPIO0->FIODIR | BPEE; //C1~C7 P0.4 - P0.11
	LPC_GPIO2->FIODIR  = LPC_GPIO2->FIODIR| BEEP; // R1~R7 P0.0 - P0.7
}

// ------------------------------------
void Eint0_Eint1_Eint2_Eint3_Init(void)
{
	LPC_PINCON->PINSEL4 =(LPC_PINCON->PINSEL4 & ~(0xFF << 20))|(0x55<<20);
	LPC_SC->EXTMODE     |=(1<<0);//                                
	LPC_SC->EXTPOLAR    &=~(1<<0); //                                
	LPC_SC->EXTINT |=(1<<0);//
	NVIC_EnableIRQ(EINT0_IRQn);//

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
void TIMER0_Init (void)
{
	LPC_TIM0->CTCR = 0;
	LPC_TIM0->PR = 0;
	LPC_TIM0->TC = 0;
	LPC_TIM0->MR0 = 0.001*PCLK0-1;
	LPC_TIM0->MCR = 0x03;
	LPC_TIM0->TCR = 1;
	NVIC_EnableIRQ(TIMER0_IRQn);
}

// --------------------
uint8_t Snake_map[8][8]=
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

// snake init
// ------------
void CreateSnake()
{
	int i;
	Die = 0;
	dirs = 'n';
	length = 3;
	front = 0;
	rear = 2;
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
	show();
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
	Snake_map[0][0] = 0;
	// 如果是 1 表明是亮的，必须继续找
	do
	{
		food.x = rand() % 8;
		food.y = rand() % 8;
	}while(Snake_map[food.x][food.y] == 1); 

	Snake_map[food.x][food.y] = 1;
	count++;
}

// ---------------------
uint8_t Snake_die[8][8]=
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
uint8_t Score_empty[8][8]=
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
			Snake_map[li][lj] = a[li][lj];
		}
	}
	myDelay(10000);
}

// 显示关卡
// ----------------------------
void ShowLevel(uint8_t a[8][8])
{
	int li,lj;
	Clear();
	for (li = 0; li < 8; li++)
	{
		for (lj = 0; lj < 8; lj++)
		{
			Snake_map[li][lj] = a[li][lj];
		}
	}
	myDelay(100000);
	Clear();
}	

// -------
void die() 
{
	int li, lj;
	Clear();
	for (li = 0; li < 8; li++)
	{
		for (lj = 0; lj < 8; lj++)
		{
			Snake_map[li][lj] = Snake_die[li][lj];
		}
	}
	Die = 1;
	dieCount++; // 死亡次数加 1 
	count = 0;
	myDelay(20000);
}

// -----------------
int oper_move(int n)
{
	if(n <= -1 || n >= 8) // 蛇越界，让蛇死亡
	{
		die();	
	}
	return n;
}

// --------
void show()
{
	int l, k;
	k = front;
	for (l = rear; l != k - 1; l--)
	{
		if (k == 0)
		{
			k = 9;
		}
		if ( l<= -1)
		{
			l = 9;
		}
		Snake_map[snake.x[l]][snake.y[l]] = 1;
	}
}

// 蛇的移动
// --------------
void SnakeMove()
{
	int x, y;
	// 获取蛇头坐标
	x = snake.x[front];
	y = nake.y[front];

	if(dirs =='w') 
	{
		x--;
		x = oper_move(x);
	}
	else if(dirs =='s') 
	{
		x++;
		x = oper_move(x);
	}
	else if(dirs =='a') 
	{
		y--;
		y = oper_move(y);
	}
	else if(dirs =='d') 
	{
		y++;
		y = oper_move(y);
	}
	if(x != snake.x[front] || y != snake.y[front])
	{
		front--;
		if(front <= -1)
		{
			front = 9;
		}
		snake.x[front] = x;
		snake.y[front] = y;
		if(Snake_map[x][y] == 1)
		{
			if(length >= MAX_length - 1 && snake.x[front] == food.x && snake.y[front] == food.y)
			{
				Snake_map[x][y] = 1;
				Snake_map[snake.x[rear]][snake.y[rear]] = 0;
				rear--;
				score++;
				CreateFood();
			}
			else if(snake.x[front] == food.x && snake.y [front] == food.y)
			{
				length++;
				score++;
				CreateFood();
			}
			else
			{
				die();
			}		
		}
		else
		{
			Snake_map[x][y] = 1;
			Snake_map[snake.x[rear]][snake.y[rear]] = 0;
			rear--;
		}
		if (rear <= -1)
		{
			rear = 9;
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
			Snake_map[o][p]=0;
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
	GPIO_Init();
	TIMER0_Init ();
	Eint0_Eint1_Eint2_Eint3_Init();
	Clear();

	if(level == 1)
	{
		ShowLevel(Level_1); // 显示第一关
	}

	while(1)
	{
		myDelay(1000);
		CreateSnake();
		CreateFood();
		count = 0;
		myDelay(10000);
		while(Die==0)
		{
			SnakeMove();
			myDelay(speed);
			if(dieCount >= 3)
				{
					if(level == 1)
				{
					lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
				ShowScore(Score_0);
						myDelay(10000);
						ShowScore(Score_empty);
					}
				}
				else if(level == 2)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
						ShowScore(Score_1);
						myDelay(10000);
						ShowScore(Score_empty);
					}
				}
				else if(level == 3)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_3);
						myDelay(10000);			
						ShowScore(Score_empty);
						
					}
				}
				else if(level == 4)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_7);
						myDelay(10000);			
						ShowScore(Score_empty);
						
					}
				}
				else if(level == 5)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_12);
						myDelay(10000);			
						ShowScore(Score_empty);
						
					}
				}
				else if(level == 6)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_18);
						myDelay(10000);			
						ShowScore(Score_empty);
						
					}
				}
				else if(level == 7)
				{
						lighting = 0;
					while(lighting <= 5)
					{
						lighting++;
								ShowScore(Score_25);
						myDelay(10000);			
						ShowScore(Score_empty);
						
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
				
				myDelay(10000);
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
		LPC_GPIO2->FIOSET |= (Snake_map[j][i]<<j);
	}
	LPC_GPIO0->FIOCLR |= (BUFFER[i]<<4);
	i++;
	if(i == 8)
	{
		i = 0;
	}
}
