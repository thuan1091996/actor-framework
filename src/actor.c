/******************************************************************************
* Includes
*******************************************************************************/
#include "../osal/include/chip/osal.h"
#include "actor.h" /* Free Active Object interface */
#include "mempool.h"
/******************************************************************************
* Module Preprocessor Constants
*******************************************************************************/

/******************************************************************************
* Module Preprocessor Macros
*******************************************************************************/

/******************************************************************************
* Module Typedefs
*******************************************************************************/

/******************************************************************************
* Module Variable Definitions
*******************************************************************************/
static ActiveId_t Active_List[ACTOR_MAX_NUMB]={0};




/******************************************************************************
* Private Functions
*******************************************************************************/
/*..........................................................................*/
/* Event-loop thread function for all Active Objects (FreeRTOS task signature) */
static void Active_eventLoop(void *pvParameters) {
	Active *me = (Active *)pvParameters;
	configASSERT(me); /* Active object must be provided */

	for (;;) {   /* for-ever "superloop" */

		EvtHandle_t p_e = NULL; /* Contain the memory addr of allocated evt */

		/* wait for any event and receive it into object 'e' */
		chip_os_error_t status = chip_os_queue_get(&me->equeue_handle, &p_e, CHIP_OS_TIME_FOREVER);
		if(CHIP_OS_OK == status)
		{
			configASSERT(p_e);
			/* dispatch event to the active object 'me' */
			StateMachine_Dispatch(&me->sm, p_e);			/* NO BLOCKING! */

			Event_GC(p_e);
		}

	}
}


/******************************************************************************
* Function Definitions
*******************************************************************************/
void Active_Init(Active *const				me,
				 StateHandler				initial_statehandler,
				 uint8_t 					priority,
				 uint32_t 					stack_size,
				 portTHREAD_ATTR_T const*	p_thread_attr,
				 portEQUEUE_ATTR_T const*	p_equeue_attr,
				 uint32_t					equeue_max_len)
{
	static uint8_t active_id = 0;
    configASSERT(me); /* Active object must be provided */

	/* Initialize the Event queue */
	chip_os_error_t status = chip_os_queue_init(&me->equeue_handle, sizeof(Evt *), equeue_max_len);
    configASSERT(CHIP_OS_OK == status);
	me->equeue_param = p_equeue_attr;

	/* Dispatch the initial event */
	StateMachine_Init(&me->sm, initial_statehandler);

	/* Initialize the Thread */
	status = chip_os_task_init(&me->thread_handle,
								"Actor",
								&Active_eventLoop,
								me,
								priority,
								stack_size);
    configASSERT(CHIP_OS_OK == status);
	me->thread_param = p_thread_attr;


	Active_List[active_id++] = me;
}

uint8_t Active_GetID(Active* const me)
{
	for(uint8_t idx=0; idx < ACTOR_MAX_NUMB; idx++)
	{
		if(me == Active_List[idx])
			return idx;
	}
	return 0xFF;
}

ActiveId_t Active_GetActiveByID(uint8_t id)
{
	if(id <= ACTOR_MAX_NUMB)
	{
		return Active_List[id];
	}
	return NULL;
}

/*..........................................................................*/
bool Active_post(Active * const me, EvtHandle_t const e){

	bool ret = false;
	chip_os_error_t status;
	for(uint8_t count=0; count < ACTOR_MAX_RETRY; count++)
	{
#ifndef CMSIS_RTOS2
		status = chip_os_queue_put(&me->equeue_handle, &e); /* Addr of memory location that contains addr of the event */
#else
		status = osMessageQueuePut(me->equeue_handle, &e, 0, 0);
#endif /* not defined CMSIS_RTOS2 */
		if (status == CHIP_OS_OK)
		{
			if(e->xdata.is_dynamic != 0)
			{
				portDISABLE_INTERRUPTS();
				e->xdata.ref_cnt = (e->xdata.ref_cnt + 1) % (1<<6); /* Ref counter only has 6-bit value */
				portENABLE_INTERRUPTS();
			}
			ret = true;
			break;
		}
		else
		{
			configASSERT(0);
		}
	}
	return ret;
}

uint8_t Framework_Init()
{
	return Mempool_Init();
}

