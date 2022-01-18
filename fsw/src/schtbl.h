/* 
** Purpose: Define KIT_SCH's Scheduler Table that defines when messages
**          defined in the message table are sent on the software bus.
**
** Notes:
**   1. Use the Singleton design pattern. A pointer to the table object
**      is passed to the constructor and saved for all other operations.
**      This is a table-specific file so it doesn't need to be re-entrant.
**   2. The table file is a JSON text file.
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
#ifndef _schtbl_
#define _schtbl_

/*
** Includes
*/

#include "app_cfg.h"
#include "cjson.h"


/***********************/
/** Macro Definitions **/
/***********************/

#define SCHTBL_UNDEF_SLOT 9999

#define SCHTBL_INDEX(slot_index,entry_index)  ((slot_index*SCHTBL_ACTIVITIES_PER_SLOT) + entry_index)


/*
** Event Message IDs
*/

#define SCHTBL_LOAD_EID       (SCHTBL_BASE_EID + 0)
#define SCHTBL_LOAD_ERR_EID   (SCHTBL_BASE_EID + 1)
#define SCHTBL_DUMP_EID       (SCHTBL_BASE_EID + 2)
#define SCHTBL_DUMP_ERR_EID   (SCHTBL_BASE_EID + 3)
#define SCHTBL_INDEX_ERR_EID  (SCHTBL_BASE_EID + 4)
#define SCHTBL_ENTRY_ERR_EID  (SCHTBL_BASE_EID + 5)

              
/**********************/
/** Type Definitions **/
/**********************/


/******************************************************************************
** Scheduler Table
**
** - Minimized SCHTBL_Entry and made word-aligned for telemetry 
*/

typedef struct
{

   bool   Enabled;
   uint8  Period;
   uint8  Offset;
   uint8  MsgTblIndex;

} SCHTBL_Entry_t;

typedef struct
{

   SCHTBL_Entry_t Entry[SCHTBL_MAX_ENTRIES];

} SCHTBL_Data_t;



/*
** Local table copy used for table load command
*/

typedef struct
{

   /*
   ** Table Parameter Data
   */
   
   SCHTBL_Data_t Data; 
   
   /*
   ** Standard CJSON table data
   */
   
   const char*  AppName;
   bool         Loaded;   /* Has entire table been loaded? */
   uint8        LastLoadStatus;
   uint16       LastLoadCnt;
   
   size_t       JsonObjCnt;
   char         JsonBuf[MSGTBL_JSON_FILE_MAX_CHAR];   
   size_t       JsonFileLen;
   
} SCHTBL_Class_t;


/************************/
/** Exported Functions **/
/************************/


/******************************************************************************
** Function: SCHTBL_Constructor
**
** Initialize a new scheduler table instance.
**
** Notes:
**   1. This method must be called prior to all other methods. The SchTbl
**      instance variable only needs to be passed to the constructor
**      because a reference is stored by schtbl.c.
**
*/
void SCHTBL_Constructor(SCHTBL_Class_t* ObjPtr, const char* AppName);


/******************************************************************************
** Function: SCHTBL_ResetStatus
**
** Reset counters and status flags to a known reset state.  The behavior of 
** the scheduler should not be impacted. The intent is to clear counters/flags 
** for telemetry.
**
** Notes:
**   1. See the SCHTBL_Class_t definition for the affected data.
**
*/
void SCHTBL_ResetStatus(void);


/******************************************************************************
** Function: SCHTBL_ProcessTable
**
** Process the scheduler table performing activities.
**
** Notes:
**   None
**
*/
bool SCHTBL_ProcessTable(void);


/******************************************************************************
** Function: SCHTBL_StartTimers
**
** Notes:
**   None
**
*/
int32 SCHTBL_StartTimers(void);


/******************************************************************************
** Function: SCHTBL_LoadCmd
**
** Command to load the table.
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
bool SCHTBL_LoadCmd(TBLMGR_Tbl_t* Tbl, uint8 LoadType, const char* Filename);


/******************************************************************************
** Function: SCHTBL_DumpCmd
**
** Command to dump the table.
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
bool SCHTBL_DumpCmd(TBLMGR_Tbl_t* Tbl, uint8 DumpType, const char* Filename);


/******************************************************************************
** SCHTBL_GetEntryIndex
**
** Compute and load EntryIndex if the SlotIndex and ActivityIndex are valid.
** Event message text assumes commands are being validated 
*/
bool SCHTBL_GetEntryIndex(const char* EventStr, uint16 SlotIndex, 
                             uint16 ActivityIndex, uint16* EntryIndex);


/******************************************************************************
** Function: SCHTBL_ValidEntry
**
** Validate table entry fields. A pointer to a structure isn't passed because
** this function is used to validate command and table parametetrs that may not
** be packed identically to the internal structure.
**
** The event string should identify the calling context such as whcih ground
** command.
*/
bool SCHTBL_ValidEntry(const char* EventStr, uint16 Enabled, uint16 Period, 
                       uint16 Offset, uint16 MsgTblIndex);
                          
#endif /* _schtbl_ */
