/* Copyright 2000 Kjetil S. Matheussen

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. */










#include "../common/nsmtracker.h"

#include "midi_i_plugin.h"
#include "midi_i_plugin_proc.h"

#include "../common/visual_proc.h"

#include "midi_menues_proc.h"



extern struct Patch *g_currpatch;

#define APP_GetVars() struct Patch *patch=g_currpatch;




int MIDIResetAllControllers( void )
{
  printf("midiresetallcontrollers called\n");
	/* routine when (sub)item "Reset All Controllers" is selected. */

	APP_GetVars()
	struct PatchData *patchdata;
	struct MidiPort *midi_port;
	int channel;

	if(patch==NULL) return 0;
	patchdata=(struct PatchData *)patch->patchdata;
	midi_port=patchdata->midi_port;

	for(channel=0;channel<16;channel++){
		R_PutMidi3(midi_port,0xb0|channel,121,0);
		midi_port->MSB[channel]=-1;
		midi_port->LSB[channel]=-1;
		midi_port->preset[channel]=-1;
	}

	return 0;
}

int MIDILocalKeyboardOn( void )
{
	/* routine when (sub)item "Local Keyboard On" is selected. */

	APP_GetVars()
	struct PatchData *patchdata;
	struct MidiPort *midi_port;
	int channel;

	if(patch==NULL) return 0;
	patchdata=(struct PatchData *)patch->patchdata;
	midi_port=patchdata->midi_port;

	for(channel=0;channel<16;channel++){
		R_PutMidi3(midi_port,0xb0|channel,122,127);
	}

	return 0;
}

int MIDILocalKeyboardOff( void )
{
	/* routine when (sub)item "Local Keyboard Off" is selected. */

	APP_GetVars()
	struct PatchData *patchdata;
	struct MidiPort *midi_port;
	int channel;

	if(patch==NULL) return 0;
	patchdata=(struct PatchData *)patch->patchdata;
	midi_port=patchdata->midi_port;

	for(channel=0;channel<16;channel++){
		R_PutMidi3(midi_port,0xb0|channel,122,0);
	}

	return 0;
}

int MIDIAllNotesOff( void )
{
	/* routine when (sub)item "All Notes Off" is selected. */

	APP_GetVars()
	struct PatchData *patchdata;
	struct MidiPort *midi_port;
	int channel;

	if(patch==NULL) return 0;
	patchdata=(struct PatchData *)patch->patchdata;
	midi_port=patchdata->midi_port;

	for(channel=0;channel<16;channel++){
		R_PutMidi3(midi_port,0xb0|channel,123,0);
	}

	return 0;
}


int MIDIAllSoundsOff( void )
{
	/* routine when (sub)item "All Sounds Off" is selected. */

	APP_GetVars()
	struct PatchData *patchdata;
	struct MidiPort *midi_port;
	int channel;

	if(patch==NULL) return 0;
	patchdata=(struct PatchData *)patch->patchdata;
	midi_port=patchdata->midi_port;

	for(channel=0;channel<16;channel++){
		R_PutMidi3(midi_port,0xb0|channel,120,0);
	}

	return 0;
}

extern struct MidiLink *inputmidilink;
extern struct MidiNode *midinode;
extern struct Root *root;
extern char *inlinkname;

int MIDISetInputPort( void ){
  RWarning("Midi input is not implemented for alsaseq yet.");
#if 0
	char *clustername;
	ReqType reqtype;
	FILE *file;

	reqtype=GFX_OpenReq(root->song->tracker_windows,70,(int)(10),"Select Input MidiLink");

	clustername=CAMD_getClusterName(reqtype);

	if(clustername==NULL){
		GFX_CloseReq(root->song->tracker_windows,reqtype);
		return 0;
	}

	RemoveMidiLink(inputmidilink);

	sprintf(inlinkname,"%s",clustername);

	inputmidilink=AddMidiLink(midinode,MLTYPE_Receiver,
		MLINK_Name, "sortoftracker.in",
		MLINK_Location,inlinkname,
		MLINK_EventMask,CMF_Channel,
		NULL
	);

	GFX_CloseReq(root->song->tracker_windows,reqtype);

	if(inputmidilink==NULL){
		return 0;
	}

	file=fopen("radium:inlinkname.txt","w");
	if(file!=NULL){
		fprintf(file,"%s",inlinkname);
		fclose(file);
	}
#endif
	return 0;
}

extern bool useOx90ForNoteOff;

int MIDIOx90ForNoteOff( void ){
	useOx90ForNoteOff=useOx90ForNoteOff==false?true:false;
	return 0;
}




