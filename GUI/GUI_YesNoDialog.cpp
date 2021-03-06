/*
 *  GUI_YesNoDialog.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Sat Feb 07 2004.
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

#include "SDL.h"
#include "nuvieDefs.h"

#include "GUI.h"
#include "GUI_button.h"
#include "GUI_text.h"

#include "GUI_Dialog.h"
#include "GUI_YesNoDialog.h"
#include "Keys.h"


GUI_YesNoDialog::GUI_YesNoDialog(GUI *gui, int x, int y, int w, int h, const char *msg, GUI_CallBack *yesCallback, GUI_CallBack *noCallback) :
   GUI_Dialog(x, y, w, h, 244, 216, 131, GUI_DIALOG_MOVABLE)
{
  GUI_Widget *widget;
  b_index_num = -1;

  yes_callback_object = yesCallback;
  no_callback_object = noCallback;

  yes_button =  new GUI_Button(this, 100, 50, 40, 18, "Yes", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, (GUI_CallBack *)this, 0);
  AddWidget(yes_button);
  button_index[0] = yes_button;

  no_button =  new GUI_Button(this, 30, 50, 40, 18, "No", gui->get_font(), BUTTON_TEXTALIGN_CENTER, 0, (GUI_CallBack *)this, 0);
  AddWidget(no_button);
  button_index[1] = no_button;

  widget = (GUI_Widget *) new GUI_Text(10, 25, 0, 0, 0, msg, gui->get_font());
  AddWidget(widget);
}


GUI_YesNoDialog::~GUI_YesNoDialog()
{
}

GUI_status GUI_YesNoDialog::KeyDown(SDL_keysym key)
{
 if(key.sym == SDLK_y)
   return yes_callback_object->callback(YESNODIALOG_CB_YES, this, this);

 KeyBinder *keybinder = Game::get_game()->get_keybinder();
 ActionType a = keybinder->get_ActionType(key);

 switch(keybinder->GetActionKeyType(a))
 {
	case EAST_KEY:
	case WEST_KEY:
		if(b_index_num != -1)
			button_index[b_index_num]->set_highlighted(false);

		if(b_index_num == 0)
			b_index_num = 1;
		else
			b_index_num = 0;
		button_index[b_index_num]->set_highlighted(true);
		return GUI_YUM;
	case DO_ACTION_KEY: if(b_index_num != -1) return button_index[b_index_num]->Activate_button(); break;
	default: break;
 }

 return no_callback_object->callback(YESNODIALOG_CB_NO, this, this);
}

GUI_status GUI_YesNoDialog::callback(uint16 msg, GUI_CallBack *caller, void *data)
{
 if(caller == (GUI_CallBack *)yes_button)
   return yes_callback_object->callback(YESNODIALOG_CB_YES, this, this);

 if(caller == (GUI_CallBack *)no_button)
   return no_callback_object->callback(YESNODIALOG_CB_NO, this, this);

 return GUI_PASS;
}
