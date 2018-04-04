/*
*********************************************************************************************************
*                                              EXAMPLE CODE
*
*                          (c) Copyright 2003-2007; Micrium, Inc.; Weston, FL
*
*               All rights reserved.  Protected by international copyright laws.
*               Knowledge of the source code may NOT be used to develop a similar product.
*               Please help us continue to provide the Embedded community with the finest
*               software available.  Your honesty is greatly appreciated.
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*
*                                            EXAMPLE CODE
*
*                                     ST Microelectronics STM32
*                                              on the
*
*                                     Micrium uC-Eval-STM32F107
*                                        Evaluation Board
*
* Filename      : app.c
* Version       : V1.00
* Programmer(s) : JJL
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                             INCLUDE FILES
*********************************************************************************************************
*/

#include <includes.h>


/*
*********************************************************************************************************
*                                            LOCAL DEFINES
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                           LOCAL VARIABLES
*********************************************************************************************************
*/

static  OS_TCB          AppTaskStartTCB; 
static  OS_TCB          AppTask1TCB; 
static  OS_TCB          AppTask2TCB; 
static  OS_TCB          AppTask3TCB; 


/*
*********************************************************************************************************
*                                                STACKS
*********************************************************************************************************
*/

static  CPU_STK         AppTaskStartStk[APP_TASK_START_STK_SIZE];

static  CPU_STK         AppTask1Stk[APP_TASK_STK_SIZE];
static  CPU_STK         AppTask2Stk[APP_TASK_STK_SIZE];
static  CPU_STK         AppTask3Stk[APP_TASK_STK_SIZE];


/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

static  void  AppTaskCreate     (void);
static  void  AppTaskStart      (void *p_arg);

static  void  AppTask1 (void *p_arg);
static  void  AppTask2 (void *p_arg);
static  void  AppTask3 (void *p_arg);

static  void  USART_CNF();



/*
*********************************************************************************************************
*                                                main()
*
* Description : This is the standard entry point for C code.  It is assumed that your code will call
*               main() once you have performed all necessary initialization.
*
* Arguments   : none
*
* Returns     : none
*********************************************************************************************************
*/

static int threshold = 0;
static int count1 = 0;
static int count2 = 0;
static int count3 = 0;

int  main (void)
{
    OS_ERR  err;
    threshold = 500000;

    BSP_IntDisAll();                                            /* Disable all interrupts.                              */

    OSInit(&err);                                               /* Init uC/OS-III.                                      */

    OSSchedRoundRobinCfg((CPU_BOOLEAN)DEF_TRUE, 
                         (OS_TICK    )10,
                         (OS_ERR    *)&err);
    
    OSTaskCreate((OS_TCB     *)&AppTaskStartTCB,                /* Create the start task                                */
                 (CPU_CHAR   *)"App Task Start",
                 (OS_TASK_PTR )AppTaskStart, 
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_START_PRIO,
                 (CPU_STK    *)&AppTaskStartStk[0],
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_START_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )0,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);

    OSStart(&err);                                              /* Start multitasking (i.e. give control to uC/OS-III). */
}


/*
*********************************************************************************************************
*                                          STARTUP TASK
*
* Description : This is an example of a startup task.  As mentioned in the book's text, you MUST
*               initialize the ticker only once multitasking has started.
*
* Arguments   : p_arg   is the argument passed to 'AppTaskStart()' by 'OSTaskCreate()'.
*
* Returns     : none
*
* Notes       : 1) The first line of code is used to prevent a compiler warning because 'p_arg' is not
*                  used.  The compiler should not generate any code for this statement.
*********************************************************************************************************
*/

static  void  AppTaskStart (void *p_arg)
{
    CPU_INT32U  cpu_clk_freq;
    CPU_INT32U  cnts;
    OS_ERR      err;
    
    
   (void)p_arg;

    BSP_Init();                                                 /* Initialize BSP functions                                 */

    CPU_Init();

    cpu_clk_freq = BSP_CPU_ClkFreq();
    cnts         = cpu_clk_freq / (CPU_INT32U)OSCfg_TickRate_Hz;/* Determine nbr SysTick increments                         */
    OS_CPU_SysTickInit(cnts);                                   /* Init uC/OS periodic time src (SysTick).                  */

#if OS_CFG_STAT_TASK_EN > 0u
    OSStatTaskCPUUsageInit(&err);                               /* Compute CPU capacity with no task running                */
#endif

    CPU_IntDisMeasMaxCurReset();
    USART_CNF();
    
    AppTaskCreate();                                            /* Create application tasks                                 */

    while (DEF_TRUE) {                                          /* Task body, always written as an infinite loop.           */
        OSTimeDlyHMSM(0, 0, 0, 250, 
                      OS_OPT_TIME_HMSM_STRICT, 
                      &err);
    }
}


static  void  USART_CNF(){
  GPIO_InitTypeDef GPIO_InitStructure;
  USART_InitTypeDef USART_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO | RCC_APB2Periph_USART1
                         , ENABLE);
  
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  USART_InitStructure.USART_BaudRate                    = 9600;
  USART_InitStructure.USART_WordLength                  = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits                    = USART_StopBits_1;
  USART_InitStructure.USART_Parity                      = USART_Parity_No;
  USART_InitStructure.USART_Mode                        = USART_Mode_Tx;
  USART_InitStructure.USART_HardwareFlowControl         = USART_HardwareFlowControl_None;
  
  USART_Init(USART1, &USART_InitStructure);
  USART_Cmd(USART1, ENABLE);
}


/*
*********************************************************************************************************
*                                      CREATE APPLICATION TASKS
*
* Description:  This function creates the application tasks.
*
* Arguments  :  none
*
* Returns    :  none
*********************************************************************************************************
*/

static  void  AppTaskCreate (void)
{
    OS_ERR  err;   
    OSTaskCreate((OS_TCB     *)&AppTask1TCB, 
                 (CPU_CHAR   *)"App 1 Start",
                 (OS_TASK_PTR )AppTask1, 
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_TEMP_SENSOR_PRIO,
                 (CPU_STK    *)&AppTask1Stk[0],
                 (CPU_STK_SIZE)APP_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )1,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    
    OSTaskCreate((OS_TCB     *)&AppTask2TCB, 
                 (CPU_CHAR   *)"App 2 Start",
                 (OS_TASK_PTR )AppTask2, 
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_TEMP_SENSOR_PRIO,
                 (CPU_STK    *)&AppTask2Stk[0],
                 (CPU_STK_SIZE)APP_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )2,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
    
    OSTaskCreate((OS_TCB     *)&AppTask3TCB, 
                 (CPU_CHAR   *)"App 3 Start",
                 (OS_TASK_PTR )AppTask3, 
                 (void       *)0,
                 (OS_PRIO     )APP_TASK_TEMP_SENSOR_PRIO,
                 (CPU_STK    *)&AppTask3Stk[0],
                 (CPU_STK_SIZE)APP_TASK_STK_SIZE / 10,
                 (CPU_STK_SIZE)APP_TASK_STK_SIZE,
                 (OS_MSG_QTY  )0,
                 (OS_TICK     )3,
                 (void       *)0,
                 (OS_OPT      )(OS_OPT_TASK_STK_CHK | OS_OPT_TASK_STK_CLR),
                 (OS_ERR     *)&err);
}

static  void  AppTask1 (void *p_arg)
{
    OS_ERR      err;
    while (DEF_TRUE) {                                                    /*  Sense the temperature every 100 ms     */
      count1 += 1;
      if(count1 >= threshold){
        BSP_LED_Toggle(1);
        USART_SendData(USART1, (u16)'*');
        count1 = 0;
      }
    }
}
static  void  AppTask2 (void *p_arg)
{
    OS_ERR      err;
    while (DEF_TRUE) {                                                    /*  Sense the temperature every 100 ms     */
      count2 += 1;
      if(count2 >= threshold){
        BSP_LED_Toggle(2);
        USART_SendData(USART1, (u16)'@');
        count2 = 0;
      }
    }
}
static  void  AppTask3 (void *p_arg)
{
    OS_ERR      err;
    while (DEF_TRUE) {                                                    /*  Sense the temperature every 100 ms     */
      count3 += 1;
      if(count3 >= threshold){
        BSP_LED_Toggle(3);
        USART_SendData(USART1, (u16)'#');
        count3 = 0;
      }
    }
}