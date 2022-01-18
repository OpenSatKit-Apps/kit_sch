/*
** Purpose: Define configurations for the OpenSatKit Scheduler application 
**
** Notes:
**   1. These configurations should have an application scope and define
**      parameters that shouldn't need to change across deployments. If
**      a change is made to this file or any other app source file during
**      a deployment then the definition of the KIT_SCH_PLATFORM_REV
**      macro in kit_sch_platform_cfg.h should be updated.
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide and the
**      osk_c_demo app that illustrates best practices with comments.  
**   2. cFS Application Developer's Guide.
**
**   Written by David McComas, licensed under the Apache License, Version 2.0
**   (the "License"); you may not use this file except in compliance with the
**   License. You may obtain a copy of the License at
**
**      http://www.apache.org/licenses/LICENSE-2.0
**
**   Unless required by applicable law or agreed to in writing, software
**   distributed under the License is distributed on an "AS IS" BASIS,
**   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**   See the License for the specific language governing permissions and
**   limitations under the License.
*/


#ifndef _app_cfg_
#define _app_cfg_

/*
** Includes
*/

#include "kit_sch_platform_cfg.h"
#include "osk_c_fw.h"

/******************************************************************************
** Versions
**   
** 1.0 - Initial release
** 1.1 - Refactored for OSK 2.2
** 2.0 - Added Sch & Msg table commands and diagnostics telemetry
** 3.0 - New baseline for separate OSK app repo compatible with cFE Bootes
*/

#define  KIT_SCH_MAJOR_VER      3
#define  KIT_SCH_MINOR_VER      0


/******************************************************************************
** JSON init file definitions/declarations.
**
** CFG_STARTUP_SYNCH_TIMEOUT
**   Defines the timeout for the CFE_ES_WaitForStartupSync call during app
**   initialization. The scheduler will wait this amount of time before
**   assuming all apps have been started and will then begin nominal scheduler
**   processing.
*/

#define CFG_APP_CFE_NAME          APP_CFE_NAME
#define CFG_APP_PERF_ID           APP_PERF_ID

#define CFG_CMD_MID               CMD_MID
#define CFG_SEND_HK_MID           SEND_HK_MID
#define CFG_HK_TLM_MID            HK_TLM_MID
#define CFG_DIAG_TLM_MID          DIAG_TLM_MID
#define CFG_TBL_ENTRY_TLM_MID     TBL_ENTRY_TLM_MID

#define CFG_CMD_PIPE_NAME         CMD_PIPE_NAME
#define CFG_CMD_PIPE_DEPTH        CMD_PIPE_DEPTH

#define CFG_MSG_TBL_LOAD_FILE     MSG_TBL_LOAD_FILE
#define CFG_MSG_TBL_DUMP_FILE     MSG_TBL_DUMP_FILE

#define CFG_SCH_TBL_LOAD_FILE     SCH_TBL_LOAD_FILE
#define CFG_SCH_TBL_DUMP_FILE     SCH_TBL_DUMP_FILE

#define CFG_STARTUP_SYNC_TIMEOUT  STARTUP_SYNC_TIMEOUT

#define APP_CONFIG(XX) \
   XX(APP_CFE_NAME,char*) \
   XX(APP_PERF_ID,uint32) \
   XX(CMD_MID,uint32) \
   XX(SEND_HK_MID,uint32) \
   XX(HK_TLM_MID,uint32) \
   XX(DIAG_TLM_MID,uint32) \
   XX(TBL_ENTRY_TLM_MID,uint32) \
   XX(CMD_PIPE_NAME,char*) \
   XX(CMD_PIPE_DEPTH,uint32) \
   XX(MSG_TBL_LOAD_FILE,char*) \
   XX(MSG_TBL_DUMP_FILE,char*) \
   XX(SCH_TBL_LOAD_FILE,char*) \
   XX(SCH_TBL_DUMP_FILE,char*) \
   XX(STARTUP_SYNC_TIMEOUT,uint32) \
   
DECLARE_ENUM(Config,APP_CONFIG)


/******************************************************************************
** Command Macros
*/

#define KIT_SCH_LOAD_TBL_CMD_FC             (CMDMGR_APP_START_FC + 0)
#define KIT_SCH_DUMP_TBL_CMD_FC             (CMDMGR_APP_START_FC + 1)
#define SCHEDULER_CFG_SCH_TBL_ENTRY_CMD_FC  (CMDMGR_APP_START_FC + 2)
#define SCHEDULER_LOAD_SCH_TBL_ENTRY_CMD_FC (CMDMGR_APP_START_FC + 3)
#define SCHEDULER_SEND_SCH_TBL_ENTRY_CMD_FC (CMDMGR_APP_START_FC + 4)
#define SCHEDULER_LOAD_MSG_TBL_ENTRY_CMD_FC (CMDMGR_APP_START_FC + 5)
#define SCHEDULER_SEND_MSG_TBL_ENTRY_CMD_FC (CMDMGR_APP_START_FC + 6)
#define SCHEDULER_SEND_DIAG_TLM_CMD_FC      (CMDMGR_APP_START_FC + 7)


/******************************************************************************
** Event Macros
**
** Define the base event message IDs used by each object/component used by the
** application. There are no automated checks to ensure an ID range is not
** exceeded so it is the developer's responsibility to verify the ranges. 
**
*/

#define KIT_SCH_APP_BASE_EID  (OSK_C_FW_APP_BASE_EID +   0)
#define SCHTBL_BASE_EID       (OSK_C_FW_APP_BASE_EID + 100)
#define MSGTBL_BASE_EID       (OSK_C_FW_APP_BASE_EID + 200)
#define SCHEDULER_BASE_EID    (OSK_C_FW_APP_BASE_EID + 300)

/*
** One event ID is used for all initialization debug messages. Uncomment one of
** the KIT_SCH_INIT_EVS_TYPE definitions. Set it to INFORMATION if you want to
** see the events during initialization. This is opposite to what you'd expect 
** because INFORMATION messages are enabled by default when an app is loaded.
*/

#define KIT_SCH_INIT_DEBUG_EID 999
#define KIT_SCH_INIT_EVS_TYPE CFE_EVS_DEBUG
//#define KIT_SCH_INIT_EVS_TYPE CFE_EVS_INFORMATION


#endif /* _app_cfg_ */
