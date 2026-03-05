
// Always declare audio Descriptions (the type of sound) before Profiles (the
// sound itself) when creating new ones.

// ----------------------------------------------------------------------------
// Now for the profiles - these are the usable sounds
// ----------------------------------------------------------------------------

datablock SFXProfile(ThrowSnd)
{
   filename = $defaultGame @ "/art/sound/throw";
   description = AudioClose3d;
   preload = false;
};
