/*
 *  ActorManager.cpp
 *  Nuive
 *
 *  Created by Eric Fry on Thu Mar 20 2003.
 *  Copyright (c) 2003. All rights reserved.
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

#include "ActorManager.h"

static uint8 walk_frame_tbl[4] = {0,1,2,1};

ActorManager::ActorManager(Configuration *cfg, Map *m, TileManager *tm, ObjManager *om)
{
 config = cfg;
 map = m;
 tile_manager = tm;
 obj_manager = om;
}

ActorManager::~ActorManager()
{
 uint16 i;
 
 for(i = 0;i < 256;i++) //we assume actors[] have been created by a call to loadActors()
   delete actors[i];

}
 
bool ActorManager::loadActors()
{
 uint16 i;
 uint8 b1, b2, b3;
 std::string filename;
 U6File objlist;
 U6File schedule;
 
 config->pathFromValue("config/ultima6/gamedir","savegame/objlist",filename);
 if(objlist.open(filename,"rb") == false)
   return false;


 objlist.seek(0x100); // Start of Actor position info
 
 for(i=0; i < 256; i++)
   {
    actors[i] = new Actor(map);
    
    b1 = objlist.read1();
    b2 = objlist.read1();
    b3 = objlist.read1();
    
    actors[i]->x = b1;
    actors[i]->x += (b2 & 0x3) << 8; 
   
    actors[i]->y = (b2 & 0xfc) >> 2;
    actors[i]->y += (b3 & 0xf) << 6;
   
    actors[i]->z = (b3 & 0xf0) >> 4;
    actors[i]->id_n = i;
   }

 for(i=0;i < 256; i++)
   {
    b1 = objlist.read1();
    b2 = objlist.read1();
    actors[i]->a_num = b1;
    actors[i]->a_num += (b2 & 0x3) << 8;
    
    actors[i]->frame_n = (b2 & 0xfc) >> 2;
   }

 loadActorSchedules();
 
 return true;
}
 
Actor *ActorManager::get_actor(uint8 actor_num)
{
 return actors[actor_num];
}

Actor *ActorManager::get_partyLeader()
{
 return actors[1]; //FIX here for dead party leader etc.
}

void ActorManager::updateActors()
{
 uint16 i;
 
 for(i=0;i<256;i++)
  actors[i]->update();
  
}
 
void ActorManager::drawActors(Screen *screen, uint16 x, uint16 y, uint16 width, uint16 height, uint8 level)
{
 uint16 i;
 Tile *tile;
 
 for(i=0;i < 256;i++)
   {
    if(actors[i]->x >= x && actors[i]->x < x + width)
      {
       if(actors[i]->y >= y && actors[i]->y < y + height)
         {
          if(actors[i]->z == level)
           {
            if(i == 1) //HACK fix this for proper frame handling
              tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(actors[i]->a_num)+(actors[i]->direction*4)+walk_frame_tbl[actors[i]->walk_frame]);
            else
              tile = tile_manager->get_tile(obj_manager->get_obj_tile_num(actors[i]->a_num)+actors[i]->frame_n);

            screen->blit(tile->data,8,(actors[i]->x - x)*16,(actors[i]->y - y)*16,16,16,tile->transparent);
           }
         }
      }
   }

 return;
}

bool ActorManager::loadActorSchedules()
{
 std::string filename;
 U6File schedule;
 uint16 i;
 uint16 index[256];
 uint16 s_num;
 uint32 bytes_read;
 unsigned char *sched_data;
 unsigned char *s_ptr;
 
 config->pathFromValue("config/ultima6/gamedir","schedule",filename);
 if(schedule.open(filename,"rb") == false)
   return false;
 
 for(i=0;i<256;i++)
   {
    index[i] = schedule.read2();
   }
 
 sched_data = schedule.readBuf(schedule.filesize() - 0x202, &bytes_read);
 
 s_ptr = sched_data + 2;

 for(i=0;i<256;i++)
  {
   if(i == 255) //Hmm a bit of a hack. might want to check if there are and scheduled events for Actor 255
     s_num = 0;
   else  
     s_num = index[i+1] - index[i];

   actors[i]->loadSchedule(s_ptr,s_num);
   s_ptr += s_num * 5;
  }
 
 free(sched_data);
 
 return true;
}