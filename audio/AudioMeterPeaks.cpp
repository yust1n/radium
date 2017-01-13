
#include <atomic>

#include "../common/nsmtracker.h"
#include "../common/instruments_proc.h"

#include "../audio/SoundPlugin.h"


#include "AudioMeterPeaks_proc.h"





static void call_very_often(AudioMeterPeaks &peaks, bool reset_falloff, float ms){

  for(int ch=0;ch<peaks.num_channels;ch++){

    float max_gain;

    // Atomically read and write RT_max_gains[ch]
    //
    for(int i = 0; i < 64; i++){ // Give up after 64 tries. If the RT thread is very busy, I guess it could happen that we stall here. (also note that RT_max_gains[ch] is not resetted when we fail, so we don't destroy max peak detection, which is most important)
      max_gain = peaks.RT_max_gains[ch];
      if(atomic_compare_and_set_float(&peaks.RT_max_gains[ch], max_gain, 0.0f)==true)
        break;
    }

    float max_db = gain2db(max_gain);

    // max db
    //
    peaks.max_dbs[ch] = max_db;

    // decaying db
    //
    if (max_db > peaks.decaying_dbs[ch])
      peaks.decaying_dbs[ch] = max_db;
    else
      peaks.decaying_dbs[ch] -= ms / 50; // TODO/FIX.

    // falloff db
    if (reset_falloff || max_db > peaks.falloff_dbs[ch])
      peaks.falloff_dbs[ch] = max_db;
  }
}

static const int g_falloff_reset = 5; // 5 seconds between each falloff reset.

static void call_very_often(SoundPlugin *plugin, int ms){
  static int counter = 0;

  counter+=ms;

  bool reset_falloff = false;

  if (counter>=1000*g_falloff_reset){
    counter = 0;
    reset_falloff = true;
  }

  call_very_often(plugin->volume_peaks, reset_falloff, ms);

  call_very_often(plugin->output_volume_peaks, reset_falloff, ms);

  call_very_often(plugin->input_volume_peaks, reset_falloff, ms);
  
  call_very_often(plugin->bus0_volume_peaks, reset_falloff, ms);
  call_very_often(plugin->bus1_volume_peaks, reset_falloff, ms);
  call_very_often(plugin->bus2_volume_peaks, reset_falloff, ms);
  call_very_often(plugin->bus3_volume_peaks, reset_falloff, ms);
  call_very_often(plugin->bus4_volume_peaks, reset_falloff, ms);

}

void AUDIOMETERPEAKS_call_very_often(int ms){
  
  VECTOR_FOR_EACH(struct Patch *, patch, &(get_audio_instrument()->patches)){
    SoundPlugin *plugin = (SoundPlugin*)patch->patchdata;
    if(plugin != NULL){
      call_very_often(plugin, ms);
    }
  }END_VECTOR_FOR_EACH;
}

AudioMeterPeaks AUDIOMETERPEAKS_create(int num_channels){
  AudioMeterPeaks peaks = {0};

  peaks.num_channels = num_channels;

  peaks.RT_max_gains = (float*)V_calloc(num_channels, sizeof(float));
  peaks.max_dbs = (float*)V_calloc(num_channels, sizeof(float));
  peaks.decaying_dbs = (float*)V_calloc(num_channels, sizeof(float));
  peaks.falloff_dbs = (float*)V_calloc(num_channels, sizeof(float));  

  for(int ch=0;ch<num_channels;ch++){
    peaks.max_dbs[ch] = -100;
    peaks.decaying_dbs[ch] = -100;
    peaks.falloff_dbs[ch] = -100;
  }

  return peaks;
}

void AUDIOMETERPEAKS_delete(AudioMeterPeaks peaks){
  V_free(peaks.RT_max_gains);
  V_free(peaks.max_dbs);
  V_free(peaks.decaying_dbs);
  V_free(peaks.falloff_dbs);
}
