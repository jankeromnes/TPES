/**
 * Simple Driver Implementation.
 */
#define ID_T UINT16
#define DATA_T UINT16

#define MAX_DEV 2^sizeof(ID_T)
/**
 * Message From Sensors to Drivers
 */
typedef struct {
	ID_T id;
	DATA_T data;
} MesSenToDrv;

/**
 * Message From Driver to Application.
 */
typedef struct {
	double time;
	DATA_T data;
} MesDrvToApp;

MesSenToDrv RegMessage;
/**
 * Install a driver
 * Return: The number of the driver, even if it is already installed.
 */
int install(int priority);

/**
 * Uninstall a driver.
 * Return: Ok if Good, ERROR otherwise.
 */
int unsinstall();

/**
 * Add a device that use our driver.
 * Return: OK if good, ERROR otherwise.
 */
int add_dev(char *name, ID_T id);

/**
 * Remove a device that use our driver.
 * Return: OK if good, ERROR otherwise.
 */
int del_dev(ID_T id);
