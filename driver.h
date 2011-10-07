/**
 * Simple Driver Implementation.
 */
 
/**
 * Redefinition of types, for convenience.
 */
#define ID_T UINT16
#define DATA_T UINT16

/**
 * We didn't find those constant in vxworks headers...
 */
#define MAX_DEV 2^sizeof(ID_T)
#define MAX_LONG 2^sizeof(long)

/**
 * TODO Global variable to put in the concentrator.
 */
MesSenToDrv RegMessage;

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
	unsigned long time;
	DATA_T data;
} MesDrvToApp;

/**
 * Install a driver
 * Return: The number of the driver, even if it is already installed.
 */
int install(int priority);

/**
 * Uninstall a driver.
 * Return: Ok if Good, ERROR otherwise.
 */
int uninstall();

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

/**
 * To use with by the concentrator to say that there is a message available.
 */
void message_available();

/**
 * Get the id of a device associated by his name.
 * Return MAX_DEV if not found.
 */
ID_T get_dev_by_name(char *name);
