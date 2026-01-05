/**
  ******************************************************************************
  * @file           : sensor_data_acquisition_strategy.h
  * @brief          : Main program body
  ******************************************************************************
*/


#ifndef SENSOR_DATA_ACQUISITION_STRATEGY_H
#define SENSOR_DATA_ACQUISITION_STRATEGY_H
/*******************************Includes **************************************/
// Standarsd includes
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"


/******************* Macros ***************************************************/
#define             TX_PERIOD_MS            10

#define             NUMBER_OF_SAMPLES       32

/************* Typedefinition *************************************************/

typedef struct stDataSample_s stDataSample_t;
typedef struct stCircuarQueue_s stCircuarQueue_t;

typedef bool                    (*pf_Queue_Check)(void);
typedef void                    (*pf_Enqueue) (stDataSample_t *);
typedef void                    (*pf_Dequeue) (stDataSample_t *);
typedef uint16_t                (*pf_Queue_Len)(void);

/***************** Structure Definitions **************************************/
// Data sample definition

#pragma pack(push, 1) // PUSHES PREVIOUS PRAGMA STATE OF COMPILER
typedef struct 
{
    uint8_t DD;
    uint8_t MM;
    uint16_t YYYY;
    uint8_t hh;
    uint8_t mm;
    uint8_t ss;
}stTimeStamp_t;
#pragma pack(pop) // POPS LATEST PRAGMA STATE OF COMPILER

#pragma pack(push, 1)
struct stDataSample_s
{
    uint8_t           u8_Sensor_Id;
    uint8_t           u8ar_Data [6];
    uint8_t           u8_Sample_Size; // upto 6 bytes
    stTimeStamp_t     st_Time;
}; 
#pragma pack(pop)

// Circular queue definition
#pragma pack(push, 1) 
struct stCircuarQueue_s
{
    uint8_t             u8_front;
    uint8_t             u8_rear;
    pf_Queue_Check      b_empty;
    pf_Queue_Check      b_full;
    pf_Enqueue          p_enq;
    pf_Dequeue          p_dequeue;
    pf_Queue_Len        p_queue_len;
    stDataSample_t starr_Data_Samples[NUMBER_OF_SAMPLES];
};
#pragma pack(pop)

/*****************Public variables declaration *********************************/


/********** Public function declaration ***************************************/
void v_Sensor_Data_Queue_Init (stCircuarQueue_t *pst_queue);
void vSensor_Data_Sample_Management (uint8_t sensor_id, const void* data, uint8_t size);
uint16_t ePacket_Formation (void);

#endif // SENSOR_DATA_ACQUISITION_STRATEGY_H