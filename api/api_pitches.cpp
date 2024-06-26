#include "Python.h"

#include "../common/nsmtracker.h"
#include "../common/TimeData.hpp"
#include "../common/list_proc.h"
#include "../common/placement_proc.h"
#include "../common/pitches_proc.h"
#include "../common/nodelines_proc.h"
#include "../common/undo.h"
#include "../common/undo_notes_proc.h"
#include "../common/notes_proc.h"

#include "api_mouse_proc.h"

#include "api_common_proc.h"
#include "api_support_proc.h"
#include "radium_proc.h"



// pitches
//////////////////////////////////////////////////

float getPitchValue(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  const r::Pitch pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (note==NULL)
    return 0;
  
  return pitch._val;
}

Place getPitchPlace(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  const r::Pitch pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (note==NULL)
    return p_Create(0,0,1);

  return ratio2place(pitch._time);
}

int getPitchLogtype(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  const r::Pitch pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (note==NULL)
    return 0;

  return pitch._logtype;
}

float getPitchChance(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = NULL;

  const r::Pitch pitch = getPitchFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote, &note, pitchnum);

  if (note==NULL)
    return 0;

  return (double)pitch._chance / 256.0;
}

int getNumPitches(dyn_t dynnote, int tracknum, int blocknum, int windownum){
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return 0;

  return 2+r::PitchTimeData::Reader(note->_pitches).size(); //ListFindNumElements3((struct ListHeader3*)note->pitches);
}

int addPitch(float value, Place place, dyn_t dynnote, int tracknum, int blocknum, int windownum){

  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return -1;

  if (PlaceLessOrEqual(&place, &note->l.p)) {
    //if (dynnote>0)
    handleError("addPitch: placement before note start for note. pitch: %s. note: %s", p_ToString(place), p_ToString(note->l.p));
    return -1;
  }

  if (place2ratio(place) > note->end) {
    handleError("addPitch: placement after note end for note. pitch: %s. note end: %s", p_ToString(place), p_ToString(ratio2place(note->end)));
    return -1;
  }

  int pos = AddPitch(window, wblock, wtrack, note, &place, value);
  if (pos < 0) {
    handleError("addPitch: Can not create new pitch with the same position as another pitch");
    return -1;
  }

  window->must_redraw_editor = true;

  return pos + 1;
}

int addPitchF(float value, float floatplace, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  if (floatplace < 0){
    handleError("Place can not be negative: %f", floatplace);
    return -1;
  }

  Place place;
  Float2Placement(floatplace, &place);
  return addPitch(value, place, dynnote, tracknum, blocknum, windownum);
}

dyn_t setPitch(float value, Place place, int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  
  struct Tracker_Windows *window;
  struct WBlocks *wblock;
  struct WTracks *wtrack;
  struct Notes *note = getNoteFromNumA(windownum, &window, blocknum, &wblock, tracknum, &wtrack, dynnote);
  if (note==NULL)
    return dynnote;

  struct Blocks *block = wblock->block;
  struct Tracks *track = wtrack->track;

  if (value < 0.001){
    handleError("setPitch: Pitch less than 0.001: %f\n", value);
    return dynnote;
  }

  if (value > 127)
    value = 127;

  if (!p_is_same_place(place))
    if(place.line < 0){
      handleError("Negative place");
      return dynnote;
    }
      
  int num_pitches = r::PitchTimeData::Reader(note->_pitches).size() + 2;
    
  if (pitchnum < 0 || pitchnum >= num_pitches) {
    handleError("There is no pitch #%d in note %d in track #%d in block #%d",pitchnum, (int)note->id, tracknum, blocknum);
    return dynnote;
  }

  window->must_redraw_editor = true;

  //printf("pitchnum==%d. floatplace: %f\n",pitchnum,floatplace);

  if (pitchnum==0) {
    
    note->note = value;
    if (!p_is_same_place(place))
      return MoveNote(block, track, note, &place, true);
    
  } else if (pitchnum==num_pitches-1) {
    
    note->pitch_end = value;
    if (!p_is_same_place(place))
      MoveEndNote(block, track, note, &place, true);
    
  } else {

    r::PitchTimeData::Writer writer(note->_pitches);

    writer.at_ref(pitchnum-1)._val = value;

    Ratio ratio = place2ratio(place);
      
    if (ratio < 0) {
      
      handleError("Position before start of block");
      
    } else if (ratio > wblock->block->num_lines) {
      
      handleError("Position after end of block");
      
    } else {
        
      writer.constraint_move(pitchnum-1,
                             R_BOUNDARIES(place2ratio(note->l.p), ratio, note->end),
                             wblock->block->num_lines
                             );
      
    }
    
  }

  return dynnote;
}

dyn_t setPitchF(float value, float floatplace, int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  Place place;
  
  if (floatplace < 0) {

    R_ASSERT_NON_RELEASE(false); // Don't know if this is a legal situation.
    place = g_same_place;
    
  }else {
    
    Float2Placement(floatplace, &place);
    
  }
  
  return setPitch(value, place, pitchnum, dynnote, tracknum, blocknum, windownum);
}


void deletePitch(int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  deletePianonote(pitchnum, dynnote, tracknum, blocknum, windownum); // Think this is correct.
}

void setPitchLogtype(int logtype, int pitchnum, dyn_t dynnote, int tracknum, int blocknum, int windownum){
  setPianonoteLogtype(logtype, pitchnum, dynnote, tracknum, blocknum, windownum); // Think this is correct
}




