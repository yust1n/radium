/*
  ==============================================================================

   This file is part of the JUCE 7 technical preview.
   Copyright (c) 2022 - Raw Material Software Limited

   You may use this code under the terms of the GPL v3
   (see www.gnu.org/licenses).

   For the technical preview this file cannot be licensed commercially.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/

namespace juce
{
namespace dsp
{

//==============================================================================
template <typename SampleType>
void Limiter<SampleType>::setThreshold (SampleType newThreshold)
{
    thresholddB = newThreshold;
    update();
}

template <typename SampleType>
void Limiter<SampleType>::setRelease (SampleType newRelease)
{
    releaseTime = newRelease;
    update();
}

//==============================================================================
template <typename SampleType>
void Limiter<SampleType>::prepare (const ProcessSpec& spec)
{
    jassert (spec.sampleRate > 0);
    jassert (spec.numChannels > 0);

    sampleRate = spec.sampleRate;

    firstStageCompressor.prepare (spec);
    secondStageCompressor.prepare (spec);

    update();
    reset();
}

template <typename SampleType>
void Limiter<SampleType>::reset()
{
    firstStageCompressor.reset();
    secondStageCompressor.reset();

    outputVolume.reset (sampleRate, 0.001);
}

//==============================================================================
template <typename SampleType>
void Limiter<SampleType>::update()
{
    firstStageCompressor.setThreshold ((SampleType) -10.0);
    firstStageCompressor.setRatio     ((SampleType) 4.0);
    firstStageCompressor.setAttack    ((SampleType) 2.0);
    firstStageCompressor.setRelease   ((SampleType) 200.0);

    secondStageCompressor.setThreshold (thresholddB);
    secondStageCompressor.setRatio     ((SampleType) 1000.0);
    secondStageCompressor.setAttack    ((SampleType) 0.001);
    secondStageCompressor.setRelease   (releaseTime);

    auto ratioInverse = (SampleType) (1.0 / 4.0);

    auto gain = (SampleType) std::pow (10.0, 10.0 * (1.0 - ratioInverse) / 40.0);
    gain *= Decibels::decibelsToGain (-thresholddB, (SampleType) -100.0);

    outputVolume.setTargetValue (gain);
}

//==============================================================================
template class Limiter<float>;
template class Limiter<double>;

} // namespace dsp
} // namespace juce