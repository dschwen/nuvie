/*
 *  GUI_Scroller.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sun Apr 18 2004.
 *  Copyright (c) 2004. All rights reserved.
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

#include <cmath>
#include <string>
#include "nuvieDefs.h"
#include "U6misc.h"
#include "GUI.h"
#include "GUI_widget.h"
#include "GUI_ScrollBar.h"
#include "GUI_Scroller.h"

GUI_Scroller::GUI_Scroller(int x, int y, int w, int h, Uint8 r, Uint8 g, Uint8 b, uint16 r_height)
 : GUI_Widget(NULL, x, y, w, h)
{

	R = r;
	G = g;
	B = b;
	bg_color = 0;
    row_height = r_height;
    rows_per_page = h / row_height;
    num_rows = 0;
    disp_offset = 0;
    scroll_bar = new GUI_ScrollBar(area.w - SCROLLBAR_WIDTH, 0, area.h, (GUI_CallBack *)this);

    GUI_Widget::AddWidget(scroll_bar); // we call the GUI_Widget::AddWidget method our method is for scroller container items.
}

/* Map the color to the display */
void GUI_Scroller::SetDisplay(Screen *s)
{
	GUI_Widget::SetDisplay(s);
	bg_color = SDL_MapRGB(surface->format, R, G, B);
}

int GUI_Scroller::AddWidget(GUI_Widget *widget)
{
 GUI_Widget::AddWidget(widget);
 num_rows++;
 update_viewport(true);

 return 0;
}

void GUI_Scroller::PlaceOnScreen(Screen *s, GUI_DragManager *dm, int x, int y)
{
 GUI_Widget::PlaceOnScreen(s,dm,x,y);

 update_viewport(true);
}

void GUI_Scroller::update_viewport(bool update_slider)
{
 uint16 i;
 float s_len = 1.0;
 float s_pos = 0.0;

 if(update_slider)
  {
   if(rows_per_page < num_rows)
     s_len = (float)rows_per_page / (float)num_rows;

   scroll_bar->set_slider_length(s_len);

   if(disp_offset > 0)
     s_pos = (float)disp_offset / (float)num_rows;

   scroll_bar->set_slider_position(s_pos);
  }

  std::list<GUI_Widget *>::iterator child;
  child = children.begin();
  child++; // skip the scroll_bar widget. This is a bit evil.

  for(i=0; child != children.end(); child++, i++)
    {
     if(i < disp_offset || i >= disp_offset + rows_per_page)
        (*child)->Hide();
     else
       {
        (*child)->Move(area.x, area.y + (i - disp_offset) * row_height);
        (*child)->Show();
       }
    }

 return;
}

/* Show the widget  */
void GUI_Scroller:: Display(bool full_redraw)
{
 SDL_Rect framerect;

 framerect = area;
 framerect.w -= SCROLLBAR_WIDTH;

 SDL_FillRect(surface, &framerect, bg_color);

 DisplayChildren();

// screen->update(area.x,area.y,area.w,area.h);

 return;
}

GUI_status GUI_Scroller::MouseDown(int x, int y, int button)
{

	if(button == SDL_BUTTON_WHEELUP)
		move_up();
	else if(button == SDL_BUTTON_WHEELDOWN)
		move_down();
 //grab_focus();

 return GUI_YUM;
}

GUI_status GUI_Scroller::MouseUp(int x, int y, int button)
{

 // release_focus();

 return GUI_YUM;
}

GUI_status GUI_Scroller::MouseMotion(int x,int y,Uint8 state)
{


 //GUI::get_gui()->moveWidget(this,dx,dy);
// Redraw();

 return (GUI_YUM);
}

void GUI_Scroller::move_up()
{
 if(disp_offset > 0)
   {
    disp_offset--;
    update_viewport(true);
   }

}

void GUI_Scroller::move_down()
{
 if(num_rows - disp_offset > rows_per_page)
    {
     disp_offset++;
     update_viewport(true);
    }
}

void GUI_Scroller::page_up(bool all)
{
 if(disp_offset == 0)
   return;
 if(all)
   disp_offset = 0;
 else
 {
   for (int i=0; i < rows_per_page; i++)
   {
     if(disp_offset > 0)
       disp_offset--;
   }
 }
 update_viewport(true);
}

void GUI_Scroller::page_down(bool all)
{
 if(num_rows - disp_offset < rows_per_page)
  return;
 if(all)
   disp_offset = num_rows - rows_per_page;
 else
 {
   for (int i=0; i < rows_per_page; i++)
   {
     if(num_rows - disp_offset > rows_per_page)
       disp_offset++;
   }
 }
 update_viewport(true);
}

void GUI_Scroller::move_percentage(float offset_percentage)
{
 // DEBUG(0,LEVEL_DEBUGGING,"offset_percentage = %f\n", offset_percentage);

 disp_offset = (int)((float)num_rows * offset_percentage);
 update_viewport(false);

}

GUI_status GUI_Scroller::callback(uint16 msg, GUI_CallBack *caller, void *data)
{

 switch(msg)
  {
   case SCROLLBAR_CB_SLIDER_MOVED :
                                    move_percentage(*(float *)data);
                                    break;

   case SCROLLBAR_CB_UP_BUTTON : move_up();
                                 break;

   case SCROLLBAR_CB_DOWN_BUTTON : move_down();
                                 break;
   case SCROLLBAR_CB_PAGE_DOWN: page_down();
                                break;
   case SCROLLBAR_CB_PAGE_UP: page_up();
                              break;

   default : DEBUG(0,LEVEL_ERROR,"Unhandled callback!\n"); break;
  }

 return GUI_YUM;
}
