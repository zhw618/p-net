/*********************************************************************
 *        _       _         _
 *  _ __ | |_  _ | |  __ _ | |__   ___
 * | '__|| __|(_)| | / _` || '_ \ / __|
 * | |   | |_  _ | || (_| || |_) |\__ \
 * |_|    \__|(_)|_| \__,_||_.__/ |___/
 *
 * www.rt-labs.com
 * Copyright 2021 rt-labs AB, Sweden.
 *
 * This software is dual-licensed under GPLv3 and a commercial
 * license. See the file LICENSE.md distributed with this software for
 * full license information.
 ********************************************************************/

#include "app_data.h"
#include "app_utils.h"
#include "app_gsdml.h"
#include "app_log.h"
#include "sampleapp_common.h"
#include "osal.h"
#include "pnal.h"
#include <pnet_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define APP_DATA_DEFAULT_OUTPUT_DATA 0

/* Parameters data
 * Todo: Data is always in pnio data format. Add conversion to uint32_t.
 */
uint32_t app_param_1 = 0;
uint32_t app_param_2 = 0;

/* Process data */
uint8_t inputdata[APP_GSDML_INPUT_DATA_SIZE] = {0};
uint8_t outputdata[APP_GSDML_OUTPUT_DATA_SIZE] = {0};
/* EtherCAT Servo PDO data, SHM Mapping */
uint8_t TxPDOdata1[APP_GSDML_TxPDO_DATA_SIZE];  //Transmit PDO, input
//uint8_t * pRxPDOdata1;  //Received PDO, output

//共享内存 SHM 指针, 全局变量
extern uint8_t * pSHMMapAddr;

/**
 * Set LED state.
 *
 * Compares new state with previous state, to minimize system calls.
 *
 * Uses the hardware specific app_set_led() function.
 *
 * @param led_state        In:    New LED state
 */
static void app_handle_data_led_state (bool led_state)
{
   static bool previous_led_state = false;

   if (led_state != previous_led_state)
   {
      app_set_led (APP_DATA_LED_ID, led_state);
   }
   previous_led_state = led_state;
}

/* ***************************************************
 * 【重要】为所有插槽处的 submod 提供Input数据
 *        即：在此函数中准备所有传给PLC的 data！
 * ***************************************************/
uint8_t * app_data_get_input_data (
   uint16_t slot_nbr,     //添加上 slot_nbr
   uint16_t subslot_nbr,  //添加上 subslot_nbr
   uint32_t submodule_id,
   bool button_pressed,
   uint8_t counter,
   uint16_t * size,  //[out参数]
   uint8_t * iops)   //[out参数]
{
   if (size == NULL || iops == NULL)
   {
      return NULL;
   }
   
   /* ===================================================================
    * Prepare input data
    * Lowest 7 bits: Counter    Most significant bit: Button
    * 
    * 为DI/DIO/PDO 这3个支持的模块, 分别准备 input 数据(字节长度也不同)
    * 注意: 为了简化起见, 此处无论 module 插到哪个 solt 上, 准备的数据都相同!
    * =================================================================== */
   if(submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN ||
      submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN_OUT )
   {
      inputdata[0] = counter;
      if (button_pressed)
      {
         inputdata[0] |= 0x80;
      }
      else
      {
         inputdata[0] &= 0x7F;
      }

      //结果输出和返回
      *size = APP_GSDML_INPUT_DATA_SIZE;
      *iops = PNET_IOXS_GOOD;

      return inputdata;
   }
   /* ===================================================================
    * 为 PDO CiA402 伺服控制器模块, 准备 input数据(共40字节)

    * 注意: 为了简化起见, 此处无论 module 插到哪个 solt 上, 准备的数据都相同!
    * =================================================================== */
   else if(submodule_id == APP_GSDML_SUBMOD_ID_BYTE_IN_OUT)
   {
      //地址映射到 SHM的 TxPDO 区, 偏移量offset=19
      memset(TxPDOdata1, 0, APP_GSDML_TxPDO_DATA_SIZE); //40字节全清零
      memcpy(TxPDOdata1, pSHMMapAddr + 19, 25); // TxPDO 仅提供 25 Bytes 数据

      //结果输出和返回
      *size = APP_GSDML_TxPDO_DATA_SIZE;
      *iops = PNET_IOXS_GOOD;

      return TxPDOdata1;
   }
   /* 目前只支持以上3种module能给PLC发送Input数据, 其他模块则报错! */ 
   else
   {
      /* Automated RT Tester scenario 2 - unsupported (sub)module */
      *iops = PNET_IOXS_BAD;
      return NULL;
   }

   //以下不可到达
   return NULL;
}

/* *******************************************************
 * 【重要】处理所有插槽处 module/submod 接收到的Output数据
 *        即：在此函数中将从PLC接收到的所有 data 应用到外界！
 * *******************************************************/
int app_data_set_output_data (
   uint16_t slot_nbr,     //添加上 slot_nbr
   uint16_t subslot_nbr,  //添加上 subslot_nbr
   uint32_t submodule_id,
   uint8_t * data,        //[in参数]
   uint16_t size)
{
   if (size == 0 || data == NULL)  //无数据
   {
      return -1;
   }

    // ===================================================================
    // 为DO、DIO 和 PDO 这3个支持的模块, 分别消费掉其 output 数据(字节长度也不同)
    // 注意: 为了简化起见, 此处无论 module 插到哪个 solt 上, 消耗outdate的方式都相同!
    // =================================================================== 
   if( submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_OUT ||
       submodule_id == APP_GSDML_SUBMOD_ID_DIGITAL_IN_OUT )
   {
      bool led_state;

      memcpy (outputdata, data, size);
      led_state = (outputdata[0] & 0x80) > 0;
      app_handle_data_led_state (led_state);
      return 0;      
   }
   /* ==========================================================================
    * 为 PDO CiA402 伺服控制器模块, 准备从PLC接收到的 output数据(共40字节)

    * 注意: 为了简化起见, 此处无论 module 插到哪个 solt 上, 处理outdata的方式都相同!
    * ========================================================================== */
   else if(submodule_id == APP_GSDML_SUBMOD_ID_BYTE_IN_OUT)
   {
      //接收到的data 拷贝到 SHM的 RxPDO 区, 偏移量offset=0, 长度19字节
      //memcpy (pSHMMapAddr, data, size);
      memcpy (pSHMMapAddr, data, 19);

      //成功返回
       return 0;
   }
   /* 目前只支持以上3种module能从PLC接收Output数据, 若为其他模块则报错! */ 
   else
   {
      return -1;
   }
   
}

int app_data_set_default_outputs (void)
{
   outputdata[0] = APP_DATA_DEFAULT_OUTPUT_DATA;
   app_handle_data_led_state (false);
   return 0;
}

int app_data_write_parameter (
   uint32_t submodule_id,
   uint32_t index,
   const uint8_t * data,
   uint16_t length)
{
   const app_gsdml_param_t * par_cfg;

   par_cfg = app_gsdml_get_parameter_cfg (submodule_id, index);
   if (par_cfg == NULL)
   {
      APP_LOG_WARNING (
         "PLC write request unsupported submodule/parameter. "
         "Submodule id: %u Index: %u\n",
         (unsigned)submodule_id,
         (unsigned)index);
      return -1;
   }

   if (length != par_cfg->length)
   {
      APP_LOG_WARNING (
         "PLC write request unsupported length. "
         "Index: %u Length: %u Expected length: %u\n",
         (unsigned)index,
         (unsigned)length,
         par_cfg->length);
      return -1;
   }

   if (index == APP_GSDM_PARAMETER_1_IDX)
   {
      memcpy (&app_param_1, data, length);
   }
   else if (index == APP_GSDM_PARAMETER_2_IDX)
   {
      memcpy (&app_param_2, data, length);
   }
   APP_LOG_DEBUG ("  Writing %s\n", par_cfg->name);
   app_log_print_bytes (APP_LOG_LEVEL_DEBUG, data, length);

   return 0;
}

int app_data_read_parameter (
   uint32_t submodule_id,
   uint32_t index,
   uint8_t ** data,
   uint16_t * length)
{
   const app_gsdml_param_t * par_cfg;

   par_cfg = app_gsdml_get_parameter_cfg (submodule_id, index);
   if (par_cfg == NULL)
   {
      APP_LOG_WARNING (
         "PLC read request unsupported submodule/parameter. "
         "Submodule id: %u Index: %u\n",
         (unsigned)submodule_id,
         (unsigned)index);
      return -1;
   }

   if (*length < par_cfg->length)
   {
      APP_LOG_WARNING (
         "PLC read request unsupported length. "
         "Index: %u Length: %u Expected length: %u\n",
         (unsigned)index,
         (unsigned)*length,
         par_cfg->length);
      return -1;
   }

   APP_LOG_DEBUG ("  Reading %s\n", par_cfg->name);
   if (index == APP_GSDM_PARAMETER_1_IDX)
   {
      *data = (uint8_t *)&app_param_1;
      *length = sizeof (app_param_1);
   }
   else if (index == APP_GSDM_PARAMETER_2_IDX)
   {
      *data = (uint8_t *)&app_param_2;
      *length = sizeof (app_param_2);
   }

   app_log_print_bytes (APP_LOG_LEVEL_DEBUG, *data, *length);

   return 0;
}
