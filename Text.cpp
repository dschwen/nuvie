/*
 *  Text.cpp
 *  Nuvie
 *
 *  Created by Eric Fry on Thu Mar 13 2003.
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

#if 0 /* This class has been depreciated by Font */

#include <stdio.h>

#include <string>
#include <cctype>

#include "nuvieDefs.h"

#include "Configuration.h"
#include "NuvieIOFile.h"

#include "Screen.h"
#include "U6Shape.h"
#include "Text.h"
#include "U6misc.h"

Text::Text(Configuration *cfg)
{
 config = cfg;

 font_data = NULL;
}

Text::~Text()
{
 if(font_data != NULL)
   free(font_data);
}

bool Text::loadFont()
{
 std::string filename;
 NuvieIOFileRead u6_ch;

 config_get_path(config,"u6.ch",filename);

 if(u6_ch.open(filename) == false)
   return false;

 font_data = u6_ch.readAll();
 if(font_data == NULL)
   return false;

 return true;
}
/*
bool Text::drawString(Screen *screen, std::string str, uint16 x, uint16 y, uint8 lang_num)
{
 const char *c_string;
 c_string = str.c_str();
 return drawString(screen, c_string, strlen(c_string), x, y, lang_num);
}
*/
bool Text::drawString(Screen *screen, const char *str, uint16 x, uint16 y, uint8 lang_num)
{
 return drawString(screen, str, strlen(str), x, y, lang_num);
}

bool Text::drawString(Screen *screen, const char *str, uint16 string_len, uint16 x, uint16 y, uint8 lang_num)
{
 uint16 i, l; // l is drawn-index of character, to determine x
 bool highlight = false;

 if(font_data == NULL)
   return false;

 uint8 highlight_index = 4;
 uint8 normal_text_index = 0;

 if(Game::get_game()->get_game_type() == NUVIE_GAME_U6)
 {
   highlight_index = 0x0c;
   normal_text_index = 0x48;
 }
 for(i=0, l=0;i<string_len;i++)
   {
    if(str[i] == '@')
       highlight = true;
    else
      {
       if(!isalpha(str[i]))
          highlight = false;
       drawChar(screen, get_char_num(str[i],lang_num), x + (l++) * 8, y,
                highlight ? highlight_index : normal_text_index);
      }
   }
 highlight = false;
 return true;
}

//draw a string ignoring @ for highlighting using a specified color.
bool Text::drawString(Screen *screen, const char *str, uint16 string_len, uint16 x, uint16 y, uint8 lang_num, uint8 color)
{
 uint16 i;

 if(font_data == NULL)
   return false;

 for(i=0;i<string_len;i++)
   {
    drawChar(screen, get_char_num(str[i],lang_num), x + i * 8, y, color);
   }

 return true;
}

uint8 Text::get_char_num(uint8 c, uint8 lang_num)
{
 if(c < 32 || c > 126) // lock char into ascii chars supported by U6 font.
   c = 32;
 if(lang_num > 0)
   {
    if(c >= 97 && c < 123) // toupper alpha characters
      c -= 32;

    if(lang_num == 1) // Britannian
      c += 128;

    if(lang_num == 2) // Gargoylian
      {
       if(c < 64) // note uses non alpha chars from britannian. :)
          c += 128;
       else
          c += 160;
      }
   }

 return c;
}

void Text::drawChar(Screen *screen, uint8 char_num, uint16 x, uint16 y,
                    uint8 color)
{
 unsigned char buf[64];
 unsigned char *pixels;
 uint16 i,j;
 unsigned char *font;
 uint16 pitch;

 memset(buf,0xff,64);

 //pixels = (unsigned char *)screen->get_pixels();
 pixels = buf;
 pitch = 8;//screen->get_pitch();

 font = &font_data[char_num * 8];

 //pixels += y * pitch + x;

 for(i=0;i<8;i++)
   {
    for(j=8;j>0;j--)
      {
       if(font[i] & (1<<(j-1)))
         pixels[8-j] = color; // 0th palette entry should be black
      }

    pixels += pitch;
   }

 screen->blit(x,y,buf,8,8,8,8,true,NULL);
 return;
}

bool Text::drawStringToShape(U6Shape *shp, const char *str, uint16 x, uint16 y, uint8 color)
{
	uint16 i;
	uint16 string_len = strlen(str);

	if(font_data == NULL)
		return false;

	for(i=0;i<string_len;i++)
	{
		x += drawCharToShape(shp, get_char_num(str[i], 0), x, y, color) + 1;
	}

	return true;
}

uint8 Text::drawCharToShape(U6Shape *shp, uint8 char_num, uint16 x, uint16 y,
                    uint8 color)
{
 unsigned char *pixels;
 uint16 i,j;
 unsigned char *font;
 uint16 pitch;
 uint16 dst_w, dst_h;

 pixels = shp->get_data();
 shp->get_size(&dst_w, &dst_h);
 pitch = dst_w;

 font = &font_data[char_num * 8];

 pixels += y * pitch + x;
 uint8 w=0;
 for(i=0;i<8;i++)
   {

    for(j=8;j>0;j--)
      {
       if(font[i] & (1<<(j-1)))
       {
         pixels[8-j] = color; // 0th palette entry should be black
         if((8-j) > w)
        	 w = 8-j;
       }
      }

    pixels += pitch;
   }

 if(w==0)
	 w = 4;

 return w;
}

#endif  /* if 0 */
