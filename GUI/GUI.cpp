/*
    GUILIB:  An example GUI framework library for use with SDL
    Copyright (C) 1997  Sam Lantinga

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

    Sam Lantinga
    5635-34 Springhouse Dr.
    Pleasanton, CA 94588 (USA)
    slouken@devolution.com
*/

/* This is a C++ class for handling a GUI, and associated widgets */

#include <stdlib.h>

#include "GUI.h"

/* Number of widget elements to allocate at once */
#define WIDGET_ARRAYCHUNK	32


GUI:: GUI(Screen *s)
{
	screen = s;
	numwidgets = 0;
	maxwidgets = 0;
	widgets = NULL;
	display = 1;

  screen_scale_factor = screen->get_scale_factor();
  dragging = false;
  
  gui_font = new GUI_Font();
  gui_drag_manager = new GUI_DragManager(screen);
}

GUI:: ~GUI()
{
	if ( widgets != NULL ) {
		for ( int i=0; i<numwidgets; ++i ) {
			delete widgets[i];
		}
		free(widgets);
	}

 delete gui_font;
 delete gui_drag_manager;
}

/* Add a widget to the GUI.
   The widget will be automatically deleted when the GUI is deleted.
 */
int
GUI:: AddWidget(GUI_Widget *widget)
{
	int i;

	/* Look for deleted widgets */
	for ( i=0; i<numwidgets; ++i ) {
		if ( widgets[i]->Status() == WIDGET_DELETED ) {
			delete widgets[i];
			break;
		}
	}
	if ( i == numwidgets ) {
		/* Expand the widgets array if necessary */
		if ( numwidgets == maxwidgets ) {
			GUI_Widget **newarray;
			int maxarray;

			maxarray = maxwidgets + WIDGET_ARRAYCHUNK;
			if ( (newarray=(GUI_Widget **)realloc(widgets,
					maxarray*sizeof(*newarray))) == NULL ) {
				return(-1);
			}
			widgets = newarray;
			maxwidgets = maxarray;
		}
		++numwidgets;
	}
	widgets[i] = widget;
  widget->PlaceOnScreen(screen,gui_drag_manager,0,0);

	return(0);
}

/* remove widget from gui system but don't delete it */
bool GUI::removeWidget(GUI_Widget *widget)
{
 int i;
 
  for(i=0; i<numwidgets; ++i)
    {
     if(widgets[i] == widget)
       {
        for(int j=i+1; j<numwidgets; ++j ) //shuffle remaining widgets down.
          {
           widgets[j-1] = widgets[j];
          }

        --numwidgets;
        Display(GUI_FULL_REDRAW);
        return true;
			 }
    }
 
 return false;
}

void
GUI:: Display(bool full_redraw)
{
	int i;

	for ( i=0; i<numwidgets; ++i ) {
		if ( widgets[i]->Status() == WIDGET_VISIBLE ) {
			widgets[i]->Display(full_redraw);
      //screen->update(widgets[i]->area.x,widgets[i]->area.y,widgets[i]->area.w,widgets[i]->area.h);
		}
	}
	//SDL_UpdateRect(screen, 0, 0, 0, 0);
}

/* Function to handle a GUI status */
void
GUI:: HandleStatus(GUI_status status)
{
	switch (status) {
		case GUI_QUIT: running = 0;
			             break;
		case GUI_REDRAW: display = 1;
			               break;
    case GUI_DRAG_AND_DROP: dragging = true;
                            break;
		default:
			break;
	}
}

/* Handle an event, passing it to widgets until they return a status */
GUI_status 
GUI:: HandleEvent(SDL_Event *event)
{
	int i;
  int hit;
	GUI_status status = GUI_PASS;

  if(screen_scale_factor != 1)
    {
     if(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP)
        {
         event->button.x /= screen_scale_factor;
         event->button.y /= screen_scale_factor;
        }
     if(event->type == SDL_MOUSEMOTION)
        {
         event->motion.x /= screen_scale_factor;
         event->motion.y /= screen_scale_factor;
         event->motion.xrel /= screen_scale_factor;
         event->motion.yrel /= screen_scale_factor;
        }
    }

  if(dragging)
   {
    if(event->type == SDL_MOUSEBUTTONUP) //FIX for button up that doesn't hit a widget.
      {
       for (hit=false,i=numwidgets-1; (i>=0)&&(hit == false); --i)
        {
				 if(widgets[i]->Status() == WIDGET_VISIBLE && widgets[i]->HitRect(event->button.x,event->button.y))
            {
             gui_drag_manager->drop((GUI_DragArea *)widgets[i],event->button.x,event->button.y);
             dragging = false;
					   break;
            }
				}
			}
   }
  else
   {
	  switch (event->type) {
 		/* SDL_QUIT events quit the GUI */ 
     case SDL_QUIT:
		 	 status = GUI_QUIT;
		 	 break;

		 /* Keyboard and mouse events go to widgets */

		 case SDL_MOUSEMOTION:
		 case SDL_MOUSEBUTTONDOWN:
		 case SDL_MOUSEBUTTONUP:
     case SDL_KEYDOWN:
		 case SDL_KEYUP:
			 /* Go through widgets, topmost first */
			 status = GUI_PASS;
			 for (i=numwidgets-1; (i>=0)&&(status==GUI_PASS); --i) {
				 if ( widgets[i]->Status() == WIDGET_VISIBLE ) {
				 	 status = widgets[i]->HandleEvent(event);
				 }
			 }
			 break;

		 /* Ignore unhandled events */
		 default:
			 status = GUI_PASS;
			 break;
    }
	 }

	HandleStatus(status);

  /* Garbage collection */
  for(i=0; i<numwidgets; ++i)
    {
     if(widgets[i]->Status() == WIDGET_DELETED)
       {
        delete widgets[i];

        for(int j=i+1; j<numwidgets; ++j ) //shuffle remaining widgets down.
          {
           widgets[j-1] = widgets[j];
          }

        --numwidgets;
        if(status != GUI_QUIT) //no point redrawing if we're going to quit.
          Display(GUI_FULL_REDRAW);
			 }
    } 
 return status;
}

/* Run the GUI.
   This returns when either a widget requests a quit, the idle
   function requests a quit, or the SDL window has been closed.
 */
void
GUI:: Run(GUI_IdleProc idle, int once, int multitaskfriendly)
{
	int i;
	SDL_Event event;

	/* If there's nothing to do, return immediately */
	if ( (numwidgets == 0) && (idle == NULL) ) {
		return;
	}

	running = 1;
	if ( ! once ) {
		display = 1;
	}
	do {
		/* Garbage collection */
		for ( i=0; i<numwidgets; ++i ) {
			if ( widgets[i]->Status() == WIDGET_DELETED ) {
				delete widgets[i];
				for ( int j=i+1; j<numwidgets; ++j ) {
					widgets[j-1] = widgets[j];
				}
				--numwidgets;
			}
		}

		/* Display widgets if necessary */
		if ( display ) {
			Display();
			display = 0;
		}

///////////////////////////////////////////////////////////////// Polling is time consuming - instead:
		if (multitaskfriendly && (idle==NULL))
		{
		  SDL_WaitEvent(&event);
		  HandleEvent(&event);
		}
		else
/////////////////////////////////////////////////////////////////
		/* Handle events, or run idle functions */
		if ( SDL_PollEvent(&event) )
		{
			/* Handle all pending events */
			do {
				HandleEvent(&event);
			} while ( SDL_PollEvent(&event) );
		}
		else
		{
			if ( idle != NULL )
			{
				HandleStatus(idle());
			}
			for ( i=numwidgets-1; i>=0; --i ) {
				HandleStatus(widgets[i]->Idle());
			}
		}
		//ERIC SDL_Delay(10);
	} while ( running && ! once );
}

GUI_Font *GUI::get_font()
{
 return gui_font;
}