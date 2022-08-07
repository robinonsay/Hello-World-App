/************************************************************************
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
*************************************************************************/

/**
 * @file
 *
 * Define Hello World Message IDs
 *
 * \note The Hello World App assumes default configuration which uses V1 of message id implementation
 */

#ifndef HELLO_WORLD_MSGIDS_H
#define HELLO_WORLD_MSGIDS_H

/* V1 Command Message IDs must be 0x18xx */
#define HELLO_WORLD_CMD_MID     0x18A0
#define HELLO_WORLD_SEND_HK_MID 0x18A1
/* V1 Telemetry Message IDs must be 0x08xx */
#define HELLO_WORLD_HK_TLM_MID  0x08AF
#define HELLO_WORLD_HLO_TLM_MID 0x08AE

#endif /* HELLO_WORLD_MSGIDS_H */
