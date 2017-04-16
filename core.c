#include "std.h"
#include "serial.h"
uint READY;  /*READY为任务就绪总表，一共可支持8个任务*/
uint CUR,OS_START=0;	/*当前最高优先级,系统是否运行*/
__xdata struct TCB tcb[8];
__idata uchar os_stack[23*8]; 
__idata uchar stack_int[20];
__xdata uchar interrup;
TCB_T tcb_t[8];	/*使用最高优先级查询任务控制块指针*/
TCB_T __idata CUR_TCB_T;TCB_T __idata HIGHT_TCB_T;	/*当前TCB指针和最高优先级任务的TCB指针*/
__code uint READYMAP[]={
	0, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	7, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	6, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	5, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0,
	4, 0, 1, 0, 2, 0, 1, 0, 3, 0, 1, 0, 2, 0, 1, 0
};		/*最高优先级查询表，0级任务最优先*/
__code uint MASK[8]={1,2,4,8,16,32,64,128};
/**********************************************************************
 *查找最高优先级任务,并调用switch_task进行任务切换
 *同时将CUR设为最高优先级
 **********************************************************************/
uint schedule(void){		//调度函数
	uchar i;
	__xdata uchar hight;
	if (OS_START==1){
		EA=0;
		hight=READYMAP[READY];
		if (hight==CUR){
			EA=1;
			return 1;	//计算最高优先级
		}
		HIGHT_TCB_T=tcb_t[hight];
		CUR=hight;
/*
			display("READY=",READY);
			display("hight=",hight);
			display("HIGHT_TCB_T=",(uchar)(HIGHT_TCB_T));
			display("HIGHT_TCB_T,sp=",(uchar)(HIGHT_TCB_T->sp));
			display("red,sp=",(uchar)(tcb_t[1]->sp));
			display("sp=",(uchar)SP);
			sendstr("\n");
			*/
		if (interrup==0){
			switch_task();
		}else{
		}
		EA=1;
	}
	return 0;
}
	/*
	   uint schedule(void){		
	   uchar hight;
	   hight=READYMAP[READY];	
	HIGHT_TCB_T=tcb_t[hight];
	if (hight==CUR){
		EA=1;
		return 1;
	}else{
		CUR=hight;
	//	CUR_TCB_T=tcb_t[CUR];
	}
	if (OS_START==1){
//		display("READY=",READY);
//		display("CUR=",CUR);

		if(interrup==0){
			switch_task();
		}else{
		//	switch_task_int();
			return;
		}
	}
		EA=1;
	return 0;
}
*/
/*****************************************************************
 *任务栈初始化函数
 *************************************************************/
uchar* stack_init(void *thread,uint p){	/*p：优先级*/
	__idata uchar *stack;
	stack=&os_stack[p*23];
	*stack++=(uchar)thread;	
	*stack++=(uchar)((uint)thread>>8);
	return (stack+13);
}
/**********************************************************************
 *填充任务控制信息
 ***********************************************************************/
void tcb_init(uchar *stack,uint p){
	tcb_t[p]->sp=(__idata uchar*)stack;
	tcb_t[p]->priority=p;
	return;
}
/**********************************************************************
 *该函数会调用stack_init初始化调用栈,调用tcb_init登记任务控制信息
 *@ void *thread: 指向进程入口的指针
 *@uint p:	进程优先级
 *
 * *******************************************************************/
void task_create(void *thread,uint p){
	uchar *stack=stack_init(thread,p);
	tcb_init(stack,p);
	READY=READY|MASK[p];
	schedule();
	return;
}
/**********************************************************************
 *该函数删除任务,清除数据
 **********************************************************************/
void task_del(uint p)
{
	EA=0;
	READY=READY&(~MASK[p]);
	EA=1;
	return;
}
/**********************************************************************
 *系统数据初始化函数,系统启动时调用
 * *******************************************************************/
void os_init(void){
	uint i;
	READY=0;
	interrup=0;
	for(i=0;i<8;i++)tcb_t[i]=&(tcb[i]);
#ifdef debug_osinit
	for(i=0;i<8;i++){
		display("in_osinit,tcb_t_H",*((uchar*)&tcb_t[i]+1));
		display("in_osinit,tcb_t_L",*((uchar*)&tcb_t[i]));
	}
#endif

	return;
}
void os_start(void)
{
	__xdata uchar hight=READYMAP[READY];
	OS_START=1;
	HIGHT_TCB_T=tcb_t[hight];
	CUR=hight;
	CUR_TCB_T=HIGHT_TCB_T;
	switch_task_int();
	return;
}

