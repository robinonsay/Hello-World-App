/*******************************************************************************
**
**      GSC-18128-1, "Core Flight Executive Version 6.7"
**
**      Copyright (c) 2006-2019 United States Government as represented by
**      the Administrator of the National Aeronautics and Space Administration.
**      All Rights Reserved.
**
**      Licensed under the Apache License, Version 2.0 (the "License");
**      you may not use this file except in compliance with the License.
**      You may obtain a copy of the License at
**
**        http://www.apache.org/licenses/LICENSE-2.0
**
**      Unless required by applicable law or agreed to in writing, software
**      distributed under the License is distributed on an "AS IS" BASIS,
**      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
**      See the License for the specific language governing permissions and
**      limitations under the License.
**
** File: hello_world.c
**
** Purpose:
**   This file contains the source code for the Hello World.
**
*******************************************************************************/

/*
** Include Files:
*/
#include "hello_world_events.h"
#include "hello_world_version.h"
#include "hello_world.h"
#include "hello_world_table.h"

/* The sample_lib module provides the SAMPLE_LIB_Function() prototype */
#include <string.h>

/*
** global data
*/
HELLO_WORLD_Data_t HELLO_WORLD_Data;

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
/* HELLO_WORLD_Main() -- Application entry point and main process loop         */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * *  * * * * **/
void HELLO_WORLD_Main(void)
{
    int32            status;
    CFE_SB_Buffer_t *SBBufPtr;

    /*
    ** Create the first Performance Log entry
    */
    CFE_ES_PerfLogEntry(HELLO_WORLD_PERF_ID);

    /*
    ** Perform application specific initialization
    ** If the Initialization fails, set the RunStatus to
    ** CFE_ES_RunStatus_APP_ERROR and the App will not enter the RunLoop
    */
    status = HELLO_WORLD_Init();
    if (status != CFE_SUCCESS)
    {
        HELLO_WORLD_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
    }

    /*
    ** SAMPLE Runloop
    */
    while (CFE_ES_RunLoop(&HELLO_WORLD_Data.RunStatus) == true)
    {
        /*
        ** Performance Log Exit Stamp
        */
        CFE_ES_PerfLogExit(HELLO_WORLD_PERF_ID);

        /* Pend on receipt of command packet */
        status = CFE_SB_ReceiveBuffer(&SBBufPtr, HELLO_WORLD_Data.CommandPipe, CFE_SB_PEND_FOREVER);

        /*
        ** Performance Log Entry Stamp
        */
        CFE_ES_PerfLogEntry(HELLO_WORLD_PERF_ID);

        if (status == CFE_SUCCESS)
        {
            HELLO_WORLD_ProcessCommandPacket(SBBufPtr);
        }
        else
        {
            CFE_EVS_SendEvent(HELLO_WORLD_PIPE_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE APP: SB Pipe Read Error, App Will Exit");

            HELLO_WORLD_Data.RunStatus = CFE_ES_RunStatus_APP_ERROR;
        }
    }

    /*
    ** Performance Log Exit Stamp
    */
    CFE_ES_PerfLogExit(HELLO_WORLD_PERF_ID);

    CFE_ES_ExitApp(HELLO_WORLD_Data.RunStatus);

} /* End of HELLO_WORLD_Main() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  */
/*                                                                            */
/* HELLO_WORLD_Init() --  initialization                                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 HELLO_WORLD_Init(void)
{
    int32 status;

    HELLO_WORLD_Data.RunStatus = CFE_ES_RunStatus_APP_RUN;

    /*
    ** Initialize app command execution counters
    */
    HELLO_WORLD_Data.CmdCounter = 0;
    HELLO_WORLD_Data.ErrCounter = 0;

    /*
    ** Initialize app configuration data
    */
    HELLO_WORLD_Data.PipeDepth = HELLO_WORLD_PIPE_DEPTH;

    strncpy(HELLO_WORLD_Data.PipeName, "HELLO_WORLD_CMD_PIPE", sizeof(HELLO_WORLD_Data.PipeName));
    HELLO_WORLD_Data.PipeName[sizeof(HELLO_WORLD_Data.PipeName) - 1] = 0;

    /*
    ** Initialize event filter table...
    */
    HELLO_WORLD_Data.EventFilters[0].EventID = HELLO_WORLD_STARTUP_INF_EID;
    HELLO_WORLD_Data.EventFilters[0].Mask    = 0x0000;
    HELLO_WORLD_Data.EventFilters[1].EventID = HELLO_WORLD_COMMAND_ERR_EID;
    HELLO_WORLD_Data.EventFilters[1].Mask    = 0x0000;
    HELLO_WORLD_Data.EventFilters[2].EventID = HELLO_WORLD_COMMANDNOP_INF_EID;
    HELLO_WORLD_Data.EventFilters[2].Mask    = 0x0000;
    HELLO_WORLD_Data.EventFilters[3].EventID = HELLO_WORLD_COMMANDRST_INF_EID;
    HELLO_WORLD_Data.EventFilters[3].Mask    = 0x0000;
    HELLO_WORLD_Data.EventFilters[4].EventID = HELLO_WORLD_INVALID_MSGID_ERR_EID;
    HELLO_WORLD_Data.EventFilters[4].Mask    = 0x0000;
    HELLO_WORLD_Data.EventFilters[5].EventID = HELLO_WORLD_LEN_ERR_EID;
    HELLO_WORLD_Data.EventFilters[5].Mask    = 0x0000;
    HELLO_WORLD_Data.EventFilters[6].EventID = HELLO_WORLD_PIPE_ERR_EID;
    HELLO_WORLD_Data.EventFilters[6].Mask    = 0x0000;

    /*
    ** Register the events
    */
    status = CFE_EVS_Register(HELLO_WORLD_Data.EventFilters, HELLO_WORLD_EVENT_COUNTS, CFE_EVS_EventFilter_BINARY);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Registering Events, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Initialize housekeeping packet (clear user data area).
    */
    CFE_MSG_Init(&HELLO_WORLD_Data.HkTlm.TlmHeader.Msg, CFE_SB_ValueToMsgId(HELLO_WORLD_HK_TLM_MID),
                 sizeof(HELLO_WORLD_Data.HkTlm));

    /*
    ** Create Software Bus message pipe.
    */
    status = CFE_SB_CreatePipe(&HELLO_WORLD_Data.CommandPipe, HELLO_WORLD_Data.PipeDepth, HELLO_WORLD_Data.PipeName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error creating pipe, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to Housekeeping request commands
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(HELLO_WORLD_SEND_HK_MID), HELLO_WORLD_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Subscribing to HK request, RC = 0x%08lX\n", (unsigned long)status);
        return (status);
    }

    /*
    ** Subscribe to ground command packets
    */
    status = CFE_SB_Subscribe(CFE_SB_ValueToMsgId(HELLO_WORLD_CMD_MID), HELLO_WORLD_Data.CommandPipe);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Subscribing to Command, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }

    /*
    ** Register Table(s)
    */
    status = CFE_TBL_Register(&HELLO_WORLD_Data.TblHandles[0], "SampleAppTable", sizeof(HELLO_WORLD_Table_t),
                              CFE_TBL_OPT_DEFAULT, HELLO_WORLD_TblValidationFunc);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Registering Table, RC = 0x%08lX\n", (unsigned long)status);

        return (status);
    }
    else
    {
        status = CFE_TBL_Load(HELLO_WORLD_Data.TblHandles[0], CFE_TBL_SRC_FILE, HELLO_WORLD_TABLE_FILE);
    }

    CFE_EVS_SendEvent(HELLO_WORLD_STARTUP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE App Initialized.%s",
                      HELLO_WORLD_VERSION_STRING);

    return (CFE_SUCCESS);

} /* End of HELLO_WORLD_Init() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  HELLO_WORLD_ProcessCommandPacket                                    */
/*                                                                            */
/*  Purpose:                                                                  */
/*     This routine will process any packet that is received on the SAMPLE    */
/*     command pipe.                                                          */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
void HELLO_WORLD_ProcessCommandPacket(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_SB_MsgId_t MsgId = CFE_SB_INVALID_MSG_ID;

    CFE_MSG_GetMsgId(&SBBufPtr->Msg, &MsgId);

    switch (CFE_SB_MsgIdToValue(MsgId))
    {
        case HELLO_WORLD_CMD_MID:
            HELLO_WORLD_ProcessGroundCommand(SBBufPtr);
            break;

        case HELLO_WORLD_SEND_HK_MID:
            HELLO_WORLD_ReportHousekeeping((CFE_MSG_CommandHeader_t *)SBBufPtr);
            break;

        default:
            CFE_EVS_SendEvent(HELLO_WORLD_INVALID_MSGID_ERR_EID, CFE_EVS_EventType_ERROR,
                              "SAMPLE: invalid command packet,MID = 0x%x", (unsigned int)CFE_SB_MsgIdToValue(MsgId));
            break;
    }

    return;

} /* End HELLO_WORLD_ProcessCommandPacket */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* HELLO_WORLD_ProcessGroundCommand() -- SAMPLE ground commands                */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
void HELLO_WORLD_ProcessGroundCommand(CFE_SB_Buffer_t *SBBufPtr)
{
    CFE_MSG_FcnCode_t CommandCode = 0;

    CFE_MSG_GetFcnCode(&SBBufPtr->Msg, &CommandCode);

    /*
    ** Process "known" SAMPLE app ground commands
    */
    switch (CommandCode)
    {
        case HELLO_WORLD_NOOP_CC:
            if (HELLO_WORLD_VerifyCmdLength(&SBBufPtr->Msg, sizeof(HELLO_WORLD_NoopCmd_t)))
            {
                HELLO_WORLD_Noop((HELLO_WORLD_NoopCmd_t *)SBBufPtr);
            }

            break;

        case HELLO_WORLD_RESET_COUNTERS_CC:
            if (HELLO_WORLD_VerifyCmdLength(&SBBufPtr->Msg, sizeof(HELLO_WORLD_ResetCountersCmd_t)))
            {
                HELLO_WORLD_ResetCounters((HELLO_WORLD_ResetCountersCmd_t *)SBBufPtr);
            }

            break;

        case HELLO_WORLD_PROCESS_CC:
            if (HELLO_WORLD_VerifyCmdLength(&SBBufPtr->Msg, sizeof(HELLO_WORLD_ProcessCmd_t)))
            {
                HELLO_WORLD_Process((HELLO_WORLD_ProcessCmd_t *)SBBufPtr);
            }

            break;

        /* default case already found during FC vs length test */
        default:
            CFE_EVS_SendEvent(HELLO_WORLD_COMMAND_ERR_EID, CFE_EVS_EventType_ERROR,
                              "Invalid ground command code: CC = %d", CommandCode);
            break;
    }

    return;

} /* End of HELLO_WORLD_ProcessGroundCommand() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  HELLO_WORLD_ReportHousekeeping                                          */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function is triggered in response to a task telemetry request */
/*         from the housekeeping task. This function will gather the Apps     */
/*         telemetry, packetize it and send it to the housekeeping task via   */
/*         the software bus                                                   */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 HELLO_WORLD_ReportHousekeeping(const CFE_MSG_CommandHeader_t *Msg)
{
    int i;

    /*
    ** Get command execution counters...
    */
    HELLO_WORLD_Data.HkTlm.Payload.CommandErrorCounter = HELLO_WORLD_Data.ErrCounter;
    HELLO_WORLD_Data.HkTlm.Payload.CommandCounter      = HELLO_WORLD_Data.CmdCounter;

    /*
    ** Send housekeeping telemetry packet...
    */
    CFE_SB_TimeStampMsg(&HELLO_WORLD_Data.HkTlm.TlmHeader.Msg);
    CFE_SB_TransmitMsg(&HELLO_WORLD_Data.HkTlm.TlmHeader.Msg, true);

    /*
    ** Manage any pending table loads, validations, etc.
    */
    for (i = 0; i < HELLO_WORLD_NUMBER_OF_TABLES; i++)
    {
        CFE_TBL_Manage(HELLO_WORLD_Data.TblHandles[i]);
    }

    return CFE_SUCCESS;

} /* End of HELLO_WORLD_ReportHousekeeping() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* HELLO_WORLD_Noop -- SAMPLE NOOP commands                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
int32 HELLO_WORLD_Noop(const HELLO_WORLD_NoopCmd_t *Msg)
{

    HELLO_WORLD_Data.CmdCounter++;

    CFE_EVS_SendEvent(HELLO_WORLD_COMMANDNOP_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: NOOP command %s",
                      HELLO_WORLD_VERSION);

    return CFE_SUCCESS;

} /* End of HELLO_WORLD_Noop */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  HELLO_WORLD_ResetCounters                                               */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function resets all the global counter variables that are     */
/*         part of the task telemetry.                                        */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 HELLO_WORLD_ResetCounters(const HELLO_WORLD_ResetCountersCmd_t *Msg)
{

    HELLO_WORLD_Data.CmdCounter = 0;
    HELLO_WORLD_Data.ErrCounter = 0;

    CFE_EVS_SendEvent(HELLO_WORLD_COMMANDRST_INF_EID, CFE_EVS_EventType_INFORMATION, "SAMPLE: RESET command");

    return CFE_SUCCESS;

} /* End of HELLO_WORLD_ResetCounters() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*  Name:  HELLO_WORLD_Process                                                     */
/*                                                                            */
/*  Purpose:                                                                  */
/*         This function Process Ground Station Command                       */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * *  * *  * * * * */
int32 HELLO_WORLD_Process(const HELLO_WORLD_ProcessCmd_t *Msg)
{
    int32               status;
    HELLO_WORLD_Table_t *TblPtr;
    const char *        TableName = "HELLO_WORLD.SampleAppTable";

    /* Sample Use of Table */

    status = CFE_TBL_GetAddress((void *)&TblPtr, HELLO_WORLD_Data.TblHandles[0]);

    if (status < CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Fail to get table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    CFE_ES_WriteToSysLog("Sample App: Table Value 1: %d  Value 2: %d", TblPtr->Int1, TblPtr->Int2);

    HELLO_WORLD_GetCrc(TableName);

    status = CFE_TBL_ReleaseAddress(HELLO_WORLD_Data.TblHandles[0]);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Fail to release table address: 0x%08lx", (unsigned long)status);
        return status;
    }

    /* Invoke a function provided by HELLO_WORLD_LIB */
    SAMPLE_LIB_Function();

    return CFE_SUCCESS;

} /* End of HELLO_WORLD_ProcessCC */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
/*                                                                            */
/* HELLO_WORLD_VerifyCmdLength() -- Verify command packet length                   */
/*                                                                            */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * **/
bool HELLO_WORLD_VerifyCmdLength(CFE_MSG_Message_t *MsgPtr, size_t ExpectedLength)
{
    bool              result       = true;
    size_t            ActualLength = 0;
    CFE_SB_MsgId_t    MsgId        = CFE_SB_INVALID_MSG_ID;
    CFE_MSG_FcnCode_t FcnCode      = 0;

    CFE_MSG_GetSize(MsgPtr, &ActualLength);

    /*
    ** Verify the command packet length.
    */
    if (ExpectedLength != ActualLength)
    {
        CFE_MSG_GetMsgId(MsgPtr, &MsgId);
        CFE_MSG_GetFcnCode(MsgPtr, &FcnCode);

        CFE_EVS_SendEvent(HELLO_WORLD_LEN_ERR_EID, CFE_EVS_EventType_ERROR,
                          "Invalid Msg length: ID = 0x%X,  CC = %u, Len = %u, Expected = %u",
                          (unsigned int)CFE_SB_MsgIdToValue(MsgId), (unsigned int)FcnCode, (unsigned int)ActualLength,
                          (unsigned int)ExpectedLength);

        result = false;

        HELLO_WORLD_Data.ErrCounter++;
    }

    return (result);

} /* End of HELLO_WORLD_VerifyCmdLength() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* HELLO_WORLD_TblValidationFunc -- Verify contents of First Table      */
/* buffer contents                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
int32 HELLO_WORLD_TblValidationFunc(void *TblData)
{
    int32               ReturnCode = CFE_SUCCESS;
    HELLO_WORLD_Table_t *TblDataPtr = (HELLO_WORLD_Table_t *)TblData;

    /*
    ** Sample Table Validation
    */
    if (TblDataPtr->Int1 > HELLO_WORLD_TBL_ELEMENT_1_MAX)
    {
        /* First element is out of range, return an appropriate error code */
        ReturnCode = HELLO_WORLD_TABLE_OUT_OF_RANGE_ERR_CODE;
    }

    return ReturnCode;

} /* End of HELLO_WORLD_TBLValidationFunc() */

/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
/*                                                                 */
/* HELLO_WORLD_GetCrc -- Output CRC                                     */
/*                                                                 */
/*                                                                 */
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */
void HELLO_WORLD_GetCrc(const char *TableName)
{
    int32          status;
    uint32         Crc;
    CFE_TBL_Info_t TblInfoPtr;

    status = CFE_TBL_GetInfo(&TblInfoPtr, TableName);
    if (status != CFE_SUCCESS)
    {
        CFE_ES_WriteToSysLog("Sample App: Error Getting Table Info");
    }
    else
    {
        Crc = TblInfoPtr.Crc;
        CFE_ES_WriteToSysLog("Sample App: CRC: 0x%08lX\n\n", (unsigned long)Crc);
    }

    return;

} /* End of HELLO_WORLD_GetCrc */
