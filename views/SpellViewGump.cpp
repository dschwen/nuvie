/*
 *  SpellViewGump.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Thu Mar 8 2012.
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
#include <cassert>
#include "nuvieDefs.h"
#include "U6misc.h"
#include "Event.h"
#include "GUI.h"
#include "GUI_button.h"
#include "Magic.h"

#include "SpellViewGump.h"
#include "MapWindow.h"
#include "NuvieBmpFile.h"

#define NEWMAGIC_BMP_W 144
#define NEWMAGIC_BMP_H 82

SpellViewGump::SpellViewGump(Configuration *cfg) : SpellView(cfg)
{
	num_spells_per_page = 10;
	bg_image = NULL; gump_button = NULL; font = NULL;
	selected_spell = -1;
}

SpellViewGump::~SpellViewGump()
{
	delete font;
}

bool SpellViewGump::init(Screen *tmp_screen, void *view_manager, uint16 x, uint16 y, Font *f, Party *p, TileManager *tm, ObjManager *om)
{
	View::init(x,y,f,p,tm,om);

	SetRect(area.x, area.y, 162, 108);

	std::string datadir = GUI::get_gui()->get_data_dir();
	std::string imagefile;
	std::string path;

	SDL_Surface *image, *image1;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;

	build_path(datadir, "gump_btn_up.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	build_path(datadir, "gump_btn_down.bmp", imagefile);
	image1 = SDL_LoadBMP(imagefile.c_str());

	gump_button = new GUI_Button(NULL, 0, 9, image, image1, this);
	this->AddWidget(gump_button);

	build_path(datadir, "spellbook", path);
	datadir = path;

	build_path(datadir, "spellbook_left_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str()); //we load this twice as they are freed in ~GUI_Button()
	image1 = SDL_LoadBMP(imagefile.c_str());

	left_button = new GUI_Button(this, 27, 4, image, image1, this);
	this->AddWidget(left_button);

	build_path(datadir, "spellbook_right_arrow.bmp", imagefile);
	image = SDL_LoadBMP(imagefile.c_str());
	image1 = SDL_LoadBMP(imagefile.c_str());

	right_button = new GUI_Button(this, 132, 4, image, image1, this);
	this->AddWidget(right_button);

	font = new GUI_Font(GUI_FONT_GUMP);
	font->SetColoring( 0x7c, 0x00, 0x00, 0xd0, 0x70, 0x00, 0x00, 0x00, 0x00);

	return true;
}


uint8 SpellViewGump::fill_cur_spell_list()
{
	uint8 count = SpellView::fill_cur_spell_list();

	//load spell images
	uint8 i;
	char filename[24]; // spellbook_spell_xxx.bmp\0
	std::string datadir = GUI::get_gui()->get_data_dir();
	std::string path;

	build_path(datadir, "images", path);
	datadir = path;
	build_path(datadir, "gumps", path);
	datadir = path;
	build_path(datadir, "spellbook", path);
	datadir = path;

	std::string imagefile;

	SDL_FreeSurface(bg_image);

	//build_path(datadir, "", spellbookdir);

	build_path(datadir, "spellbook_bg.bmp", imagefile);
	bg_image = bmp.getSdlSurface32(imagefile);
	if(bg_image == NULL)
	{
		DEBUG(0,LEVEL_ERROR,"Failed to load spellbook_bg.bmp from '%s' directory\n", datadir.c_str());
		return count;
	}

	set_bg_color_key(0, 0x70, 0xfc);

	for(i=0;i<count;i++)
	{
		sprintf(filename, "spellbook_spell_%03d.bmp", cur_spells[i]);
		build_path(datadir, filename, imagefile);
		SDL_Surface *spell_image = bmp.getSdlSurface32(imagefile);
		if(spell_image == NULL)
		{
			DEBUG(0,LEVEL_ERROR,"Failed to load %s from '%s' directory\n", filename, datadir.c_str());
		}
		else
		{
			SDL_Rect dst;

			dst.w = 58;
			dst.h = 13;

			uint8 base = (level-1) * 16;

			uint8 spell = cur_spells[i] - base;
			dst.x = ((spell < 5) ? 25 : 88);
			dst.y = 18 + (spell % 5) * 14;
			SDL_BlitSurface(spell_image, NULL, bg_image, &dst);
			SDL_FreeSurface(spell_image);
			printSpellQty(cur_spells[i], dst.x + ((spell < 5) ? 50 : 48), dst.y);
		}
	}

	loadCircleString(datadir);

	return count;
}

void SpellViewGump::loadCircleString(std::string datadir)
{
	std::string imagefile;
	char filename[7]; // n.bmp\0

	sprintf(filename, "%d.bmp", level);
	build_path(datadir, filename, imagefile);

	SDL_Surface *s = bmp.getSdlSurface32(imagefile);
	if(s != NULL)
	{
		SDL_Rect dst;
		dst.x = 70;
		dst.y = 7;
		dst.w = 4;
		dst.h = 6;
		SDL_BlitSurface(s, NULL, bg_image, &dst);
	}

	switch(level)
	{
		case 1 : loadCircleSuffix(datadir, "st.bmp"); break;
		case 2 : loadCircleSuffix(datadir, "nd.bmp"); break;
		case 3 : loadCircleSuffix(datadir, "rd.bmp"); break;
		default: break;
	}
}

void SpellViewGump::loadCircleSuffix(std::string datadir, std::string image)
{
	std::string imagefile;

	build_path(datadir, image, imagefile);
	SDL_Surface *s = bmp.getSdlSurface32(imagefile);
	if(s != NULL)
	{
		SDL_Rect dst;
		dst.x = 75;
		dst.y = 7;
		dst.w = 7;
		dst.h = 6;
		SDL_BlitSurface(s, NULL, bg_image, &dst);
	}
}


void SpellViewGump::printSpellQty(uint8 spell_num, uint16 x, uint16 y)
{
	Magic *m = Game::get_game()->get_magic();
	char num_str[4];

	Spell *spell = m->get_spell((uint8)spell_num);

	uint16 qty = get_available_spell_count(spell);
	snprintf(num_str, 3, "%d", qty);

	if(qty < 10)
		x += 5;

	font->TextOut(bg_image, x, y, num_str);
}

void SpellViewGump::Display(bool full_redraw)
{
 //display_level_text();
 //display_spell_list_text();
 SDL_Rect dst;
 dst = area;
 dst.w = 162;
 dst.h = 108;
 SDL_BlitSurface(bg_image, NULL, surface, &dst);

 DisplayChildren(full_redraw);

 sint16 spell = get_selected_spell();

 if(spell < 0)
	 spell = 0;

 spell = spell % 16;
 screen->fill(248, area.x + ((spell < 5) ? 75 : 136), area.y + 18 + 7 + (spell % 5) * 14, 10, 1);

 update_display = false;
 screen->update(area.x, area.y, area.w, area.h);

 return;
}

GUI_status SpellViewGump::callback(uint16 msg, GUI_CallBack *caller, void *data)
{
	//close gump and return control to Magic class for clean up.
	if(caller == gump_button)
	{
		if(Game::get_game()->get_event()->is_looking_at_spellbook())
			close_look();
		else
			close_spellbook();
		return GUI_YUM;
	}
	else if(caller == left_button)
	{
		move_left();
		return GUI_YUM;
	}
	else if(caller == right_button)
	{
		move_right();
		return GUI_YUM;
	}

    return GUI_PASS;
}

void SpellViewGump::close_spellbook()
{
	Game::get_game()->get_event()->close_spellbook();
}

sint16 SpellViewGump::getSpell(int x, int y)
{
	int localy = y - area.y;
	int localx = x - area.x;

	localy += 3; //align the pointer in the center of the crosshair cursor.
	localx += 3;

	if(localy < 21 || localy > 88 || localx < 28 || localx > 148)
	{
		return -1;
	}

	uint8 spell = (level - 1) * 16;


	if(localx >= 89)
		spell += 5;

	spell += (localy - 20) / 14;

	for(uint8 i=0;cur_spells[i] != -1 && i < 16;i++)
	{
		if(cur_spells[i] == spell)
		{
			return spell;
		}
	}

	return -1;
}

GUI_status SpellViewGump::MouseDown(int x, int y, int button)
{
	if(button == SDL_BUTTON_WHEELDOWN)
	{
		move_right();
		return GUI_YUM;
	}
	else if(button == SDL_BUTTON_WHEELUP)
	{
		move_left();
		return GUI_YUM;
	}
	else if(SDL_BUTTON(button) & SDL_BUTTON_RMASK)
	{
		close_spellbook();
		return GUI_YUM;
	}

	sint16 clicked_spell = getSpell(x, y);
	
	if(clicked_spell != -1)
	{
		selected_spell = clicked_spell;
		return GUI_YUM;
	}

	bool can_target = true; // maybe put this check into GUI_widget
	if(HitRect(x, y))
	{
		if(bg_image)
		{
			Uint32 pixel = sdl_getpixel(bg_image, x - area.x, y - area.y);
			if(pixel != bg_color_key)
				can_target = false;
		}
		else
			can_target = false;
	}

	if(can_target)
	{
		Event *event = Game::get_game()->get_event();
		event->target_spell(); //Simulate a global key down event.
		if(event->get_mode() == INPUT_MODE)
			Game::get_game()->get_map_window()->select_target(x, y);
		if(event->get_mode() != MOVE_MODE)
			close_spellbook();
		return GUI_YUM;
	}

	return DraggableView::MouseDown(x, y, button);
}

GUI_status SpellViewGump::MouseUp(int x, int y, int button)
{
	sint16 spell = getSpell(x, y);

	if(spell != -1 && spell == selected_spell)
	{
		spell_container->quality = spell;
		if(Game::get_game()->get_event()->is_looking_at_spellbook())
			show_spell_description();
		else if(event_mode)
		{
			event_mode_select_spell();
		}
		else
		{
			//Simulate a global key down event.
			Game::get_game()->get_event()->target_spell();
		}

		return GUI_YUM;
	}


	return DraggableView::MouseUp(x, y, button);
}
