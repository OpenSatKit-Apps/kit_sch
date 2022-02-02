/*
** Purpose: Define the OpenSat Kit scheduler application. 
**
** Notes:
**   1. The scheduler object owns the message and scheduler tables
**      so it provides the tale load/dump command functions. 
**
** References:
**   1. OpenSatKit Object-based Application Developer's Guide.
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
#ifndef _kit_sch_app_
#define _kit_sch_app_

/*
** Includes
*/

#include "app_cfg.h"
#include "scheduler.h"


/***********************/
/** Macro Definitions **/
/***********************/

/*
** Events
*/

#define KIT_SCH_APP_NOOP_EID    (KIT_SCH_APP_BASE_EID + 0)
#define KIT_SCH_APP_INIT_EID    (KIT_SCH_APP_BASE_EID + 1)
#define KIT_SCH_APP_EXIT_EID    (KIT_SCH_APP_BASE_EID + 2)
#define KIT_SCH_APP_MID_ERR_EID (KIT_SCH_APP_BASE_EID + 3)
#define KIT_SCH_APP_DEBUG_EID   (KIT_SCH_APP_BASE_EID + 4)


/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Command Packets
*/


/******************************************************************************
** Telemetry Packets
*/

typedef struct
{

   CFE_MSG_TelemetryHeader_t TlmHeader;

   /*
   ** CMDMGR Data
   */
   uint16   ValidCmdCnt;
   uint16   InvalidCmdCnt;

   /*
   ** TBLMGR Data
   */

   uint8    MsgTblLastLoadStatus;
   uint8    SchTblLastLoadStatus;

   uint16   MsgTblAttrErrCnt;
   uint16   SchTblAttrErrCnt;

   /*
   ** SCHTBL Data
   ** - At a minimum every sch-tbl variable effected by a reset must be included
   ** - These have been rearranged to align data words
   */

   uint32  SlotsProcessedCount;
   uint32  ScheduleActivitySuccessCount;
   uint32  ScheduleActivityFailureCount;
   uint32  ValidMajorFrameCount;
   uint32  MissedMajorFrameCount;
   uint32  UnexpectedMajorFrameCount;
   uint32  TablePassCount;
   uint32  ConsecutiveNoisyFrameCounter;
   uint16  SkippedSlotsCount;
   uint16  MultipleSlotsCount;
   uint16  SameSlotCount;
   uint16  SyncAttemptsLeft;
   uint16  LastSyncMETSlot;
   bool    IgnoreMajorFrame;
   bool    UnexpectedMajorFrame;

} KIT_SCH_HkPkt_t;
#define KIT_SCH_HK_TLM_LEN sizeof (KIT_SCH_HkPkt_t)


/******************************************************************************
** KIT_SCH_Class
*/
typedef struct
{
   
   /* 
   ** App Framework
   */
   
   INITBL_Class_t   IniTbl; 
   CFE_SB_PipeId_t  CmdPipe;
   CMDMGR_Class_t   CmdMgr;
   TBLMGR_Class_t   TblMgr;

   
   /*
   ** Telemetry Packets
   */
   
   KIT_SCH_HkPkt_t  HkPkt;


   /*
   ** KIT_SCH State & Contained Objects
   */
   
   uint32   StartupSyncTimeout;
   CFE_SB_MsgId_t   CmdMid;
   CFE_SB_MsgId_t   SendHkMid;
   
   SCHEDULER_Class_t  Scheduler;
  
} KIT_SCH_Class;


/*******************/
/** Exported Data **/
/*******************/

extern KIT_SCH_Class  KitSch;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: KIT_SCH_Main
**
*/
void KIT_SCH_Main(void);


/******************************************************************************
** Function: KIT_SCH_NoOpCmd
**
** Notes:
**   1. Function signature must match the CMDMGR_CmdFuncPtr_t definition
**
*/
bool KIT_SCH_NoOpCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


/******************************************************************************
** Function: KIT_SCH_ResetAppCmd
**
** Notes:
**   1. Function signature must match the CMDMGR_CmdFuncPtr_t definition
**
*/
bool KIT_SCH_ResetAppCmd(void* ObjDataPtr, const CFE_MSG_Message_t *MsgPtr);


#endif /* _kit_sch_app_ */

