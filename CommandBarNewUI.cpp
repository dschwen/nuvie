/*
 *  CommandBarNewUI.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Oct 7 2012.
 *  Copyright (c) 2012 The Nuvie Team. All rights reserved.
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
#include <cassert>
#include "SDL.h"

#include "Configuration.h"

#include "nuvieDefs.h"
#include "U6misc.h"
#include "Game.h"
#include "Screen.h"
#include "Event.h"
#include "TileManager.h"
#include "ConvFont.h"
#include "GameClock.h"
#include "GamePalette.h"
#include "CommandBarNewUI.h"
#include "Weather.h"
#include "Party.h"
#include "Player.h"
#include "Objlist.h"
#include "NuvieIO.h"


using std::string;

#define btn_size 17
#define SELECTED_COLOR 248

CommandBarNewUI::CommandBarNewUI(Game *g) : CommandBar()
{
    game = g;
    Weather *weather;
    Configuration *config = g->get_config();

    uint16 x_off = config_get_video_x_offset(config);
    uint16 y_off = config_get_video_y_offset(config);

    icon_w = 5;
    icon_h = 2;
    num_icons = 10;

	offset = OBJLIST_OFFSET_U6_COMMAND_BAR;
	Init(NULL, 120+x_off, 74+y_off, 0, 0);
	area.w = btn_size * icon_w; // space for 5x2 icons
	area.h = btn_size * icon_h + 20; //

    event = NULL; // it's not set yet
    
    weather = game->get_weather();
    
    selected_action = -1;
    combat_mode = false;
    wind = "?";

    bg_color = game->get_palette()->get_bg_color();

    if(game->get_game_type() == NUVIE_GAME_U6)
        init_buttons();

    weather->add_wind_change_notification_callback((CallBack *)this); //we want to know when the wind direction changes.

    cur_pos = 0;

    font = new ConvFont();
    font->init(NULL, 256, 0);
}

CommandBarNewUI::~CommandBarNewUI()
{
}



GUI_status CommandBarNewUI::MouseDown(int x, int y, int button)
{
    x -= area.x;
    y -= area.y;

    if((game->get_game_type() == NUVIE_GAME_U6 && y >= 8 && y <= 24)
        || game->get_game_type() != NUVIE_GAME_U6)
    {
        uint8 activate = x / 16; // icon selected
        if(game->get_game_type() == NUVIE_GAME_SE)
              activate = x/18;
        else if(game->get_game_type() == NUVIE_GAME_MD)
              activate = (x-1)/18;
        if(button == COMMANDBAR_USE_BUTTON)
            return(hit(activate));
        else if(button == COMMANDBAR_ACTION_BUTTON)
        {
            select_action(activate);
        }
    }
    return(GUI_YUM);
}

GUI_status CommandBarNewUI::KeyDown(SDL_keysym key)
{
    switch(key.sym)
    {
        case SDLK_UP:
        case SDLK_KP8:
            if(cur_pos - icon_w < 0)
            	cur_pos = icon_w * icon_h - (icon_w - cur_pos%icon_w);
            else
            	cur_pos -= icon_w;
            break;
        case SDLK_DOWN:
        case SDLK_KP2:
            cur_pos = (cur_pos + icon_w) % (icon_w * icon_h);
            break;
        case SDLK_LEFT:
        case SDLK_KP4:
        	if(cur_pos%icon_w == 0)
        		cur_pos = (cur_pos/icon_w)*icon_w+icon_w-1;
        	else
        		cur_pos--;
            break;
        case SDLK_RIGHT:
        case SDLK_KP6:
            cur_pos = (cur_pos/icon_w)*icon_w + (cur_pos+1) % icon_w;
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        	if(cur_pos < num_icons)
        	{
				hit((sint8)cur_pos);
				Hide();
				return GUI_YUM;
        	}
        	break;
        case SDLK_ESCAPE:
        	Hide();
        	break;
        default :
        	return GUI_PASS;
    }

    return GUI_YUM;
}

void CommandBarNewUI::Display(bool full_redraw)
{
    Screen *screen = game->get_screen();

    //if(full_redraw || update_display)
   // {
        update_display = false;
      if(game->get_game_type() == NUVIE_GAME_U6)
      {
        //screen->fill(bg_color, area.x, area.y, area.w, area.h);

        //display_information();
    	  string infostring(game->get_clock()->get_date_string());
    	  infostring += " Wind:";
    	  infostring += wind;
    	  font->drawString(screen, infostring.c_str(), area.x, area.y);
    	uint8 i=0;
        for(uint8 y=0; y < icon_h; y++)
        {
            for(uint8 x = 0; x < icon_w && i < num_icons; x++,i++)
            {
            	screen->blit(area.x+x*btn_size, 10+area.y+y*btn_size, icon[i]->data, 8, 16, 16, 16);
            	if(i == cur_pos)
            	{
            		screen->stipple_8bit(SELECTED_COLOR, area.x+x*btn_size, 10+area.y+y*btn_size, 16, 16);
            	}
            }
        }


      }
      font->drawString(screen, get_command_name(cur_pos), area.x, area.y + 10 + icon_h * btn_size);
        screen->update(area.x, area.y, area.w, area.h);
  //  }
}

static const char *U6_mode_name_tbl[10] = {"Attack", "Cast", "Talk", "Look", "Get", "Drop", "Move", "Use", "Rest", "Combat mode"};

const char *CommandBarNewUI::get_command_name(sint8 command_num)
{
	if(command_num < 0 || command_num > num_icons)
		return "";

	return U6_mode_name_tbl[command_num];
}

/*
void CommandBarNewUI::display_information()
{
    string infostring(game->get_clock()->get_date_string());
    infostring += " Wind:";
    infostring += wind;
    text->drawString(screen, infostring.c_str(), area.x + 8, area.y, 0);
}
*/
