#include<stdio.h>
#include<ioLib.h>
#include<iosLib.h>
#include<stdlib.h>
#include<semLib.h>
#include<string.h>
#include<taskLib.h>

#include<sys/types.h>

#include <time.h>

#include "code.h"

#define STACK_SIZE_CHECK 1000

/**
 * Driver Number, Error if not installed.
 */
static int num_drv = ERROR;

/**
 * Semaphore to simulate an interrupt.
 */
static SEM_ID sem_id;

static int check_task_id;



/**
 * Minimal Driver Structure.
 */
typedef struct{
	DEV_HDR devHdr;
	MesDrvToApp lastMesDrvToApp;
} myDev;

/**
 * Keep the devices associated with the driver.
 * The access is direct with the dev id.
 */
static myDev *listDev[MAX_DEV];

double getTime() {
	struct timespec timev;
	long nano;
	time_t sec;
	
	clock_gettime(CLOCK_REALTIME, &timev);
	nano = timev.tv_nsec;
	sec = timev.tv_sec;
	
	return sec + nano/1000000000;
}

void check_message() {
	
	MesSenToDrv myMes;
	
	for(;;) {
		
		semTake(sem_id,WAIT_FOREVER);
		myMes = RegMessage;

		if(myMes.id >= 0 
				&& myMes.id < MAX_DEV
				&& listDev[myMes.id] != NULL) {
			listDev[myMes.id]->lastMesDrvToApp.data = myMes.data;
			listDev[myMes.id]->lastMesDrvToApp.time = getTime();
		}
	}
}

/**
 * Driver Basic Functions to Implement.
 */
void myOpen() {}
void myClose() {}
void myRead() {}
void myIoCtl() {}

/**
 * If it not installed, we do it and we return the driver number,
 * even if it's already installed.
 */
int install(int priority) {
	if(num_drv == ERROR) {
		sem_id = semBCreate(0, SEM_EMPTY);
		num_drv = iosDrvInstall(0,0,(FUNCPTR)myOpen,(FUNCPTR)myClose,(FUNCPTR)myRead,0,(FUNCPTR)myIoCtl);
		memset(listDev, 0, MAX_DEV*sizeof(char *));
		check_task_id = taskSpawn ( "check_task",
		    priority,0,STACK_SIZE_CHECK,(FUNCPTR)check_message,
		    0,0,0,0,0,0,0,0,0,0);
	}
	return num_drv;
}

/**
 * If the driver is installed, we uninstall it.
 */
int uninstall() {
	if(num_drv == ERROR)
		return ERROR;
	iosDrvRemove(num_drv,0);
	num_drv = ERROR;
	return OK;	
}

/**
 * If the driver is installed, we allocate a driver struct,
 * and we add it to the Device table.
 */
int add_dev(char *name, ID_T id) {
	myDev *oneDev;
	if(num_drv == ERROR || id < 0 || id >= MAX_DEV)
		return ERROR;

	oneDev = (myDev *)malloc(sizeof(myDev));
	if(iosDevAdd((DEV_HDR *)oneDev,name, num_drv) == OK) {
		listDev[id] = oneDev;
		return OK;
	}
	else {
		free(oneDev);
		return ERROR;
	}
}

/**
 * Delete a device by his name, and check if it belongs to our driver.
 * Free the driver struct.
 */
int del_dev(ID_T id) {
	DEV_HDR *head;
	if(id < 0 || id > MAX_DEV || listDev[id] == NULL)
		return ERROR;
	
	head = (DEV_HDR *)(listDev[id]);
	if(num_drv == ERROR 
			|| head == NULL 
			|| head->drvNum != num_drv)
		return ERROR;
	
	listDev[id] = NULL;
	iosDevDelete(head);
	free(head);
	return OK;
}

void message_available() {
	semGive(sem_id);
}





