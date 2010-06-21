#ifndef PROTOCOL_H
#define PROTOCOL_H

/*  Protocol Defines, typedefs and Enums
 *
 *  Copyright (C) 2008  Lee Begg and the Thousand Parsec Project
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <stdint.h>

enum ProtocolVersion{
  fv0_1 = 1,
  fv0_2 = 2,
  fv0_3 = 3,
  fv0_4 = 4,
  fv_Max,
};

enum FrameType{
  // New Frame Codes
  ft_Invalid = -1,
  ft02_Invalid = -1,
  ft02_OK = 0,
  ft02_Fail = 1,
  ft02_Sequence = 2,
  ft02_Connect = 3,
  ft02_Login = 4,
  ft02_Object_GetById = 5,
  ft02_Object_GetByPos = 6,
  ft02_Object = 7,
  ft02_OrderDesc_Get = 8,
  ft02_OrderDesc = 9,
  ft02_Order_Get = 10,
  ft02_Order = 11,
  ft02_Order_Insert = 12,
  ft02_Order_Remove = 13,
  ft02_Time_Remaining_Get = 14,
  ft02_Time_Remaining = 15,
  ft02_Board_Get = 16,
  ft02_Board = 17,
  ft02_Message_Get = 18,
  ft02_Message = 19,
  ft02_Message_Post = 20,
  ft02_Message_Remove = 21,
  ft02_ResDesc_Get = 22,
  ft02_ResDesc = 23,
  ft02_Max = 24,
  //Version 3 additions follow
  ft03_Redirect = 24,
  ft03_Features_Get = 25,
  ft03_Features = 26,
  ft03_Ping = 27,
  ft03_ObjectIds_Get = 28,
  ft03_ObjectIds_GetByPos = 29,
  ft03_ObjectIds_GetByContainer = 30,
  ft03_ObjectIds_List = 31,
  ft03_OrderTypes_Get = 32,
  ft03_OrderTypes_List = 33,
  ft03_Order_Probe = 34,
  ft03_BoardIds_Get = 35,
  ft03_BoardIds_List = 36,
  ft03_ResType_Get = 37,
  ft03_ResType_List = 38,
  ft03_Player_Get = 39,
  ft03_Player = 40,
  //Design category and component
  ft03_Category_Get = 41,
  ft03_Category = 42,
  ft03_Category_Add = 43,
  ft03_Category_Remove = 44,
  ft03_CategoryIds_Get = 45,
  ft03_CategoryIds_List = 46,
  ft03_Design_Get = 47,
  ft03_Design = 48,
  ft03_Design_Add = 49,
  ft03_Design_Modify = 50,
  ft03_Design_Remove = 51,
  ft03_DesignIds_Get = 52,
  ft03_DesignIds_List = 53,
  ft03_Component_Get = 54,
  ft03_Component = 55,
  ft03_ComponentIds_Get = 56,
  ft03_ComponentIds_List = 57,
  ft03_Property_Get = 58,
  ft03_Property = 59,
  ft03_PropertyIds_Get = 60,
  ft03_PropertyIds_List = 61,
  ft03_Account = 62,
  ft03_Max = 63,
  // Version 4 frame types follow
  ft04_TurnFinished = 63,
  ft04_Filters_Set = 64,
  ft04_GameInfo_Get = 65,
  ft04_GameInfo = 66,
  ft04_ObjectDesc_Get = 67,
  ft04_ObjectDesc = 68,
  ft04_ObjectTypes_Get = 69,
  ft04_ObjectTypes_List = 70,
  ft04_Object_Modify = 71,
  ft04_PlayerIds_Get = 72,
  ft04_PlayerIds_List = 73,
  ft04_Max,
  // Administration frame types follow
  ftad_Min = 1000,
  ftad_LogMessage = 1000,
  ftad_CommandUpdate = 1001,
  ftad_CommandDesc_Get = 1002,
  ftad_CommandDesc = 1003,
  ftad_CommandTypes_Get = 1004,
  ftad_CommandTypes_List = 1005,
  ftad_Command = 1006,
  ftad_CommandResult = 1007,
  ftad_Max,


};

enum FrameErrorCode{
  fec_Invalid = -1,
  fec_ProtocolError = 0,
  fec_FrameError = 1,
  fec_PermUnavailable = 2,
  fec_TempUnavailable = 3,
  fec_NonExistant = 4,
  fec_PermissionDenied = 5,
  fec_Max
};


// some helpful defines

#define UINT64_NEG_ONE 0xffffffffffffffffULL
#define UINT32_NEG_ONE 0xffffffff
#define MAX_ID_LIST_SIZE 87378

//feature ids
enum FeatureIDs {
  fid_sec_conn_this = 1,
  fid_sec_conn_other = 2,
  fid_http_this = 3,
  fid_http_other = 4,
  fid_keep_alive = 5,
  fid_serverside_property = 6,
  fid_account_register = 1000,
  fid_filter_tls = 0x1000,
  fid_filter_stringpad = 0x1D00
};

//typedefs
typedef uint32_t objectid_t;
typedef uint32_t objecttypeid_t;
typedef uint32_t orderqueueid_t;
typedef uint32_t ordertypeid_t;
typedef uint32_t orderslot_t;
typedef uint32_t boardid_t;
typedef uint32_t messageslot_t;
typedef uint32_t resourcetypeid_t;
typedef uint32_t playerid_t;
typedef uint32_t categoryid_t;
typedef uint32_t designid_t;
typedef uint32_t componentid_t;
typedef uint32_t propertyid_t;

typedef uint32_t commandtypeid_t;

typedef int32_t reftype_t;
typedef uint32_t refvalue_t;

#endif
