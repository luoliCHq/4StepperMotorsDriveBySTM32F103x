/*
 * luoliCHq
 */

 
 
#include "timer1.h"
#include "GPIO_Motor.h"
#include "motor.h"
#include "motor_ctrl.h"



void TIM1Nvic_Config_Init(void)
{
    NVIC_InitTypeDef NVIC_InitStructure;
    
    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;     // TIM1更新中断通道,这玩意儿在stm32f10x.h找
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;   // 抢占优先级
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;      // 子优先级
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;     // IQR通道使能
    NVIC_Init(&NVIC_InitStructure);
}

void TIM1_Config_Init(u16 arr,u16 psc)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* 1. 定时器所需的系统时钟使能 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1,ENABLE);		// 注意TIM1是挂载在APB2预分频器所在总线上
    /* 2. 定时器TIM1初始化 */
	
    TIM_TimeBaseStructure.TIM_Period = arr-1;     // 自动重装周期值
    TIM_TimeBaseStructure.TIM_Prescaler = psc-1;  // 预分频系数
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;     // CKD[1:0]： 时钟分频因子 (Clock division)，与CK_INT有关
    TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;     // 向上计数模式
	//高级定时器TIM1与普通定时器TIM3等不同的是，这里要初始化，重复计数设置
	TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;		// 重复计数设置
	TIM_TimeBaseInit(TIM1,&TIM_TimeBaseStructure);

	/* 使能前，清除更新中断标志位 */
    TIM_ClearFlag(TIM1,TIM_FLAG_Update);		//使能前清除中断标志位，以免一启用中断后立即产生中断
	
	/* 4. 允许更新中断使能 */
    TIM_ITConfig(TIM1,TIM_IT_Update,ENABLE);

    /* 5. 中断优先级设置 */
    TIM1Nvic_Config_Init();

    /* 6. 打开/使能TIM3 */
    TIM_Cmd(TIM1,ENABLE);
}

// 中断服务子程序名，参考启动文件startup_stm32f10x_md.s
void TIM1_UP_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM1,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM1,TIM_IT_Update);	
		if(count_time == motor1.totalTime)	// 当达到指定时间motor1.totaltime（ms）后，电机到达目标点		
		{
			Stop_Motor_withS(MOTOX);
			if(motor1.nextMove == 0)		// 如果是正向过程，到达目的地停止
			{
				motor1.isDestn = 1;
			}
			if(motor1.nextMove == 1)		// 如果是反向过程，到达复位点停止
			{
				motor1.isResetPoint = 1;
				motor1.isDestn = 0;			// 这俩值相反的
				motor1.nextMove = 0;
			}
		}
		
		if(count_time == motor2.totalTime)
		{
			Stop_Motor_withS(MOTOY);
			if(motor2.nextMove == 0)
			{
				motor2.isDestn = 1;
			}
			if(motor2.nextMove == 1)
			{
				motor2.isResetPoint = 1;
				motor2.isDestn = 0;
				motor2.nextMove = 0;
			}
			
		}

		count_time ++;
	}
}
