#include<stdio.h>
#include<ioLib.h>
#include<iosLib.h>
#include<stdlib.h>
#include<semLib.h>
#include<string.h>
#include<taskLib.h>

#include<sys/types.h>

#include <time.h>

#include "driver.h"

/**
 * Stack size of the task which check the availabity of datas from sensors.
 */
#define STACK_SIZE_CHECK 1000

/**
 * Driver Number, Error if not installed.
 */
static int num_drv = ERROR;

/**
 * Semaphore to simulate an interrupt, 
 * It's full if a sensor emmited data, emptied if read.
 */
static SEM_ID sem_id;

/**
 * Id of the task to check the availability of the data.
 */
static int check_task_id;

/**
 * Device(sensor) Structure, with the last data given.
 */
typedef struct{
	DEV_HDR devHdr;
	MesDrvToApp lastMesDrvToApp;
} myDev;

/**
 * List of the devices associated to the driver.
 * It keep a pointer to the device in the device table,
 * and is indexed with the device id.
 */
static myDev *listDev[MAX_DEV];

/**
 * Return the actual time in milliseconds (To Verify).
 */
unsigned long getTime() {
	struct timespec timev;
	unsigned long nano;
	time_t sec;
	
	clock_gettime(CLOCK_REALTIME, &timev);
	nano = timev.tv_nsec;
	sec = timev.tv_sec;
	
	return (sec*1000 + nano/1000000) % MAX_LONG;
}

/**
 * Function, used as a motor for a task.
 * It wait for the semaphore to be full,
 * then get the message, the associated device,
 * and create a message for the application.
 */
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
 * TODO Driver Basic Functions to Implement.
 */
void myOpen() {}
void myClose() {}
void myRead() {}
void myIoCtl() {}

/**
 * If it not installed, we do it and we return the driver number,
 * even if it's already installed.
 * 
 * We initialise the device list, and create a task which "check the sensors".
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
	
	/* TODO
	 *   Stop the task
	 *   Remove the Semaphore
	 * 	 Maybe we have to free the devices attached...
	 */
	
	return OK;	
}

/**
 * If the driver is installed, we allocate a driver struct,
 * and we add it to the Device table.
 * 
 * We add the device to the list.
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
 * Delete a device by his ID, and check if it belongs to our driver.
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

/**
 * Function used by the concentrator to 
 * say that there is data available.
 */
void message_available() {
	semGive(sem_id);
}

/**
 * Get the id of a device associated by his name.
 * Return MAX_DEV if not found.
 */
ID_T get_dev_by_name(char *name) {
	int i;
	for(i=0; i<MAX_DEV;i++) {
		if(strcmp(listDev[i], name) == 0) {
			return i;
		}
	}
	return MAX_DEV;
}




