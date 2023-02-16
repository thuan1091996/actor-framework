
#ifndef MEMPOOL_H_
#define MEMPOOL_H_

/******************************************************************************
* Includes
*******************************************************************************/
#include "actor.h"
/******************************************************************************
* Configuration Constants
*******************************************************************************/
#define POOL_SMALL_SIZE						50
#define POOL_MEDIUM_SIZE					20
#define POOL_BIG_SIZE						10

#define POOL_SMALL_BLOCKSIZE				10
#define POOL_MEDIUM_BLOCKSIZE				20
#define POOL_BIG_BLOCKSIZE					50

#define POOL_NUMB_SIZE						3	/* SMALL, MEDIUM, BIG */

/******************************************************************************
* Macros
*******************************************************************************/



/******************************************************************************
* Typedefs
*******************************************************************************/
typedef struct
{
	osMemoryPoolId_t* Handle;
	int16_t Capacity;
	int16_t Blocksize;
}MPool_t;

/******************************************************************************
* Variables
*******************************************************************************/
extern MPool_t* Mempool_P;

/******************************************************************************
* Function Prototypes
*******************************************************************************/
bool Mempool_Init(void);

#endif /* MEMPOOL_H_ */
