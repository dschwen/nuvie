/*
 * Adplug - Replayer for many OPL2/OPL3 audio file formats.
 * Copyright (C) 1999 - 2008 Simon Peter, <dn.tlp@gmx.net>, et al.
 * 
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *
 * MIDI & MIDI-like file player - Last Update: 10/15/2005
 *                  by Phil Hassey - www.imitationpickles.org
 *                                   philhassey@hotmail.com
 *
 * Can play the following
 *      .LAA - a raw save of a Lucas Arts Adlib music
 *             or
 *             a raw save of a LucasFilm Adlib music
 *      .MID - a "midi" save of a Lucas Arts Adlib music
 *           - or general MIDI files
 *      .CMF - Creative Music Format
 *      .SCI - the sierra "midi" format.
 *             Files must be in the form
 *             xxxNAME.sci
 *             So that the loader can load the right patch file:
 *             xxxPATCH.003  (patch.003 must be saved from the
 *                            sierra resource from each game.)
 *
 * 6/2/2000:  v1.0 relased by phil hassey
 *      Status:  LAA is almost perfect
 *                      - some volumes are a bit off (intrument too quiet)
 *               MID is fine (who wants to listen to MIDI vid adlib anyway)
 *               CMF is okay (still needs the adlib rythm mode implemented
 *                            for real)
 * 6/6/2000:
 *      Status:  SCI:  there are two SCI formats, orginal and advanced.
 *                    original:  (Found in SCI/EGA Sierra Adventures)
 *                               played almost perfectly, I believe
 *                               there is one mistake in the instrument
 *                               loader that causes some sounds to
 *                               not be quite right.  Most sounds are fine.
 *                    advanced:  (Found in SCI/VGA Sierra Adventures)
 *                               These are multi-track files.  (Thus the
 *                               player had to be modified to work with
 *                               them.)  This works fine.
 *                               There are also multiple tunes in each file.
 *                               I think some of them are supposed to be
 *                               played at the same time, but I'm not sure
 *                               when.
 * 8/16/2000:
 *      Status:  LAA: now EGA and VGA lucas games work pretty well
 *
 * 10/15/2005: Changes by Simon Peter
 *	Added rhythm mode support for CMF format.
 *
 * 09/13/2008: Changes by Adam Nielsen (malvineous@shikadi.net)
 *      Fixed a couple of CMF rhythm mode bugs
 *      Disabled note velocity for CMF files
 *      Added support for nonstandard CMF AM+VIB controller (for VGFM CMFs)
 *
 * Other acknowledgements:
 *  Allegro - for the midi instruments and the midi volume table
 *  SCUMM Revisited - for getting the .LAA / .MIDs out of those
 *                    LucasArts files.
 *  FreeSCI - for some information on the sci music files
 *  SD - the SCI Decoder (to get all .sci out of the Sierra files)
 */

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "nuvieDefs.h"
#include "U6misc.h"
#include "U6Lib_n.h"
#include "Game.h"
#include "OriginFXAdLibDriver.h"
#include "mid.h"

//#define TESTING
#ifdef TESTING
#define midiprintf printf
#else
void CmidPlayer::midiprintf(const char *format, ...)
    {
    }
#endif

#define LUCAS_STYLE   1
#define CMF_STYLE     2
#define MIDI_STYLE    4
#define SIERRA_STYLE  8

// AdLib melodic and rhythm mode defines
#define ADLIB_MELODIC	1
#define ADLIB_RYTHM	0

// File types
#define FILE_LUCAS      1
#define FILE_MIDI       2
#define FILE_CMF        3
#define FILE_SIERRA     4
#define FILE_ADVSIERRA  5
#define FILE_OLDLUCAS   6

CPlayer *CmidPlayer::factory(Copl *newopl)
{
  return new CmidPlayer(newopl);
}

CmidPlayer::CmidPlayer(Copl *newopl)
  : CPlayer(newopl), author(&emptystr), title(&emptystr), remarks(&emptystr),
    emptystr('\0'), flen(0), data(0)
{
	origin_fx_driver = new OriginFXAdLibDriver(Game::get_game()->get_config(), newopl);
}

CmidPlayer::~CmidPlayer()
{
	if(data)
		delete [] data;

	delete origin_fx_driver;
}

unsigned char CmidPlayer::datalook(long pos)
{
    if (pos<0 || pos >= flen) return(0);
    return(data[pos]);
}

unsigned long CmidPlayer::getnexti(unsigned long num)
{
	unsigned long v=0;
	unsigned long i;

    for (i=0; i<num; i++)
        {
        v+=(datalook(pos)<<(8*i)); pos++;
        }
    return(v);
}

unsigned long CmidPlayer::getnext(unsigned long num)
{
	unsigned long v=0;
	unsigned long i;

    for (i=0; i<num; i++)
        {
        v<<=8;
        v+=datalook(pos); pos++;
        }
    return(v);
}

unsigned long CmidPlayer::getval()
{
    int v=0;
	unsigned char b;

    b=(unsigned char)getnext(1);
	v=b&0x7f;
	while ((b&0x80) !=0)
		{
        b=(unsigned char)getnext(1);
        v = (v << 7) + (b & 0x7F);
		}
	return(v);
}

bool CmidPlayer::load(const std::string &filename)
{
	return false;
}

bool CmidPlayer::load(std::string &filename, int song_index)
{
	U6Lib_n f;
	f.open(filename, 4, NUVIE_GAME_MD);
    //binistream *f = fp.open(filename); if(!f) return false;
    int good;

    flen = f.get_item_size(song_index);
    data = new unsigned char [flen];
    f.get_item(song_index, data);
    //f->readString((char *)data, flen);

    //f->readString((char *)s, 6);

    good=0;
    subsongs=0;
    switch(data[0])
        {
        case 'A':
            if (data[1]=='D' && data[2]=='L') good=FILE_LUCAS;
            break;
        case 'M':
            if (data[1]=='T' && data[2]=='h' && data[3]=='d') good=FILE_MIDI;
            break;
        case 'C':
            if (data[1]=='T' && data[2]=='M' && data[3]=='F') good=FILE_CMF;
            break;
	  break;
        default:
            if (data[4]=='A' && data[5]=='D') good=FILE_OLDLUCAS;
            break;
        }

    if (good!=0)
		subsongs=1;
    else {
      delete data;
      data = NULL;
      return false;
    }

    type=good;
    //f->seek(0);

    rewind(0);
    return true;
}

void CmidPlayer::interrupt_vector()
{
	origin_fx_driver->interrupt_vector();
}

bool CmidPlayer::update()
{
	const uint8 adlib_chan_tbl[] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 10,
			10, 18, 11, 0, 12, 13, 17, 13, 16, 13, 14, 13, 13, 15,
			13, 19, 0, 0, 0, 0, 21, 0, 0, 0, 26, 26, 25, 20, 20,
			0, 0, 21, 21, 22, 23, 0, 0, 24, 0, 20, 0 };

	const uint8 adlib_note_tbl[] = {
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
			0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 48,
			48, 48, 48, 0, 48, 42, 71, 42, 71, 47, 71, 47, 52, 79,
			52, 77, 0, 0, 0, 0, 71, 0, 0, 0, 72, 79, 79, 64, 58,
			0, 0, 89, 84, 48, 72, 0, 0, 36, 0, 96, 0 };

    //long w,v,note,vel,ctrl,nv,x,l,lnum;
    long w,v,note,vel,ctrl,x,l,lnum;
    int i=0,j,c;
    //int on,onl,numchan;
    int ret;

    int current_status[16];

    for(i=0;i<16;i++)
    	current_status[i] = 0;

    if (doing == 1)
        {
        // just get the first wait and ignore it :>
        for (curtrack=0; curtrack<16; curtrack++)
            if (track[curtrack].on)
                {
                pos=track[curtrack].pos;
                if (type != FILE_SIERRA && type !=FILE_ADVSIERRA)
                    track[curtrack].iwait+=getval();
                    else
                    track[curtrack].iwait+=getnext(1);
                track[curtrack].pos=pos;
                }
        doing=0;
        }

    iwait=0;
    ret=1;

    while (iwait==0 && ret==1)
        {
        for (curtrack=0; curtrack<16; curtrack++)
        if (track[curtrack].on && track[curtrack].iwait==0 &&
            track[curtrack].pos < track[curtrack].tend)
        {
        pos=track[curtrack].pos;

		v=getnext(1);

        //  This is to do implied MIDI events. aka 'Running Status'
        if (v<0x80)
        {
        	v=track[curtrack].pv;
        	printf ("Running status [%2X]\n",(unsigned int)v);
        	pos--;
        }
        else
        {
        	if(v >= 0xf0 && v < 0xf9)
        	{
        		track[curtrack].pv=0; //reset running status.
        	}
        	else if(v < 0xf0)
        	{
        		track[curtrack].pv=(unsigned char)v;
        	}
        	// if v > 0xf9 then current running status is maintained.
        }
		c=v&0x0f;
        midiprintf ("[%2X]",(unsigned int)v);
        if(v==0xfe)
        	midiprintf("pos=%d",(int)pos);
        current_status[curtrack] = v;
        switch(v&0xf0)
            {
			case 0x80: /*note off*/
				midiprintf("Trk%02d: Note Off\n",curtrack);
				note=getnext(1); vel=getnext(1);
				origin_fx_driver->play_note(c, note, 0);
                break;
            case 0x90: /*note on*/
              //  doing=0;
            	midiprintf("Trk%02d: Note On\n",curtrack);
                note=getnext(1); vel=getnext(1);
                if(c == 9)
                {
                	if(adlib_chan_tbl[note] != 0)
                	{
                		origin_fx_driver->play_note(adlib_chan_tbl[note] - 1, adlib_note_tbl[note], vel);
                	}
                }
                else
                {
                	origin_fx_driver->play_note(c, note, vel);
                }

                break;
            case 0xa0: /*key after touch */
                note=getnext(1); vel=getnext(1);

                break;
            case 0xb0: /*control change .. pitch bend? */
                ctrl=getnext(1); vel=getnext(1);
                origin_fx_driver->control_mode_change(c, ctrl, vel);
                break;
            case 0xc0: /*patch change*/
            	x=getnext(1);
            	origin_fx_driver->program_change(c, x);
            	break;
            case 0xd0: /*chanel touch*/
                x=getnext(1);
                break;
            case 0xe0: /*pitch wheel*/
                x=getnext(1);
                l=getnext(1);
                origin_fx_driver->pitch_bend(c,x,l);
                break;
            case 0xf0:
                switch(v)
                    {
                    case 0xf0:
                    case 0xf7: /*sysex*/
		      l=getval();
		      if (datalook(pos+l)==0xf7)
			i=1;
		      midiprintf("{%d}",(int)l);
		      midiprintf("\n");

                        if (datalook(pos)==0x7d &&
                            datalook(pos+1)==0x10 &&
                            datalook(pos+2)<16)
							{
                            adlib_style=LUCAS_STYLE|MIDI_STYLE;
							for (i=0; i<l; i++)
								{
                                midiprintf ("%x ",datalook(pos+i));
                                if ((i-3)%10 == 0) midiprintf("\n");
								}
                            midiprintf ("\n");
                            getnext(1);
                            getnext(1);
							c=getnext(1);
							getnext(1);

                          //  getnext(22); //temp


                            i=(getnext(1)<<4)+getnext(1);


                            //if ((i&1)==1) ch[c].ins[10]=1;

                            midiprintf ("\n%d: ",c);

                            getnext(l-26);
							}
                            else
                            {
                            midiprintf("\n");
                            for (j=0; j<l; j++)
                                midiprintf ("%2X ",(unsigned int)getnext(1));
                            }

                        midiprintf("\n");
						if(i==1)
							getnext(1);
                        break;
                    case 0xf1:
                        break;
                    case 0xf2:
                        getnext(2);
                        break;
                    case 0xf3:
                        getnext(1);
                        break;
                    case 0xf4:
                        break;
                    case 0xf5:
                        break;
                    case 0xf6: /*something*/
                    case 0xf8:
                    case 0xfa:
                    case 0xfb:
                    case 0xfc:
                        break;
                    case 0xfe:
                    	i=getnext(1);
                    	//printf("FE %02X pos=%d\n",i, (int)pos);//(unsigned int)getnext(1),(unsigned int)getnext(1));
                    	getnext(2);
                    	if(i==0)
                    	{
                    		//printf(" %02X",(unsigned int)getnext(1));
                    		//getnext(1);
                    	}
                    	//printf("\n");
                    	if(i != 3)
                    	{
                    		origin_fx_driver->control_mode_change(c,0x7b,0);
                    	}
                    	break;
                    case 0xfd:
                        break;
                    case 0xff:
                        v=getnext(1);
                        l=getval();
                        midiprintf ("\n");
                        midiprintf("{%X_%X}",(unsigned int)v,(int)l);
                        if (v==0x51)
                            {
                            lnum=getnext(l);
                            msqtr=lnum; /*set tempo*/
                            midiprintf ("Set Tempo (qtr=%ld)",msqtr);
                            }
                        else if (v==0x3)
                            {
                        		midiprintf ("Track Name: ");
                        		for (i=0; i<l; i++)
                        	       midiprintf ("%c",(unsigned char)getnext(1));
                            }
                        else if (v==0x6)
                            {
                        		printf ("Marker: ");
                        		for (i=0; i<l; i++)
                        		{
                        	       //midiprintf ("%c",(unsigned char)getnext(1));
                        	       printf ("%c",(unsigned char)getnext(1));
                        		}
                        		printf("\n");
                            }
                            else
                            {
                            for (i=0; i<l; i++)
                                midiprintf ("%2X ",(unsigned int)getnext(1));
                            }
                        break;
					}
                break;
            default: midiprintf("! v = %d",(int)v); /* if we get down here, a error occurred */
			break;
            }

        if (pos < track[curtrack].tend)
            {
            if (type != FILE_SIERRA && type !=FILE_ADVSIERRA)
                w=getval();
                else
                w=getnext(1);
            track[curtrack].iwait=w;
            /*
            if (w!=0)
                {
                midiprintf("\n<%d>",w);
                f = 
((float)w/(float)deltas)*((float)msqtr/(float)1000000);
                if (doing==1) f=0; //not playing yet. don't wait yet
                }
                */
            }
            else
            track[curtrack].iwait=0;

        track[curtrack].pos=pos;
        }
/*
        for(i=0;i<16;i++)
        {
        	if(current_status[i] == 0)
        		printf("--");
        	else
        		printf("%02X", current_status[i]);
        	printf(" ");
        }
        printf("\n");
*/
        ret=0; //end of song.
        iwait=0;
        for (curtrack=0; curtrack<16; curtrack++)
            if (track[curtrack].on == 1 &&
                track[curtrack].pos < track[curtrack].tend)
                ret=1;  //not yet..

        if (ret==1)
            {
            iwait=0xffffff;  // bigger than any wait can be!
            for (curtrack=0; curtrack<16; curtrack++)
               if (track[curtrack].on == 1 &&
                   track[curtrack].pos < track[curtrack].tend &&
                   track[curtrack].iwait < iwait)
                   iwait=track[curtrack].iwait;
            }
        }


    if (iwait !=0 && ret==1)
        {
        for (curtrack=0; curtrack<16; curtrack++)
            if (track[curtrack].on)
                track[curtrack].iwait-=iwait;

        
fwait=1.0f/(((float)iwait/(float)deltas)*((float)msqtr/(float)1000000));
        }
        else
        fwait=50;  // 1/50th of a second

    midiprintf ("\n");
    for (i=0; i<16; i++)
      if (track[i].on) {
	if (track[i].pos < track[i].tend)
	  ;//midiprintf ("<%d:%d>",(int)i,(int)track[i].iwait);
	else
	  midiprintf("stop");
      }

	if(ret)
		return true;
	else
		return false;
}

float CmidPlayer::getrefresh()
{
    return (fwait > 0.01f ? fwait : 0.01f);
}

void CmidPlayer::rewind(int subsong)
{
    long i;

    pos=0; tins=0;
    adlib_style=MIDI_STYLE|CMF_STYLE;
    adlib_mode=ADLIB_MELODIC;

    /* General init */
    for (i=0; i<9; i++)
        {
        chp[i][0]=-1;
        chp[i][2]=0;
        }

    deltas=250;  // just a number,  not a standard
    msqtr=500000;
    fwait=123; // gotta be a small thing.. sorta like nothing
    iwait=0;

    subsongs=1;

    for (i=0; i<16; i++)
        {
        track[i].tend=0;
        track[i].spos=0;
        track[i].pos=0;
        track[i].iwait=0;
        track[i].on=0;
        track[i].pv=0;
        }
    curtrack=0;

    /* specific to file-type init */

        pos=0;
        i=getnext(1);
        switch(type)
            {
            case FILE_MIDI:
                if (type != FILE_LUCAS)
                    tins=128;
                getnext(9);  /*skip header*/
                track_count = getnext(2); //total number of tracks.
                deltas=getnext(2);
                midiprintf ("deltas:%ld\n",deltas);


                load_ultima_midi_tracks();

                break;
            }


/*        sprintf(info,"%s\r\nTicks/Quarter Note: %ld\r\n",info,deltas);
        sprintf(info,"%sms/Quarter Note: %ld",info,msqtr); */

        for (i=0; i<16; i++)
            if (track[i].on)
                {
                track[i].pos=track[i].spos;
                track[i].pv=0;
                track[i].iwait=0;
                }

    doing=1;
    origin_fx_driver->init();
}

void CmidPlayer::load_ultima_midi_tracks()
{
    for(curtrack=0;curtrack<track_count;curtrack++)
    {
        getnext(4); //skip MTrk

    track[curtrack].on=1;
    track[curtrack].tend=getnext(4);
    track[curtrack].tend+=pos;
    track[curtrack].spos=pos;
    pos=track[curtrack].tend;
    midiprintf ("tracklen:%ld\n",track[curtrack].tend-track[curtrack].spos);
    }
}

std::string CmidPlayer::gettype()
{
	switch(type) {
	case FILE_LUCAS:
		return std::string("LucasArts AdLib MIDI");
	case FILE_MIDI:
		return std::string("General MIDI");
	case FILE_CMF:
		return std::string("Creative Music Format (CMF MIDI)");
	case FILE_OLDLUCAS:
		return std::string("Lucasfilm Adlib MIDI");
	case FILE_ADVSIERRA:
		return std::string("Sierra On-Line VGA MIDI");
	case FILE_SIERRA:
		return std::string("Sierra On-Line EGA MIDI");
	default:
		return std::string("MIDI unknown");
	}
}
