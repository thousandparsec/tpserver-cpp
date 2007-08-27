#ifndef FRAME_H
#define FRAME_H
/*  TP protocol Frame class
 *
 *  Copyright (C) 2004-2005, 2007  Lee Begg and the Thousand Parsec Project
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
#include <string>

typedef enum {
  fv0_1 = 1,
  fv0_2 = 2,
  fv0_3 = 3,
  fv0_4 = 4,
  fv_Max,
} FrameVersion;

typedef enum {
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
  ft04_Max,


} FrameType;

typedef enum {
  fec_Invalid = -1,
  fec_ProtocolError = 0,
  fec_FrameError = 1,
  fec_PermUnavailable = 2,
  fec_TempUnavailable = 3,
  fec_NonExistant = 4,
  fec_PermissionDenied = 5,
  fec_Max
} FrameErrorCode;


//class std::string;

class Frame {

  public:
    Frame();
    Frame(FrameVersion v);
    Frame(const Frame &rhs);
  
    ~Frame();
  
    Frame operator=(const Frame &rhs);
  
    int setHeader(char *newhead);
    char *getPacket() const;
    int getHeaderLength() const;		// The length of the header section
    int getDataLength() const;		// The length of the data section
    int getLength() const;			// The total length of the packet
    
    // Data
    char *getData() const;
    bool setData(char *newdata, int dlen);
  
    // Type
    FrameType getType() const;
    bool setType(FrameType nt);
    
    // frame type version
    uint32_t getTypeVersion() const;
    bool setTypeVersion(uint32_t tv);
    
    // Sequence
    int getSequence() const;
    bool setSequence(int s);
    
    // Version
    FrameVersion getVersion() const;
    
    //string padding
    bool isPaddingStrings() const;
    void enablePaddingStrings(bool on);
    
    bool packString(const char *str);
    bool packString(const std::string &str);
    bool packInt(int val);
    bool packInt64(long long val);
    bool packInt8(char val);
    bool packData(unsigned int len, char* bdata);
  
    bool isEnoughRemaining(uint32_t size) const;
    // uses these functions with care
    uint32_t getUnpackOffset() const;
    bool setUnpackOffset(uint32_t newoffset);
  
    int unpackInt();
    std::string unpackStdString();
    long long unpackInt64();
    char unpackInt8();
    void unpackData(unsigned int len, char* bdata);
  
    void createFailFrame(FrameErrorCode code, const char *reason);
  
  private:
    FrameVersion version;
    FrameType type;
    uint32_t typeversion;

    // Which packet sequence does this refer to?
    uint32_t sequence;
    
    // Frame length
    uint32_t length;

    // Actual data of the frame
    char *data;
    
    bool padstrings;

    uint32_t unpackptr;
};

#endif
