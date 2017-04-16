#ifndef __CORE__H
#define __CORE__H
#include "std.h"
void os_init(void);
void os_start(void);
void task_create(void *thread,uint p);
void task_del(uint p);
uint schedule(void);
extern uint OS_START;
extern uint READY;
extern __code uint MASK[8];
extern uint CUR;
extern __code uint  READYMAP[];
extern TCB_T __idata  HIGHT_TCB_T;
extern __xdata interrup;
extern TCB_T tcb_t[8];	/*使用最高优先级查询任务控制块指针*/

#define enter_int	\
	{ \
	__asm   \
	CLR EA	\
	MOV DPL,# _interrup   \
	MOV DPH,# ( _interrup>>8) \
	MOVX A, @DPTR  \
	INC A   \
	MOVX @DPTR, A \
	MOV R0,# _CUR_TCB_T	\
	MOV DPL ,@R0   \
	INC R0   \
	MOV DPH , @R0   \
	MOV A , SP   \
	MOVX @DPTR , A   \
	MOV R0 ,# _stack_int	  \
	MOV A , R0   \
	MOV SP , ACC   \
	__endasm;   \
	}   
#endif
