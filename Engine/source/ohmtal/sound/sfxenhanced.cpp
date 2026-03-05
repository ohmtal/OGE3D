/**
   sfxEnhanced 

   @since 2024-02-11
   @author XXTH

   goal is to play a melody changing the pitch
   ============================================
   * 1.  using oTuner (open source guitar tuner to see if i touch a note
   * 2.  sound setting on win: mmsys.cpl to get the output to input
   * 3.  the pitch is not linear so, i need something else to get it working
   *     i'am sure it's possible to calulate it - but how ?
   * 4.  openAL have a Resampler maybe this helps ? example: mOpenAL.alSourcef(mSourceName, AL_SOURCE_RESAMPLER_SOFT,  4); //BSinc24Resampler


   License at: ohmtal/misc/ohmtalMIT.h
*/

#include "platform/platform.h"
//#include "console/consoleObject.h"
//#include "console/console.h"
#include "console/engineAPI.h"
#include "sfx/sfxSource.h"


//----------------------------------------------------------------------------
//
// pitch is limited from 0.001f to 2.0f so i cant play higher tones than c2
// raised it to 33.f but my tuner did not get a right result
// returned to default 2 maybe used in car sound ?! 
// 
// $trollpain = sfxCreateSource(AudioMusic2D,"playground/data/sound/trollpain.ogg");
// $trollpain.playWithPitch(1.1);
//
//
// $g6 = sfxCreateSource(AudioMusic2D,"playground/data/sound/samples/guitar6.wav");
// how much pitch for one tone to another ...
// alle meine endchen C D E F E E
//  $g6.playWithPitch(1);
//
// NOT LINEAR !!!... mhhhhh
// $c = 1.04;
// C-:  $g6.playWithPitch($c-0.113-0.103-0.045-0.086-0.038-0.037-0.0347-0.063);
// D-:  $g6.playWithPitch($c-0.113-0.103-0.045-0.086-0.038-0.037-0.0347);
// E-:  $g6.playWithPitch($c-0.113-0.103-0.045-0.086-0.038-0.037);
// F-:  $g6.playWithPitch($c-0.113-0.103-0.045-0.086-0.038);
// G-:  $g6.playWithPitch($c-0.113-0.103-0.045-0.086); //???
// A-:  $g6.playWithPitch($c-0.113-0.103);
// B-:  $g6.playWithPitch($c-0.113);
// *C:  $g6.playWithPitch($c);
// D:   $g6.playWithPitch($c + 0.062);
// E:   $g6.playWithPitch($c + 0.062 + 0.134);
// F:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074);
// G:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16);
// A:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16 + 0.18);
// B:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16 + 0.18 + 0.2);
// C+:  $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16 + 0.18 + 0.2 + 0.11);
// D+:  NOT tuner maybe sucks ! $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16 + 0.18 + 0.2 + 0.11 + 0.37);
//-----------------------------------------------------------------------------


// $g6 = sfxCreateSource(AudioMusic2D,"playground/data/sound/samples/guitar6.wav");
// $g6.playTone(0,0.04);
//
// 
// function playScale(%i) {  $g6.playTone(%i,0.04);echo("playing" SPC %i); %i++; if (%i < 8)schedule(700,0,playscale, %i); }


// $g6 = sfxCreateSource(AudioMusic2D,"playground/data/sound/samples/guitar5.wav");


//----------------------------------------
//  Resampling ... cant here a difference ^^ 
// mOpenAL.alSourcef(mSourceName, AL_SOURCE_RESAMPLER_SOFT, /*BSinc24Resampler*/ 4);
//-----------------------------------------



//playTone: wanted to make it better but good enough for tonight!
// not correct i guess 
DefineEngineMethod(SFXSource, playTone, void, (S32 toneId, F32 pitchModulator),(0.f),
   "play tone C=0 ... C+=7, pitchmodulator if pitch == 1.f is not C\n"
   "@hide")
{
   //default c or something out of range. 
   F32 pitch = 1.f + pitchModulator; 
   switch (toneId)
   {
   case 1: //D
      // D:   $g6.playWithPitch($c + 0.062);
      pitch += 0.062f;
      break;
   case 2:
      // E:   $g6.playWithPitch($c + 0.062 + 0.134);
      pitch += 0.062f + 0.134f;
      break;
   case 3:
      // F:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074);
      pitch += 0.062f + 0.134f + 0.074f;
      break;
   case 4:
      // G:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16);
      pitch += 0.062f + 0.134f + 0.074f + 0.16f;
      break;
   case 5:
      // A:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16 + 0.18);
      pitch += 0.062f + 0.134f + 0.074f + 0.16f + 0.18f;
      break;
   case 6:
      // B:   $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16 + 0.18 + 0.2);
      pitch += 0.062f + 0.134f + 0.074f + 0.16f + 0.18f + 0.2f;
      break;
   case 7:
      // C+:  $g6.playWithPitch($c + 0.062 + 0.134 + 0.074 + 0.16 + 0.18 + 0.2 + 0.11);
      pitch += 0.062f + 0.134f + 0.074f + 0.16f + 0.18f + 0.2f + 0.11f;
      break;

   } //toneId

   object->stop();
   object->setPitch(pitch);
   object->play();
}
//-----------------------------------------------------------------------------
DefineEngineMethod(SFXSource, playWithPitch, void, (F32 pitch), ,
   "play with pitch\n"
   "@hide")
{
   object->stop();
   object->setPitch(pitch);
   object->play();
}

