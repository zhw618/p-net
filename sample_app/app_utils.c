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

#define _GNU_SOURCE /* For asprintf() */

#include "app_utils.h"
#include "app_log.h"
#include "app_gsdml.h"
#include "sampleapp_common.h"
#include "osal.h"
#include "osal_log.h" /* For LOG_LEVEL */
#include "pnal.h"
#include <pnet_api.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define GET_HIGH_BYTE(id) ((id >> 8) & 0xFF)
#define GET_LOW_BYTE(id)  (id & 0xFF)

void app_utils_ip_to_string (pnal_ipaddr_t ip, char * outputstring)
{
   snprintf (
      outputstring,
      PNAL_INET_ADDRSTR_SIZE,
      "%u.%u.%u.%u",
      (uint8_t) ((ip >> 24) & 0xFF),
      (uint8_t) ((ip >> 16) & 0xFF),
      (uint8_t) ((ip >> 8) & 0xFF),
      (uint8_t) (ip & 0xFF));
}

void app_utils_mac_to_string (pnet_ethaddr_t mac, char * outputstring)
{
   snprintf (
      outputstring,
      PNAL_ETH_ADDRSTR_SIZE,
      "%02X:%02X:%02X:%02X:%02X:%02X",
      mac.addr[0],
      mac.addr[1],
      mac.addr[2],
      mac.addr[3],
      mac.addr[4],
      mac.addr[5]);
}

const char * app_utils_submod_dir_to_string (pnet_submodule_dir_t direction)
{
   const char * s = "<error>";

   switch (direction)
   {
   case PNET_DIR_NO_IO:
      s = "NO_IO";
      break;
   case PNET_DIR_INPUT:
      s = "INPUT";
      break;
   case PNET_DIR_OUTPUT:
      s = "OUTPUT";
      break;
   case PNET_DIR_IO:
      s = "INPUT_OUTPUT";
      break;
   }

   return s;
}

const char * app_utils_ioxs_to_string (pnet_ioxs_values_t ioxs)
{
   const char * s = "<error>";
   switch (ioxs)
   {
   case PNET_IOXS_BAD:
      s = "IOXS_BAD";
      break;
   case PNET_IOXS_GOOD:
      s = "IOXS_GOOD";
      break;
   }

   return s;
}

void app_utils_get_error_code_strings (
   uint16_t err_cls,
   uint16_t err_code,
   const char ** err_cls_str,
   const char ** err_code_str)
{
   if (err_cls_str == NULL || err_cls_str == NULL)
   {
      return;
   }
   *err_cls_str = "Not decoded";
   *err_code_str = "Not decoded";

   switch (err_cls)
   {
   case PNET_ERROR_CODE_1_RTA_ERR_CLS_PROTOCOL:
      *err_cls_str = "Real-Time Acyclic Protocol";
      switch (err_code)
      {
      case PNET_ERROR_CODE_2_ABORT_AR_CONSUMER_DHT_EXPIRED:
         *err_code_str = "Device missed cyclic data "
                         "deadline, device terminated AR";
         break;
      case PNET_ERROR_CODE_2_ABORT_AR_CMI_TIMEOUT:
         *err_code_str = "Communication initialization "
                         "timeout, device terminated AR";
         break;
      case PNET_ERROR_CODE_2_ABORT_AR_RELEASE_IND_RECEIVED:
         *err_code_str = "AR release indication received";
         break;
      case PNET_ERROR_CODE_2_ABORT_DCP_STATION_NAME_CHANGED:
         *err_code_str = "DCP station name changed, "
                         "device terminated AR";
         break;
      case PNET_ERROR_CODE_2_ABORT_DCP_RESET_TO_FACTORY:
         *err_code_str = "DCP reset to factory or factory "
                         "reset, device terminated AR";
         break;
      }
      break;

   case PNET_ERROR_CODE_1_CTLDINA:
      *err_cls_str = "CTLDINA = Name and IP assignment from controller";
      switch (err_code)
      {
      case PNET_ERROR_CODE_2_CTLDINA_ARP_MULTIPLE_IP_ADDRESSES:
         *err_code_str = "Multiple users of same IP address";
         break;
      }
      break;
   }
}

void app_utils_copy_ip_to_struct (
   pnet_cfg_ip_addr_t * destination_struct,
   pnal_ipaddr_t ip)
{
   destination_struct->a = ((ip >> 24) & 0xFF);
   destination_struct->b = ((ip >> 16) & 0xFF);
   destination_struct->c = ((ip >> 8) & 0xFF);
   destination_struct->d = (ip & 0xFF);
}

const char * app_utils_dcontrol_cmd_to_string (
   pnet_control_command_t control_command)
{
   const char * s = NULL;

   switch (control_command)
   {
   case PNET_CONTROL_COMMAND_PRM_BEGIN:
      s = "PRM_BEGIN";
      break;
   case PNET_CONTROL_COMMAND_PRM_END:
      s = "PRM_END";
      break;
   case PNET_CONTROL_COMMAND_APP_RDY:
      s = "APP_RDY";
      break;
   case PNET_CONTROL_COMMAND_RELEASE:
      s = "RELEASE";
      break;
   default:
      s = "<error>";
      break;
   }

   return s;
}

const char * app_utils_event_to_string (pnet_event_values_t event)
{
   const char * s = "<error>";

   switch (event)
   {
   case PNET_EVENT_ABORT:
      s = "PNET_EVENT_ABORT";
      break;
   case PNET_EVENT_STARTUP:
      s = "PNET_EVENT_STARTUP";
      break;
   case PNET_EVENT_PRMEND:
      s = "PNET_EVENT_PRMEND";
      break;
   case PNET_EVENT_APPLRDY:
      s = "PNET_EVENT_APPLRDY";
      break;
   case PNET_EVENT_DATA:
      s = "PNET_EVENT_DATA";
      break;
   }

   return s;
}

int app_utils_pnet_cfg_init_default (pnet_cfg_t * cfg)
{
   memset (cfg, 0, sizeof (pnet_cfg_t));

   cfg->tick_us = APP_TICK_INTERVAL_US;

   /* Identification & Maintenance */

   cfg->im_0_data.im_vendor_id_hi = GET_HIGH_BYTE (APP_GSDML_VENDOR_ID);
   cfg->im_0_data.im_vendor_id_lo = GET_LOW_BYTE (APP_GSDML_VENDOR_ID);

   cfg->im_0_data.im_hardware_revision = 1;
   cfg->im_0_data.im_sw_revision_prefix = APP_GSDML_SW_REV_PREFIX;
   cfg->im_0_data.im_sw_revision_functional_enhancement = PNET_VERSION_MAJOR;
   cfg->im_0_data.im_sw_revision_bug_fix = PNET_VERSION_MINOR;
   cfg->im_0_data.im_sw_revision_internal_change = PNET_VERSION_PATCH;
   cfg->im_0_data.im_revision_counter = 0; /* Only 0 allowed according
                                                       to standard */
   cfg->im_0_data.im_profile_id = APP_GSDML_PROFILE_ID;
   cfg->im_0_data.im_profile_specific_type = APP_GSDML_PROFILE_SPEC_TYPE;
   cfg->im_0_data.im_version_major = 1;
   cfg->im_0_data.im_version_minor = 1;
   cfg->im_0_data.im_supported = APP_GSDML_IM_SUPPORTED;
   strcpy (cfg->im_0_data.im_order_id, APP_GSDML_ORDER_ID);
   strcpy (cfg->im_0_data.im_serial_number, "00001");
   strcpy (cfg->im_1_data.im_tag_function, "my function");
   strcpy (cfg->im_1_data.im_tag_location, "my location");
   strcpy (cfg->im_2_data.im_date, "2020-09-03 13:53");
   strcpy (cfg->im_3_data.im_descriptor, "my descriptor");
   strcpy (cfg->im_4_data.im_signature, ""); /* Functional safety */

   /* Device configuration */
   cfg->device_id.vendor_id_hi = GET_HIGH_BYTE (APP_GSDML_VENDOR_ID);
   cfg->device_id.vendor_id_lo = GET_LOW_BYTE (APP_GSDML_VENDOR_ID);
   cfg->device_id.device_id_hi = GET_HIGH_BYTE (APP_GSDML_DEVICE_ID);
   cfg->device_id.device_id_lo = GET_LOW_BYTE (APP_GSDML_DEVICE_ID);

   cfg->oem_device_id.vendor_id_hi = 0xc0;
   cfg->oem_device_id.vendor_id_lo = 0xff;
   cfg->oem_device_id.device_id_hi = 0xee;
   cfg->oem_device_id.device_id_lo = 0x01;
   strcpy (cfg->product_name, APP_GSDML_PRODUCT_NAME);

   cfg->send_hello = true;

   /* Timing */
   cfg->min_device_interval = APP_GSDML_MIN_DEVICE_INTERVAL;

   /* Should be set by application as part of network configuration. */
   cfg->num_physical_ports = 1;

   snprintf (
      cfg->station_name,
      sizeof (cfg->station_name),
      APP_GSDML_DEFAULT_STATION_NAME);

   /* Diagnosis mechanism */
   /* Use "Extended channel diagnosis" instead of
    * "Qualified channel diagnosis" format on the wire,
    * as this is better supported by Wireshark.
    */
   cfg->use_qualified_diagnosis = false;

   return 0;
}

int app_utils_get_netif_namelist (
   const char * arg_str,
   uint16_t max_port,
   app_utils_netif_namelist_t * p_if_list,
   uint16_t * p_num_ports)
{
   int ret = 0;
   uint16_t i = 0;
   uint16_t j = 0;
   uint16_t if_index = 0;
   uint16_t number_of_given_names = 1;
   uint16_t if_list_size = max_port + 1;
   char c;

   if (max_port == 0)
   {
      printf ("Error: max_port is 0.\n");
      return -1;
   }

   memset (p_if_list, 0, sizeof (*p_if_list));
   c = arg_str[i++];
   while (c != '\0')
   {
      if (c != ',')
      {
         if (if_index < if_list_size)
         {
            p_if_list->netif[if_index].name[j++] = c;
         }
      }
      else
      {
         if (if_index < if_list_size)
         {
            p_if_list->netif[if_index].name[j++] = '\0';
            j = 0;
            if_index++;
         }
         number_of_given_names++;
      }

      c = arg_str[i++];
   }

   if (max_port == 1 && number_of_given_names > 1)
   {
      printf ("Error: Only 1 network interface expected as max_port is 1.\n");
      return -1;
   }
   if (number_of_given_names == 2)
   {
      printf ("Error: It is illegal to give 2 interface names. Use 1, or one "
              "more than the number of physical interfaces.\n");
      return -1;
   }
   if (number_of_given_names > max_port + 1)
   {
      printf (
         "Error: You have given %u interface names, but max is %u as "
         "PNET_MAX_PHYSICAL_PORTS is %u.\n",
         number_of_given_names,
         max_port + 1,
         max_port);
      return -1;
   }

   if (number_of_given_names == 1)
   {
      if (strlen (p_if_list->netif[0].name) == 0)
      {
         printf ("Error: Zero length network interface name.\n");
         return -1;
      }
      else
      {
         p_if_list->netif[1] = p_if_list->netif[0];
         *p_num_ports = 1;
      }
   }
   else
   {
      for (i = 0; i < number_of_given_names; i++)
      {
         if (strlen (p_if_list->netif[i].name) == 0)
         {
            printf ("Error: Zero length network interface name (%d).\n", i);
            return -1;
         }
      }

      *p_num_ports = number_of_given_names - 1;
   }

   return ret;
}

int app_utils_pnet_cfg_init_netifs (
   const char * netif_list_str,
   app_utils_netif_namelist_t * if_list,
   uint16_t * number_of_ports,
   pnet_if_cfg_t * if_cfg)
{
   int ret = 0;
   int i = 0;
   pnal_ipaddr_t ip;
   pnal_ipaddr_t netmask;
   pnal_ipaddr_t gateway;

   ret = app_utils_get_netif_namelist (
      netif_list_str,
      PNET_MAX_PHYSICAL_PORTS,
      if_list,
      number_of_ports);
   if (ret != 0)
   {
      return ret;
   }
   if_cfg->main_netif_name = if_list->netif[0].name;

   for (i = 1; i <= *number_of_ports; i++)
   {
      if_cfg->physical_ports[i - 1].netif_name = if_list->netif[i].name;
   }

   /* Read IP, netmask, gateway from operating system */
   ip = pnal_get_ip_address (if_cfg->main_netif_name);
   netmask = pnal_get_netmask (if_cfg->main_netif_name);
   gateway = pnal_get_gateway (if_cfg->main_netif_name);

   app_utils_copy_ip_to_struct (&if_cfg->ip_cfg.ip_addr, ip);
   app_utils_copy_ip_to_struct (&if_cfg->ip_cfg.ip_gateway, gateway);
   app_utils_copy_ip_to_struct (&if_cfg->ip_cfg.ip_mask, netmask);

   return ret;
}

static void app_utils_print_mac_address (const char * netif_name)
{
   pnal_ethaddr_t pnal_mac_addr;
   if (pnal_get_macaddress (netif_name, &pnal_mac_addr) == 0)
   {
      APP_LOG_INFO (
         "%02X:%02X:%02X:%02X:%02X:%02X\n",
         pnal_mac_addr.addr[0],
         pnal_mac_addr.addr[1],
         pnal_mac_addr.addr[2],
         pnal_mac_addr.addr[3],
         pnal_mac_addr.addr[4],
         pnal_mac_addr.addr[5]);
   }
   else
   {
      APP_LOG_ERROR ("Failed read mac address\n");
   }
}

void app_utils_print_network_config (
   pnet_if_cfg_t * if_cfg,
   uint16_t number_of_ports)
{
   uint16_t i;
   char hostname_string[PNAL_HOSTNAME_MAX_SIZE]; /* Terminated string */

   APP_LOG_INFO ("Management port:      %s ", if_cfg->main_netif_name);
   app_utils_print_mac_address (if_cfg->main_netif_name);
   for (i = 1; i <= number_of_ports; i++)
   {
      APP_LOG_INFO (
         "Physical port [%u]:    %s ",
         i,
         if_cfg->physical_ports[i - 1].netif_name);

      app_utils_print_mac_address (if_cfg->physical_ports[i - 1].netif_name);
   }

   if (pnal_get_hostname (hostname_string) != 0)
   {
      hostname_string[0] = '\0';
   }

   APP_LOG_INFO ("Hostname:             %s\n", hostname_string);
   APP_LOG_INFO (
      "IP address:           %u.%u.%u.%u\n",
      if_cfg->ip_cfg.ip_addr.a,
      if_cfg->ip_cfg.ip_addr.b,
      if_cfg->ip_cfg.ip_addr.c,
      if_cfg->ip_cfg.ip_addr.d);
   APP_LOG_INFO (
      "Netmask:              %u.%u.%u.%u\n",
      if_cfg->ip_cfg.ip_mask.a,
      if_cfg->ip_cfg.ip_mask.b,
      if_cfg->ip_cfg.ip_mask.c,
      if_cfg->ip_cfg.ip_mask.d);
   APP_LOG_INFO (
      "Gateway:              %u.%u.%u.%u\n",
      if_cfg->ip_cfg.ip_gateway.a,
      if_cfg->ip_cfg.ip_gateway.b,
      if_cfg->ip_cfg.ip_gateway.c,
      if_cfg->ip_cfg.ip_gateway.d);
}

void app_utils_print_ioxs_change (
   app_subslot_t * subslot,
   const char * ioxs_str,
   uint8_t iocs_current,
   uint8_t iocs_new)
{
   if (iocs_current != iocs_new)
   {
      if (iocs_new == PNET_IOXS_BAD)
      {
         APP_LOG_DEBUG (
            "PLC reports %s BAD for slot %u subslot %u \"%s\"\n",
            ioxs_str,
            subslot->slot_nbr,
            subslot->subslot_nbr,
            subslot->submodule_name);
      }
      else if (iocs_new == PNET_IOXS_GOOD)
      {
         APP_LOG_DEBUG (
            "PLC reports %s GOOD for slot %u subslot %u \"%s\".\n",
            ioxs_str,
            subslot->slot_nbr,
            subslot->subslot_nbr,
            subslot->submodule_name);
      }
      else if (iocs_new != PNET_IOXS_GOOD)
      {
         APP_LOG_DEBUG (
            "PLC reports %s %u for input slot %u subslot %u \"%s\".\n"
            "  Is the PLC in STOP mode?\n",
            ioxs_str,
            iocs_new,
            subslot->slot_nbr,
            subslot->subslot_nbr,
            subslot->submodule_name);
      }
   }
}

int app_utils_plug_module (
   app_api_t * p_api,
   uint16_t slot_nbr,
   uint32_t id,
   const char * name)
{
   if (slot_nbr >= PNET_MAX_SLOTS)
   {
      return -1;
   }

   p_api->slots[slot_nbr].module_id = id;
   p_api->slots[slot_nbr].plugged = true;
   p_api->slots[slot_nbr].name = name;

   return 0;
}

int app_utils_pull_module (app_api_t * p_api, uint16_t slot_nbr)
{
   if (slot_nbr >= PNET_MAX_SLOTS)
   {
      return -1;
   }

   p_api->slots[slot_nbr].plugged = false;

   return 0;
}

app_subslot_t * app_utils_plug_submodule (
   app_api_t * p_api,
   uint16_t slot_nbr,
   uint16_t subslot_nbr,
   uint32_t submodule_ident,
   const pnet_data_cfg_t * p_data_cfg,
   const char * submodule_name,
   app_utils_cyclic_callback cyclic_callback,
   void * tag)
{
   uint16_t subslot_ix;

   if (slot_nbr >= PNET_MAX_SLOTS || p_api == NULL || p_data_cfg == NULL)
   {
      return NULL;
   }

   for (subslot_ix = 0; subslot_ix < PNET_MAX_SUBSLOTS; subslot_ix++)
   {
      if (p_api->slots[slot_nbr].subslots[subslot_ix].used == false)
      {
         app_subslot_t * p_subslot =
            &p_api->slots[slot_nbr].subslots[subslot_ix];

         p_subslot->used = true;
         p_subslot->plugged = true;
         p_subslot->slot_nbr = slot_nbr;
         p_subslot->subslot_nbr = subslot_nbr;
         p_subslot->submodule_name = submodule_name;
         p_subslot->submodule_id = submodule_ident;
         p_subslot->data_cfg = *p_data_cfg;
         p_subslot->cyclic_callback = cyclic_callback;
         p_subslot->tag = tag;
         p_subslot->iocs = 0;
         return p_subslot;
      }
   }

   return NULL;
}

int app_utils_pull_submodule (
   app_api_t * p_api,
   uint16_t slot_nbr,
   uint16_t subslot_nbr)
{
   app_subslot_t * p_subslot = NULL;

   if (slot_nbr >= PNET_MAX_SUBSLOTS || p_api == NULL)
   {
      return -1;
   }

   p_subslot = app_utils_subslot_get (p_api, slot_nbr, subslot_nbr);
   if (p_subslot != NULL)
   {
      memset (p_subslot, 0, sizeof (app_subslot_t));
      p_subslot->used = false;
      return 0;
   }

   return -1;
}

app_subslot_t * app_utils_subslot_get (
   app_api_t * p_api,
   uint16_t slot_nbr,
   uint16_t subslot_nbr)
{
   uint16_t subslot_ix;

   if (slot_nbr >= PNET_MAX_SLOTS || p_api == NULL)
   {
      return NULL;
   }

   for (subslot_ix = 0; subslot_ix < PNET_MAX_SUBSLOTS; subslot_ix++)
   {
      if (p_api->slots[slot_nbr].subslots[subslot_ix].subslot_nbr == subslot_nbr)
      {
         return &p_api->slots[slot_nbr].subslots[subslot_ix];
      }
   }

   return NULL;
}

bool app_utils_subslot_is_input (const app_subslot_t * p_subslot)
{
   if (
      p_subslot != NULL && (p_subslot->data_cfg.data_dir == PNET_DIR_INPUT ||
                            p_subslot->data_cfg.data_dir == PNET_DIR_IO))
   {
      return true;
   }

   return false;
}

bool app_utils_subslot_is_no_io (const app_subslot_t * p_subslot)
{
   if (p_subslot != NULL && p_subslot->data_cfg.data_dir == PNET_DIR_NO_IO)
   {
      return true;
   }

   return false;
}

bool app_utils_subslot_is_output (const app_subslot_t * p_subslot)
{
   if (
      p_subslot != NULL && (p_subslot->data_cfg.data_dir == PNET_DIR_OUTPUT ||
                            p_subslot->data_cfg.data_dir == PNET_DIR_IO))
   {
      return true;
   }

   return false;
}

void app_utils_cyclic_data_poll (app_api_t * p_api)
{
   uint16_t slot;
   uint16_t subslot;
   app_subslot_t * p_subslot;

   for (slot = 0; slot < PNET_MAX_SLOTS; slot++)
   {
      for (subslot = 0; subslot < PNET_MAX_SUBSLOTS; subslot++)
      {
         p_subslot = &p_api->slots[slot].subslots[subslot];
         if (p_subslot->plugged && p_subslot->cyclic_callback != NULL)
         {
            p_subslot->cyclic_callback (p_subslot, p_subslot->tag);
         }
      }
   }
}
