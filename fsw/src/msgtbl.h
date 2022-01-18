/* 
** Purpose: Define KIT_SCH's Message Table that provides the messages to 
**          be sent by the scheduler.
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
#ifndef _msgtbl_
#define _msgtbl_

/*
** Includes
*/

#include "app_cfg.h"
#include "cjson.h"


/***********************/
/** Macro Definitions **/
/***********************/


/*
** Event Message IDs
*/

#define MSGTBL_LOAD_EID      (MSGTBL_BASE_EID + 0)
#define MSGTBL_LOAD_ERR_EID  (MSGTBL_BASE_EID + 1)
#define MSGTBL_DUMP_EID      (MSGTBL_BASE_EID + 2)
#define MSGTBL_DUMP_ERR_EID  (MSGTBL_BASE_EID + 3)


/**********************/
/** Type Definitions **/
/**********************/

/******************************************************************************
** Message Table -  Local table copy used for table loads
** 
*/

typedef struct
{
   
   uint16   Buffer[MSGTBL_MAX_MSG_WORDS];

} MSGTBL_Entry_t;

typedef struct
{

   MSGTBL_Entry_t Entry[MSGTBL_MAX_ENTRIES];

} MSGTBL_Data_t;


typedef struct
{

   /*
   ** Table parameter data
   */
   
   MSGTBL_Data_t Data;

   /*
   ** Standard CJSON table data
   */
   
   const char*  AppName;
   boolean      Loaded;   /* Has entire table been loaded? */
   uint8        LastLoadStatus;
   uint16       LastLoadCnt;
   
   size_t       JsonObjCnt;
   char         JsonBuf[MSGTBL_JSON_FILE_MAX_CHAR];   
   size_t       JsonFileLen;
   
} MSGTBL_Class_t;


/************************/
/** Exported Functions **/
/************************/

/******************************************************************************
** Function: MSGTBL_Constructor
**
** Initialize the Message Table object.
**
** Notes:
**   1. This must be called prior to any other function.
**   2. The local table data is not populated. This is done when the table is 
**      registered with the app framework table manager.
*/
void MSGTBL_Constructor(MSGTBL_Class_t* ObjPtr, const char* AppName);


/******************************************************************************
** Function: MSGTBL_ResetStatus
**
** Reset counters and status flags to a known reset state.  The behavior of
** the table manager should not be impacted. The intent is to clear counters
** and flags to a known default state for telemetry.
**
** Notes:
**   1. See the MSGTBL_Class_t definition for the affected data.
**
*/
void MSGTBL_ResetStatus(void);


/******************************************************************************
** Function: MSGTBL_LoadCmd
**
** Command to load the table.
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
boolean MSGTBL_LoadCmd(TBLMGR_Tbl_t* Tbl, uint8 LoadType, const char* Filename);


/******************************************************************************
** Function: MSGTBL_DumpCmd
**
** Command to dump the table.
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr_t.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager.
**
*/
boolean MSGTBL_DumpCmd(TBLMGR_Tbl_t* Tbl, uint8 DumpType, const char* Filename);


/******************************************************************************
** Function: MSGTBL_SendMsg
**
** Send a SB message containing the message table entry at location EntryId.
**
** Notes:
**  1. Range checking is performed on EntryId and an event message is sent for
**     an invalid ID.
**
*/
boolean MSGTBL_SendMsg(uint16  EntryId);


#endif /* _msgtbl_ */
