/* 
** Purpose: Implement KIT_SCH's Message Table management functions.
**
** Notes:
**   None
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

/*
** Include Files:
*/

#include <string.h>
#include "cfe_endian.h"
#include "msgtbl.h"

/***********************/
/** Macro Definitions **/
/***********************/

#define JSON_DATA_WORD_STR_MAX 32

/**********************/
/** Type Definitions **/
/**********************/

/* See LoadJsonData()prologue for details */
typedef CJSON_IntObj_t JsonId_t;
typedef CJSON_IntObj_t JsonStreamId_t;
typedef CJSON_IntObj_t JsonSeqSeg_t;
typedef CJSON_IntObj_t JsonLength_t;
typedef struct
{
   CJSON_Obj_t  Obj;
   char         Value[JSON_DATA_WORD_STR_MAX];
} JsonDataWords_t;


typedef struct
{
   JsonId_t         Id;
   JsonStreamId_t   StreamId;
   JsonSeqSeg_t     SeqSeg;
   JsonLength_t     Length;
   JsonDataWords_t  DataWords;
     
} JsonMessage_t;


/************************************/
/** Local File Function Prototypes **/
/************************************/

static void ConstructJsonMessage(JsonMessage_t* JsonMessage, uint16 MsgArrayIdx);
static bool LoadJsonData(size_t JsonFileLen);
static char *SplitStr(char *Str, const char *Delim);

/**********************/
/** Global File Data **/
/**********************/

static MSGTBL_Class_t* MsgTbl = NULL;
static MSGTBL_Data_t   TblData;        /* Working buffer for loads */


/******************************************************************************
** Function: MSGTBL_Constructor
**
** Notes:
**    1. This must be called prior to any other functions
**
*/
void MSGTBL_Constructor(MSGTBL_Class_t*  ObjPtr, const char* AppName)
{
   
   MsgTbl = ObjPtr;

   CFE_PSP_MemSet(MsgTbl, 0, sizeof(MSGTBL_Class_t));

   MsgTbl->AppName        = AppName;
   MsgTbl->LastLoadStatus = TBLMGR_STATUS_UNDEF;

} /* End MSGTBL_Constructor() */


/******************************************************************************
** Function: MSGTBL_ResetStatus
**
*/
void MSGTBL_ResetStatus(void)
{
   
   MsgTbl->LastLoadStatus = TBLMGR_STATUS_UNDEF;
   MsgTbl->LastLoadCnt = 0;
    
} /* End MSGTBL_ResetStatus() */


/******************************************************************************
** Function: MSGTBL_LoadCmd
**
** Notes:
**  1. Function signature must match TBLMGR_LoadTblFuncPtr.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager that has verified the file.
*/
bool MSGTBL_LoadCmd(TBLMGR_Tbl_t* Tbl, uint8 LoadType, const char* Filename)
{

   bool  RetStatus = false;

   if (CJSON_ProcessFile(Filename, MsgTbl->JsonBuf, MSGTBL_JSON_FILE_MAX_CHAR, LoadJsonData))
   {
      MsgTbl->Loaded = true;
      MsgTbl->LastLoadStatus = TBLMGR_STATUS_VALID;
      RetStatus = true;
   }
   else
   {
      MsgTbl->LastLoadStatus = TBLMGR_STATUS_INVALID;
   }

   return RetStatus;
   
} /* End MSGTBL_LoadCmd() */


/******************************************************************************
** Function: MSGTBL_DumpCmd
**
** Notes:
**  1. Function signature must match TBLMGR_DumpTblFuncPtr.
**  2. Can assume valid table file name because this is a callback from 
**     the app framework table manager that has verified the file. If the
**     filename exists it will be overwritten.
**  3. File is formatted so it can be used as a load file. 
**  4. DumpType is unused.
*/

bool MSGTBL_DumpCmd(TBLMGR_Tbl_t* Tbl, uint8 DumpType, const char* Filename)
{

   bool        RetStatus = false;
   int32       OsStatus;
   osal_id_t   FileHandle;
   int32       i, d;
   char        DumpRecord[256];
   char        SysTimeStr[64];
   uint16      DataWords;
   os_err_name_t      OsErrStr;
   CFE_MSG_Size_t     MsgBytes;
   const MSGTBL_Tbl_t *MsgTblPtr;
   
   OsStatus = OS_OpenCreate(&FileHandle, Filename, OS_FILE_FLAG_CREATE | OS_FILE_FLAG_TRUNCATE, OS_READ_WRITE);

   if (OsStatus == OS_SUCCESS)
   {

      sprintf(DumpRecord,"{\n   \"app-name\": \"%s\",\n   \"tbl-name\": \"Message\",\n",MsgTbl->AppName);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      CFE_TIME_Print(SysTimeStr, CFE_TIME_GetTime());
      sprintf(DumpRecord,"   \"description\": \"Table dumped at %s\",\n",SysTimeStr);
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      /* 
      ** Message Array 
      **
      ** - Not all fields in ground table are saved in FSW so they are not
      **   populated in the dump file. However, the dump file can still
      **   be loaded.
      **
      **   "name":  Not loaded,
      **   "descr": Not Loaded,
      **   "id": 101,
      **   "stream-id": 65303,
      **   "seq-seg": 192,
      **   "length": 1792,
      **   "data-words": "0,1,2,3,4,5"
      */
      
      sprintf(DumpRecord,"\"message-array\": [\n");
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      for (i=0; i < MSGTBL_MAX_ENTRIES; i++)
      {
         
         if (i > 0)  /* Complete previous entry */
         { 
            sprintf(DumpRecord,",\n");
            OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
         }
          
         sprintf(DumpRecord,"   {\"message\": {\n");
         OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
         
         sprintf(DumpRecord,"      \"id\": %d,\n      \"stream-id\": %d,\n      \"seq-seg\": %d,\n      \"length\": %d",
                 i,
                 CFE_MAKE_BIG16(MsgTblPtr->Entry[i].Buffer[0]),
                 CFE_MAKE_BIG16(MsgTblPtr->Entry[i].Buffer[1]),
                 CFE_MAKE_BIG16(MsgTblPtr->Entry[i].Buffer[2]));
         OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
         
         /*
         ** DataWords is everything past the primary header so they include
         ** the secondary header and don't distinguish between cmd or tlm
         ** packets. 
         */
         
         // TODO - cFE7.0 Rethink table itself. Either only allow commands or use message type to get header length and data words will truly just be payload
         
         CFE_MSG_GetSize((const CFE_MSG_Message_t *)MsgTblPtr->Entry[i].Buffer, &MsgBytes);

         DataWords = (MsgBytes-PKTUTIL_PRI_HDR_BYTES)/2;
         if (DataWords > (uint8)(MSGTBL_MAX_MSG_WORDS))
         {
            
            CFE_EVS_SendEvent(MSGTBL_DUMP_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Error creating dump file message entry %d. Message word length %d is greater than max data buffer %ld",
                              i, DataWords, PKTUTIL_PRI_HDR_WORDS);         
         }
         else
         {

            /* 
            ** Omit "data-words" property if no data
            ** - Properly terminate 'length' line 
            */
            if (DataWords > 0)
            {
         
               sprintf(DumpRecord,",\n      \"data-words\": \"");         
               OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
                  
               for (d=0; d < DataWords; d++)
               {
                  
                  if (d == (DataWords-1))
                  {
                     sprintf(DumpRecord,"%d\"\n   }}",MsgTbl->Data.Entry[i].Buffer[PKTUTIL_PRI_HDR_WORDS+d]);
                  }
                  else
                  {
                     sprintf(DumpRecord,"%d,",MsgTbl->Data.Entry[i].Buffer[PKTUTIL_PRI_HDR_WORDS+d]);
                  }
                  OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

               } /* End DataWord loop */
                           
            } /* End if non-zero data words */
            else
            {
               sprintf(DumpRecord,"\n   }}");         
               OS_write(FileHandle,DumpRecord,strlen(DumpRecord));
            }
         } /* End if DataWords within range */

      } /* End message loop */

      /* Close message-array and top-level object */      
      sprintf(DumpRecord,"\n]}\n");
      OS_write(FileHandle,DumpRecord,strlen(DumpRecord));

      RetStatus = true;

      OS_close(FileHandle);

      CFE_EVS_SendEvent(MSGTBL_DUMP_EID, CFE_EVS_EventType_INFORMATION,
                        "Successfully dumped message table to %s", Filename);

   } /* End if file create */
   else
   {
      OS_GetErrorName(OsStatus,&OsErrStr);
      CFE_EVS_SendEvent(MSGTBL_DUMP_ERR_EID, CFE_EVS_EventType_ERROR,
                        "Error creating dump file '%s', status=%s",
                        Filename, OsErrStr);
   
   } /* End if file create error */

   
   return RetStatus;
   
} /* End of MSGTBL_DumpCmd() */


/******************************************************************************
** Function: ConstructJsonMessage
**
*/
static void ConstructJsonMessage(JsonMessage_t* JsonMessage, uint16 MsgArrayIdx)
{

   char KeyStr[64];

   sprintf(KeyStr,"message-array[%d].message.id", MsgArrayIdx);
   CJSON_ObjConstructor(&JsonMessage->Id.Obj, KeyStr, JSONNumber, &JsonMessage->Id.Value, 4);

   sprintf(KeyStr,"message-array[%d].message.stream-id", MsgArrayIdx);
   CJSON_ObjConstructor(&JsonMessage->StreamId.Obj, KeyStr, JSONNumber, &JsonMessage->StreamId.Value, 4);

   sprintf(KeyStr,"message-array[%d].message.seq-seg", MsgArrayIdx);
   CJSON_ObjConstructor(&JsonMessage->SeqSeg.Obj, KeyStr, JSONNumber, &JsonMessage->SeqSeg.Value, 4);

   sprintf(KeyStr,"message-array[%d].message.length", MsgArrayIdx);
   CJSON_ObjConstructor(&JsonMessage->Length.Obj, KeyStr, JSONNumber, &JsonMessage->Length.Value, 4);

   sprintf(KeyStr,"message-array[%d].message.data-words", MsgArrayIdx);
   CJSON_ObjConstructor(&JsonMessage->DataWords.Obj, KeyStr, JSONString, JsonMessage->DataWords.Value, JSON_DATA_WORD_STR_MAX);

} /* ConstructJsonMessage() */


/******************************************************************************
** Function: LoadJsonData
**
** Notes:
**  1. The JSON file can contain 1 to MSGTBL_MAX_ENTRIES entries. The table can
**     be sparsely populated. The Scheduler Table uses indices into the Message
**     Table and it's the developer's responsibility to make sure they are
**     defined correctly.  
**  2. JSON message object
**
**        "name":  Not saved,
**        "descr": Not saved,
**        "id": 101,
**        "stream-id": 65303,
**        "seq-seg": 192,
**        "length": 1792,
**        "data-words": "0,1,2,3,4,5"  # Optional field
**
**        The data words contain the secondary header if it is present. No
**        integrity checks are made on the packet.
*/
static boolean LoadJsonData(size_t JsonFileLen)
{

   bool    RetStatus = true;
   bool    ReadMsg = true;
   uint16  i;
   uint16  AttributeCnt;
   uint16  MsgArrayIdx;
   char*   DataStrPtr;

   JsonMessage_t  JsonMessage;
   MSGTBL_Entry_t MsgEntry;


   MsgTbl->JsonFileLen = JsonFileLen;

   /* 
   ** 1. Copy table owner data into local table buffer
   ** 2. Process JSON file which updates local table buffer with JSON supplied values
   ** 3. If valid, copy local buffer over owner's data 
   */
   
   memcpy(&TblData, &MsgTbl->Data, sizeof(MSGTBL_Data_t));

   MsgArrayIdx = 0;
   while (ReadMsg)
   {

      ConstructJsonMessage(&JsonMessage, MsgArrayIdx);
      memset((void*)&MsgEntry,0,sizeof(MSGTBL_Entry_t));

      /*
      ** Use 'id' field to determine whether processing the file
      ** is complete. A missing or malformed 'id' field error will
      ** not be caught or reported.
      ** The 'id' field is required but CJSON_LoadObjOptional() is
      ** used so the 'object not found' event will be suppressed 
      */      
      
      if (CJSON_LoadObjOptional(&JsonMessage.Id.Obj, MsgTbl->JsonBuf, MsgTbl->JsonFileLen))
      {
         if (JsonMessage.Id.Value < MSGTBL_MAX_ENTRIES)
         {
            
            AttributeCnt = 0;
            if (CJSON_LoadObj(&JsonMessage.StreamId.Obj, MsgTbl->JsonBuf, MsgTbl->JsonFileLen)) AttributeCnt++;
            if (CJSON_LoadObj(&JsonMessage.SeqSeg.Obj,   MsgTbl->JsonBuf, MsgTbl->JsonFileLen)) AttributeCnt++;
            if (CJSON_LoadObj(&JsonMessage.Length.Obj,   MsgTbl->JsonBuf, MsgTbl->JsonFileLen)) AttributeCnt++;
            
            if (AttributeCnt == 3)
            {
               
               MsgEntry.Buffer[0] = CFE_MAKE_BIG16((uint16)JsonMessage.StreamId.Value);
               MsgEntry.Buffer[1] = CFE_MAKE_BIG16((uint16)JsonMessage.SeqSeg.Value);
               MsgEntry.Buffer[2] = CFE_MAKE_BIG16((uint16)JsonMessage.Length.Value);
              
               if (CJSON_LoadObjOptional(&JsonMessage.DataWords.Obj, MsgTbl->JsonBuf, MsgTbl->JsonFileLen))
               {
                  if (strlen(JsonMessage.DataWords.Value) > 0)
                  {
                     i = 3;
                     /* No protection against malformed data array */
                     DataStrPtr = SplitStr(JsonMessage.DataWords.Value,",");
                     if (DataStrPtr != NULL)
                     {
                        MsgEntry.Buffer[i++] = atoi(DataStrPtr);
                        CFE_EVS_SendEvent(KIT_SCH_INIT_DEBUG_EID, KIT_SCH_INIT_EVS_TYPE,
                                          "MSGTBL::LoadJsonData data[%d] = 0x%4X, DataStrPtr=%s\n",
                                          i-1,MsgEntry.Buffer[i-1],DataStrPtr);         
                        while ((DataStrPtr = SplitStr(NULL,",")) != NULL)
                        {
                           MsgEntry.Buffer[i++] = atoi(DataStrPtr);
                           CFE_EVS_SendEvent(KIT_SCH_INIT_DEBUG_EID, KIT_SCH_INIT_EVS_TYPE,
                                             "MSGTBL::LoadJsonData data[%d] = 0x%4X, DataStrPtr=%s\n",i-1,MsgEntry.Buffer[i-1],DataStrPtr);
                        }
                     }
                  } /* End if strlen > 0 */
               } /* End if DataWords */
               
               memcpy(&TblData.Entry[JsonMessage.Id.Value],&MsgEntry,sizeof(MSGTBL_Entry_t));
               
            } /* End if valid attributes */
            else
            {
               CFE_EVS_SendEvent(MSGTBL_LOAD_ERR_EID, CFE_EVS_ERROR,
                                 "Message[%d] only has %d attributes. stream-id, seq-seg, or length is missing",
                                 MsgArrayIdx, AttributeCnt);
               ReadMsg = false;
               RetStatus = false;
            }
         } /* End if valid ID */
         else
         {
            CFE_EVS_SendEvent(MSGTBL_LOAD_ERR_EID, CFE_EVS_ERROR,
                              "Message[%d] has an invalid ID value of %d. Valid ID range is 0 to %d",
                              MsgArrayIdx, JsonMessage.Id.Value, (MSGTBL_MAX_ENTRIES-1));
         }
         
         MsgArrayIdx++;
         
      } /* End if 'id' */
      else
      {
         ReadMsg = false;
      }
      
   } /* End ReadMsg */
   
   if (MsgArrayIdx == 0)
   {
      CFE_EVS_SendEvent(MSGTBL_LOAD_ERR_EID, CFE_EVS_ERROR,
                        "JSON table file has no message entries");
   }
   else
   {
      if (RetStatus == true)
      {
         memcpy(&MsgTbl->Data,&TblData, sizeof(MSGTBL_Data_t));
         MsgTbl->LastLoadCnt = MsgArrayIdx;
         CFE_EVS_SendEvent(MSGTBL_LOAD_EID, CFE_EVS_INFORMATION,
                           "Message Table load updated %d entries", MsgArrayIdx);
      }
   }
   
   return RetStatus;
   
} /* End LoadJsonData() */


/******************************************************************************
** Function: SplitStr
**
** Split a string based on a delimiter. 
**
** Example Usage
**    char str[] = "A,B,,,C";
**    printf("1 %s\n",zstring_strtok(s,","));
**    printf("2 %s\n",zstring_strtok(NULL,","));
**    printf("3 %s\n",zstring_strtok(NULL,","));
**    printf("4 %s\n",zstring_strtok(NULL,","));
**    printf("5 %s\n",zstring_strtok(NULL,","));
**    printf("6 %s\n",zstring_strtok(NULL,","));
** Example Output
**    1 A
**    2 B
**    3 ,
**    4 ,
**    5 C
**    6 (null)
**
** Notes:
**   1. Plus: No extra memory required
**   2. Minus: Use of static variable is not rentrant so can't use in app_fw
**   3. Minus: Modifies caller's string
*/
static char *SplitStr(char *Str, const char *Delim) {
   
   static char *StaticStr=NULL;      /* Store last address */
   int i=0, StrLength=0;
   boolean DelimFound = FALSE;                  

   /* Valid Delim and have characters left */
   if (Delim == NULL || (Str == NULL && StaticStr == NULL))
      return NULL;

   if (Str == NULL)
      Str = StaticStr;

   StrLength = strlen(&Str[StrLength]);

   /* find the first occurance of delim */
   for (i=0; i < StrLength; i++)
   {
      if (Str[i] == Delim[0])
      {
         DelimFound = TRUE;
         break;
      }
   }
   
   if (!DelimFound) {
      StaticStr = NULL;
      return Str;
   }

   /* Check for consecutive delimiters */
   if (Str[0] == Delim[0])
   {
      StaticStr = (Str + 1);
      return (char *)Delim;
   }

   /* terminate the string
    * this assignment requires char[], so str has to
    * be char[] rather than *char
    */
   Str[i] = '\0';

   /* save the rest of the string */
   if ((Str + i + 1) != 0)
      StaticStr = (Str + i + 1);
   else
      StaticStr = NULL;

   return Str;

} /* End SplitStr() */
