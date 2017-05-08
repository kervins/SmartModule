/**@file		main.h
 * @brief		Header file which defines the functionality of the RTOS and scheduler, as well as SmartModule-specific tasks
 * @author		Jonathan Ruisi
 * @version		1.0
 * @date		December 16, 2016
 * @copyright	GNU Public License
 */

#ifndef MAIN_H
#define MAIN_H

#include "serial_comm.h"
#include "linked_list.h"
#include "utility.h"

// DEFINITIONS (SHELL)---------------------------------------------------------
#define SHELL_MAX_RESULT_VALUES					4		/**< The maximum number of parameters that can accompany a warning or error */
#define SHELL_MAX_TASK_PARAMS					4		/**< The maximum number of parameters that can be passed to a task */
#define SHELL_MAX_TASKS							16		/**< The maximum number of tasks that can run at a given time */
#define SHELL_RESET_DELAY						3000	/**< Amount of time (in milliseconds) after startup before the scheduler is started */
// Warnings
#define SHELL_WARNING_DATA_TRUNCATED			1
#define SHELL_WARNING_FIFO_BUFFER_OVERWRITE		2
// Errors
#define SHELL_ERROR_SRAM_BUSY					1
#define SHELL_ERROR_ZERO_LENGTH					2
#define SHELL_ERROR_ADDRESS_RANGE				3
#define SHELL_ERROR_LINE_QUEUE_EMPTY			4
#define SHELL_ERROR_COMMAND_NOT_RECOGNIZED		5
#define SHELL_ERROR_TASK_TIMEOUT				6
#define SHELL_ERROR_NULL_REFERENCE				7
#define SHELL_ERROR_WIFI_COMMAND				8

// DEFINITIONS (MEASUREMENT)---------------------------------------------------
#define ADC_DC_OFFSET		3103	//*< ((x steps/4096) * 3.3V = offset in volts) */
#define ADC_WINDOW_SIZE		128		//*< ADC sample window size */
#define TIMER0_START_VALUE	0xDB60	//*< 100ms (Higher values = SHORTER timer period) */

// DEFINITIONS (OTHER)---------------------------------------------------------
#define FIRMWARE_VERSION	1.00	//*< Current firmware version */

// MACROS----------------------------------------------------------------------
/**@def CURRENT_TASK
 * Shortcut for accessing information about the currently running task
 */
#define CURRENT_TASK ((Task*) _shell.task.current->data)

// TYPE DEFINITIONS------------------------------------------------------------

/**@struct Task
 * Structure which defines a task
 */
typedef struct Task
{
	B_Action action;					/**< Pointer to the task's function */
	void* params[4];					/**< Array of pointers to parameters to be passed to the function */
	unsigned int runsRemaining;			/**< Number of remaining runs */
	unsigned long int lastRun;			/**< Timestamp indicating when the task last executed */
	unsigned long int runInterval;		/**< Interval (in ticks) at which the task executes */
	unsigned long int timeout;			/**< Defines the period at which the task is considered to have timed out */

	union
	{

		struct
		{
			unsigned modeExclusive : 1;	/**< Task is given exclusive priority */
			unsigned modeInfinite : 1;	/**< Task will run indefinitely */
			unsigned modePeriodic : 1;	/**< Task runs periodically */
			unsigned busy : 1;			/**< Task is busy */
			unsigned : 4;
		} statusBits;
		unsigned char status;
	} ;
} Task;

/**@struct Shell
 * Structure containing all necessary means of controlling the RTOS
 */
typedef struct Shell
{

	struct
	{
		unsigned char lastWarning;	/**< The most recent warning to occur */
		unsigned char lastError;	/**< The most recent error to occur */
		unsigned long int values[SHELL_MAX_RESULT_VALUES];	/**< Relevant information relating to the error or warning */
	} result;

	struct
	{
		LinkedList_16Element list;	/**< Task list */
		LinkedListNode* current;	/**< Current task */
	} task;

	CommPort* server;				/**< Pointer to a <b>CommPort</b> which serves as the TCP host */
	CommPort* terminal;				/**< Pointer to a <b>CommPort</b> which serves as the debug terminal */
	Buffer swapBuffer;				/**< All data in and out of the shell passes through this buffer */
} Shell;

typedef struct AdcRmsInfo
{
	RingBuffer samples;
	unsigned char pinFloatAnimation;
} AdcRmsInfo;

typedef struct ProxDetectInfo
{
	bool isTripped;
	unsigned int count;
	unsigned long int lastTripped;
} ProxDetectInfo;

// CONSTANTS-------------------------------------------------------------------
static const char* _id	= "SM000001";
const struct Point COORD_LABEL_UPTIME		= {52, 1};
const struct Point COORD_LABEL_NAME			= {20, 1};
const struct Point COORD_LABEL_STATUS		= {37, 1};
const struct Point COORD_LABEL_SSID			= {14, 2};
const struct Point COORD_LABEL_HOST			= {14, 3};
const struct Point COORD_LABEL_RELAY		= {14, 5};
const struct Point COORD_LABEL_PROX			= {15, 6};
const struct Point COORD_LABEL_TEMP			= {32, 6};
const struct Point COORD_LABEL_LOAD			= {32, 5};
const struct Point COORD_LABEL_COMM1A		= {1, 9};
const struct Point COORD_LABEL_COMM1B		= {6, 10};
const struct Point COORD_LABEL_COMM1C		= {6, 11};
const struct Point COORD_LABEL_COMM1D		= {6, 12};
const struct Point COORD_LABEL_COMM2A		= {1, 15};
const struct Point COORD_LABEL_COMM2B		= {6, 16};
const struct Point COORD_LABEL_COMM2C		= {6, 17};
const struct Point COORD_LABEL_COMM2D		= {6, 18};
const struct Point COORD_LABEL_CMD			= {1, 20};
const struct Point COORD_VALUE_UPTIME		= {52, 2};
const struct Point COORD_VALUE_DATE			= {0, 5};
const struct Point COORD_VALUE_TIME			= {5, 6};
const struct Point COORD_VALUE_SSID_NAME	= {20, 2};
const struct Point COORD_VALUE_SSID_STATUS	= {37, 2};
const struct Point COORD_VALUE_HOST_NAME	= {20, 3};
const struct Point COORD_VALUE_HOST_STATUS	= {37, 3};
const struct Point COORD_VALUE_RELAY		= {21, 5};
const struct Point COORD_VALUE_PROX			= {21, 6};
const struct Point COORD_VALUE_TEMP			= {38, 6};
const struct Point COORD_VALUE_LOAD			= {38, 5};
const struct Point COORD_VALUE_ERROR		= {1, 32};
const struct Point COORD_VALUE_COMM1A		= {8, 9};
const struct Point COORD_VALUE_COMM1B		= {8, 10};
const struct Point COORD_VALUE_COMM1C		= {8, 11};
const struct Point COORD_VALUE_COMM1D		= {8, 12};
const struct Point COORD_VALUE_COMM2A		= {8, 15};
const struct Point COORD_VALUE_COMM2B		= {8, 16};
const struct Point COORD_VALUE_COMM2C		= {8, 17};
const struct Point COORD_VALUE_COMM2D		= {8, 18};
const struct Point COORD_VALUE_CMD			= {6, 20};

// GLOBAL VARIABLES------------------------------------------------------------
extern volatile unsigned long int _tick;
extern volatile struct ButtonInfo _button;
extern struct CommPort _comm1, _comm2;
extern const struct CommDataRegisters _comm1Regs, _comm2Regs;
extern Shell _shell;
extern struct AdcRmsInfo _adc;
extern struct ProxDetectInfo _prox;
extern unsigned char _relayState;

// FUNCTION PROTOTYPES---------------------------------------------------------
// Shell Management
void UpdateShell(void);
void ShellInitialize(CommPort* serverComm, CommPort* terminalComm,
					 unsigned int swapBufferSize, char* swapBufferData);
void ShellParseCommandLine(Buffer* buffer);
void ShellHandleSequence(CommPort* comm);
void ShellPrintBasicLayout(void);
void ShellPrintLastWarning(unsigned char row, unsigned char col);
void ShellPrintLastError(unsigned char row, unsigned char col);
// Task Management
void TaskScheduler(void);
LinkedListNode* ShellAddTask(B_Action action,
							 unsigned int runCount, unsigned long int runInterval, unsigned long int timeout,
							 bool isExclusive, bool isInfinite, bool isPeriodic,
							 unsigned char paramCount, ...);
// Tasks
bool TaskPrintTick(void);
bool TaskPrintDateTime(void);
bool TaskCalculateRMSCurrent(void);
bool TaskUpdateRelayStatus(void);
bool TaskUpdateProximityStatus(void);
bool TaskPrintTemp(void);
bool TaskConnectNetwork(void);
bool TaskConnectTcp(void);
// Button Actions
void ButtonPress(void);
void ButtonHold(void);
void ButtonRelease(void);
// Load Measurement
void InitializeLoadMeasurement(void);
double CalculateCurrentRMS(void);
// Relay Control
void RelayControl(unsigned char state);

#endif