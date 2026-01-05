/**
  ******************************************************************************
  * @file           : sensor_data_acquisition_strategy.c
  * @brief          : Main program body
  ******************************************************************************
*/

/*******************************Includes **************************************/
// Standarsd includes
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "math.h"
// Application includes
#include "sensor_interface.h"
#include "communication_interface.h"
#include "sensor_data_acquisition_strategy.h"

/******************* Macros ***************************************************/

/************* Typedefinition *************************************************/


/***************** Structure Definitions **************************************/



/***************** Private variables ******************************************/

/******************** Extern Variables ****************************************/
extern uint64_t u64_get_tick_count;
extern stCircuarQueue_t     st_Sensor_Data_Queue;
extern uint8_t              u8arr_packet [200]; // to acquire sensors data

/***************** Function Definitions ***************************************/
/**
 * @brief       This function checks if the circular queue is full.
 * @param in    None
 * @param out   None
 * @return      1 -> Full, 0 -> Not Full
*/
static bool isFull(void)
{
    if ( (st_Sensor_Data_Queue.u8_rear + 1)%NUMBER_OF_SAMPLES ==  st_Sensor_Data_Queue.u8_front)
    {
        return 1;
    }
    return 0;
}


/**
 * @brief       This function checks if the circular queue is empty.
 * @param in    None
 * @param out   None
 * @return      1 -> Empty, 0 -> Not Empty
*/
static bool isEmpty(void)
{
    if (st_Sensor_Data_Queue.u8_front == st_Sensor_Data_Queue.u8_rear)
    {
        return 1;
    }
    return 0;
}

/**
 * @brief       This function enqueues a sample.
 * @param in    *st_sample : adrees of sample to be queued
 * @param out   None
 * @return      None
*/
static void enQueue (stDataSample_t *st_sample)
{
    if (!st_Sensor_Data_Queue.b_full())
    {
        st_Sensor_Data_Queue.starr_Data_Samples[st_Sensor_Data_Queue.u8_rear] = *st_sample;

        st_Sensor_Data_Queue.u8_rear = (st_Sensor_Data_Queue.u8_rear + 1)%NUMBER_OF_SAMPLES;
    }

}


/**
 * @brief       This function dequeues a sample.
 * @param in    None
 * @param out   None
 * @return      None
*/
static void deQueue (stDataSample_t *st_sample)
{
    if (!st_Sensor_Data_Queue.b_empty())
    { 
        *st_sample =  st_Sensor_Data_Queue.starr_Data_Samples[st_Sensor_Data_Queue.u8_front];

        st_Sensor_Data_Queue.u8_front = (st_Sensor_Data_Queue.u8_front + 1)%NUMBER_OF_SAMPLES;
    }

}


/**
 * @brief       This function counts number of elements in queue
 * @param in    None
 * @param out   None
 * @return      Length of the queue
*/
static uint16_t queue_length (void)
{
    if (st_Sensor_Data_Queue.b_empty())
    {
        return 0;
    }
    else
    {
        if ( st_Sensor_Data_Queue.u8_front < st_Sensor_Data_Queue.u8_rear )
        {
            return (st_Sensor_Data_Queue.u8_rear - st_Sensor_Data_Queue.u8_front);
        }
        else
        {
            return (NUMBER_OF_SAMPLES - (st_Sensor_Data_Queue.u8_front - st_Sensor_Data_Queue.u8_rear));
        }
    }

    return 0;

}
/************************************************************************* */


//sensor_data_callback(uint8_t sensor_id, const void* data, uint8_t size) -->give data 

/**************************  Initialize a circular queue **********************/

/**
 * @brief       This function initializes a circular queue.
 * @param in    pst_queue - pointer to queue
 * @param out   None
 * @return      None
*/
void v_Sensor_Data_Queue_Init (stCircuarQueue_t *pst_queue)
{
    pst_queue->u8_front = 0;
    pst_queue->u8_rear = 0;
    pst_queue->b_empty = isEmpty;
    pst_queue->b_full = isFull;
    pst_queue->p_enq = enQueue;
    pst_queue->p_dequeue = deQueue;
    pst_queue->p_queue_len = queue_length;
}

/**
 * @brief       Returns times (Not date) by using system tick @1ms
 * @param in    None
 * @param out   None
 * @return      None
*/
static stTimeStamp_t getTimeStamp( void )
{
    stTimeStamp_t st_var = {0};
    float tot_sec_spent = u64_get_tick_count/1000.0;

    // float tot_min_spent =  tot_sec_spent/60.0;
    // float tot_hr_spent  = tot_min_spent/60.0;

    float tot_hr_spent  = tot_sec_spent/3600.0;

    st_var.hh = tot_hr_spent;

    float remain_sec = fmodf (tot_sec_spent , 3600.0);

    float tot_min_spent =  fmodf (remain_sec , 60.0);

    st_var.mm = tot_min_spent;

    remain_sec = fmodf(tot_min_spent , 60.0);

    st_var.ss = remain_sec;




    // float tot_days_spent   = tot_hr_spent/24.0;
    // float tot_months_spent = tot_days_spent/30.0;
    // float tot_years_spent = tot_months_spent/12.0;

    // st_var.DD = 0;
    // st_var.MM = 0;
    // st_var.YYYY = 0;

    // st_var.hh = tot_hr_spent;
    // st_var.mm = tot_min_spent;

    return st_var;

}

/**
 * @brief       This function enques a sensor sample whenever available using the
 *              API sensor_data_callback() given. It is a data producer routine
 *              implemented on MCU1. It manages the incoming data stream from sensors.
 * @param in    None
 * @param out   None
 * @return      None
*/
void vSensor_Data_Sample_Management (uint8_t sensor_id, const void* data, uint8_t size)
{
    stDataSample_t st_Data_Sample;

    // Create a sample 
    st_Data_Sample.u8_Sensor_Id = sensor_id;

    memcpy( &(st_Data_Sample.u8ar_Data[0]), (uint8_t*)data, size);

    st_Data_Sample.u8_Sample_Size = size;

    st_Data_Sample.st_Time = getTimeStamp();

    // Enqueue the created sample to the circular queue

    st_Sensor_Data_Queue.p_enq (&st_Data_Sample);
}



/**
 * @brief       This function forms a packet to send to MCU2 wirelessly.
 * @param in    None
 * @param out   None
 * @return      Packet size formed
*/

uint16_t ePacket_Formation (void)
{
    uint8_t         *pu8_data_packet = u8arr_packet; 
    stDataSample_t  st_sensor_sample = {0};
    uint16_t        u16_packet_size = 0;
    
    // Enter Crirical Section
    while ( (!st_Sensor_Data_Queue.b_empty()) && (u16_packet_size <= 200))
    {
        st_Sensor_Data_Queue.p_dequeue( &st_sensor_sample );

        memcpy(pu8_data_packet, (uint8_t*)&st_sensor_sample, sizeof(stDataSample_t));

        pu8_data_packet += sizeof(stDataSample_t);

        u16_packet_size += sizeof(stDataSample_t);
    }
    // Exit critical section

    return u16_packet_size/sizeof(stDataSample_t);
}

