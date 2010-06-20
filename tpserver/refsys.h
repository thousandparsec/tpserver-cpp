#ifndef REFSYS_H
#define REFSYS_H

/*  Generic Reference System Enums
 *
 *  Copyright (C) 2005  Lee Begg and the Thousand Parsec Project
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

enum RefSysType{
  rst_Action_Server = -1000,
  rst_Action_Player = -4,
  rst_Action_Message = -3,
  rst_Action_Order = -2,
  rst_Action_Object = -1,
  rst_Special = 0,
  rst_Object = 1,
  rst_OrderType = 2,
  rst_Order = 3,
  rst_Board = 4,
  rst_Message = 5,
  rst_ResouceType = 6,
  rst_Player = 7,
  rst_Category = 8,
  rst_Design = 9,
  rst_Component = 10,
  rst_Property = 11,
  rst_ObjectType = 12,
  rst_OrderQueue = 13,
};

enum RefSysSpecialValue{
  rssv_System = 1,
  rssv_Admin = 2,
  rssv_Important = 3,
  rssv_Unimportant = 4
};

enum RefSysPlayerAValue{
  rspav_Eliminated = 1,
  rspav_Quit = 2,
  rspav_Joined = 3
};

enum RefSysOrderAValue{
  rsorav_Completion = 1,
  rsorav_Canceled = 2,
  rsorav_Incompatible = 3,
  rsorav_Invalid = 4
};

enum RefSysObjectAValue{
  rsobav_Idle = 1
};

#endif
