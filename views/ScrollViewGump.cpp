/*
 *  ScrollViewGump.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Fri Nov 08 2013.
 *  Copyright (c) 2013. All rights reserved.
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
#include "nuvieDefs.h"
#include "U6misc.h"
#include "Event.h"
#include "GUI.h"
#include "FontManager.h"
#include "ViewManager.h"
#include "ScrollWidgetGump.h"
#include "ScrollViewGump.h"

#define SIGN_BG_W (SCROLLWIDGETGUMP_W + 16)
#define SIGN_BG_H (SCROLLWIDGETGUMP_H + 16)

ScrollViewGump::ScrollViewGump(Configuration *cfg) : DraggableView(cfg)
{
  scroll_widget = NULL;
}

ScrollViewGump::~ScrollViewGump()
{

}

bool ScrollViewGump::init(Screen *tmp_screen, void *view_manager, Font *f, Party *p, TileManager *tm, ObjManager *om, std::string text_string)
{
	uint16 x_off = Game::get_game()->get_game_x_offset();
	uint16 y_off = Game::get_game()->get_game_y_offset();

	x_off += (Game::get_game()->get_game_width() - SIGN_BG_W)/2;
	y_off += (Game::get_game()->get_game_height() - SIGN_BG_H)/2;

	View::init(x_off,y_off,f,p,tm,om);
	SetRect(area.x, area.y, SIGN_BG_W, SIGN_BG_H);
/*
	std::string datadir = GUI::get_gui()->get_data_dir();
	std::string imagefile;
	std::string path;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;
	build_path(datadir, "sign", path);
	datadir = path;

	build_path(datadir, "sign_bg.bmp", imagefile);
	bg_image = SDL_LoadBMP(imagefile.c_str());

	set_bg_color_key(0, 0x70, 0xfc);
*/
  scroll_widget = new ScrollWidgetGump(config, tmp_screen);
  scroll_widget->init(config, Game::get_game()->get_font_manager()->get_conv_font());

  scroll_widget->display_string(text_string);

  AddWidget(scroll_widget);

	return true;
}


void ScrollViewGump::Display(bool full_redraw)
{
 /*
 SDL_Rect dst;
 dst = area;
 SDL_BlitSurface(bg_image, NULL, surface, &dst);
 */
 screen->fill(26, area.x, area.y, area.w, area.h);
 DisplayChildren(full_redraw);

 //font->TextOut(screen->get_sdl_surface(), area.x + 29, area.y + 6, "This is a test sign");

 //font->drawString(screen, sign_text, strlen(sign_text), area.x + (area.w - font->getStringWidth(sign_text)) / 2, area.y + (area.h - 19) / 2, 0, 0);
 update_display = false;
 screen->update(area.x, area.y, area.w, area.h);


 return;
}

GUI_status ScrollViewGump::MouseDown(int x, int y, int button)
{
  if(scroll_widget->MouseDown(x, y, button) != GUI_YUM)
  {
    Game::get_game()->get_view_manager()->close_gump(this);
  }
	return GUI_YUM;
}

GUI_status ScrollViewGump::KeyDown(SDL_keysym key)
{
  if(scroll_widget->KeyDown(key) != GUI_YUM)
  {
    Game::get_game()->get_view_manager()->close_gump(this);
  }

	return GUI_YUM;
}

GUI_status ScrollViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data)
{
		Game::get_game()->get_view_manager()->close_gump(this);
		return GUI_YUM;
}
