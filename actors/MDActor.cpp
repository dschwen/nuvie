/*
 *  MDActor.cpp
 *  Nuvie
 *
 *  Created by the Nuvie Team on Thur Jul 12 2012.
 *  Copyright (c) 2012. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#include "MDActor.h"
#include "Game.h"
#include "GameClock.h"
#include "U6misc.h"
#include "DirFinder.h"

#define MD_DOWNWARD_FACING_FRAME_N 9

MDActor::MDActor(Map *m, ObjManager *om, GameClock *c): WOUActor(m,om,c)
{
}

MDActor::~MDActor()
{
}

bool MDActor::init()
{
 Actor::init();
 return true;
}

bool MDActor::will_not_talk()
{
	if(worktype == 0xa0)
		return true;
	return false;
}

bool MDActor::check_move(uint16 new_x, uint16 new_y, uint8 new_z, ActorMoveFlags flags)
{
 if(ethereal)
   return true;

 if(Actor::check_move(new_x, new_y, new_z, flags) == false)
    return false;

 if(z == new_z) //FIXME check if new pos is adjacent to current position
 {
   uint8 movement_dir = DirFinder::get_nuvie_dir(x, y, new_x, new_y, z);
//   printf("%d (%d,%d) -> (%d,%d) move = %d %s\n", id_n, x, y, new_x, new_y, movement_dir, get_direction_name(movement_dir));
   return map->is_passable(new_x,new_y,new_z, movement_dir);
 }

 return map->is_passable(new_x,new_y,new_z);
}

uint16 MDActor::get_downward_facing_tile_num()
{
 return get_tile_num(obj_n) + MD_DOWNWARD_FACING_FRAME_N;
}

uint8 MDActor::get_hp_text_color()
{
  if(is_poisoned())
    return 4;

  if(get_status_flag(ACTOR_MD_STATUS_FLAG_COLD))
    return 0xf;

  if(get_hp() <= 10)
    return 0xc;

  if(get_obj_flag(ACTOR_MD_OBJ_FLAG_HYPOXIA))
    return 9;

  if(get_obj_flag(ACTOR_MD_OBJ_FLAG_FRENZY) && id_n != 1)
    return 1;

  return 0;
}

uint8 MDActor::get_str_text_color()
{
  uint8 color = 0;
  if(get_obj_flag(ACTOR_MD_OBJ_FLAG_HYPOXIA))
    color = 9;

  if(id_n <= 0xf && Game::get_game()->get_clock()->get_purple_berry_counter(id_n) > 0)
  {
    color = 0xd;
  }
  else if(get_obj_flag(ACTOR_MD_OBJ_FLAG_FRENZY)) //battle frenzy
  {
    color = 1;
  }

  return color;
}

uint8 MDActor::get_dex_text_color()
{
  uint8 color = 0;
  if(get_obj_flag(ACTOR_MD_OBJ_FLAG_HYPOXIA))
    color = 9;
  else if (get_obj_flag(ACTOR_MD_OBJ_FLAG_FRENZY))
  {
    color = 1;
  }

  return color;
}
