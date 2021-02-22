

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "PluginParameter.h"

//==============================================================================

DistortionAudioProcessor::DistortionAudioProcessor():
#ifndef JucePlugin_PreferredChannelConfigurations
    AudioProcessor (BusesProperties()
                    #if ! JucePlugin_IsMidiEffect
                     #if ! JucePlugin_IsSynth
                      .withInput  ("Input",  AudioChannelSet::stereo(), true)
                     #endif
                      .withOutput ("Output", AudioChannelSet::stereo(), true)
                    #endif
                   ),
#endif
    parameters (*this)
    , paramDistortionType (parameters, "Distortion type", distortionTypeItemsUI, distortionTypeFullWaveRectifier)
    , paramInputGain (parameters, "Input gain", "dB", -60.0f, 24.0f, 12.0f,
                      [](float value){ return powf (10.0f, value * 0.05f); })
    , paramOutputGain (parameters, "Output gain", "dB", -60.0f, 24.0f, -24.0f,
                       [](float value){ return powf (10.0f, value * 0.05f); })
    , paramTone (parameters, "Tone", "dB", -24.0f, 24.0f, 12.0f,
                 [this](float value){ paramTone.setCurrentAndTargetValue (value); updateFilters(); return value; })
{
    parameters.apvts.state = ValueTree (Identifier (getName().removeCharacters ("- ")));
    mixer.setMixingRule (dsp::DryWetMixingRule::linear);

}

DistortionAudioProcessor::~DistortionAudioProcessor()
{
}

//==============================================================================

void DistortionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    const double smoothTime = 1e-3;
    paramDistortionType.reset (sampleRate, smoothTime);
    paramInputGain.reset (sampleRate, smoothTime);
    paramOutputGain.reset (sampleRate, smoothTime);
    paramTone.reset (sampleRate, smoothTime);
    //======================================
    dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumInputChannels();
    
    inputGain.prepare(spec);
    outputGain.prepare(spec);
    
    for (auto& waveshaper : waveShapers)
    {
        waveshaper.prepare(spec);
    }
    //======================================
    oversampler.initProcessing(samplesPerBlock);
    mixer.setMixingRule(dsp::DryWetMixingRule::linear);
    mixer.prepare(spec);
    
    
    //======================================

    filters.clear();
    for (int i = 0; i < getTotalNumInputChannels(); ++i) {
        Filter* filter;
        filters.add (filter = new Filter());
    }
    updateFilters();
    
    //======================================

    //We need to limit the input when using ArayaAndSuyama
    //otherwise, f(x) will tend to infinite
    //if((int)paramDistortionType.getTargetValue() == 5  && paramInputGain.getNextValue() > 7.6f)
    
    //======================================
    
}

void DistortionAudioProcessor::releaseResources()
{
}

void DistortionAudioProcessor::processBlock (AudioSampleBuffer& buffer, MidiBuffer& midiMessages)
{
    ScopedNoDenormals noDenormals;

    const int numInputChannels = getTotalNumInputChannels();
    const int numOutputChannels = getTotalNumOutputChannels();
    const int numSamples = buffer.getNumSamples();
    
    //======================================
    
    dsp::AudioBlock<float> audioBlock (buffer);
    
    inputGain.setGainLinear(paramInputGain.getNextValue());
    dsp::ProcessContextReplacing<float> inputGainBlock (audioBlock);
        
    inputGain.process(inputGainBlock);
    
    dsp::ProcessContextReplacing<float> filterBlock (inputGainBlock);
    
    for (int i = 0; i < filters.size(); i++) {
       const auto& filter = filters.getUnchecked(i);
        
        auto& block = filterBlock.getOutputBlock();
        float* samples= block.getChannelPointer(i);
        const int numSamples = (int) block.getNumSamples();
        filter->processSamples(samples, numSamples);
    }

    auto& waveshaper = waveShapers[(int) paramDistortionType.getTargetValue()];
    dsp::ProcessContextReplacing<float> distortionBlock (filterBlock);
    waveshaper.process(distortionBlock);
    
    outputGain.setGainLinear(paramOutputGain.getNextValue());
    dsp::ProcessContextReplacing<float> outputGainBlock (distortionBlock);
    outputGain.process(outputGainBlock);
    
    //======================================
    for (int channel = numInputChannels; channel < numOutputChannels; ++channel)
        buffer.clear (channel, 0, numSamples);
}

//==============================================================================

void DistortionAudioProcessor::updateFilters()
{
    double discreteFrequency = M_PI * 0.01;
    double gain = pow (10.0, (double)paramTone.getTargetValue() * 0.05);

    for (int i = 0; i < filters.size(); ++i)
        filters[i]->updateCoefficients (discreteFrequency, gain);
}

//==============================================================================

float DistortionAudioProcessor::hardClipping(const float& _in)
{
    float out;
    float threshold = 0.5f;
    
    if (_in > threshold)
        out = threshold;
    else if (_in < -threshold)
        out = -threshold;
    else
        out = _in;
    
    return out;
}

float DistortionAudioProcessor::softClipping(const float& _in)
{
    float out;
    float threshold1 = 1.0f / 3.0f;
    float threshold2 = 2.0f / 3.0f;
    
    if (_in > threshold2)
        out = 1.0f;
    else if (_in > threshold1)
        out = 1.0f - powf (2.0f - 3.0f * _in, 2.0f) / 3.0f;
    else if (_in < -threshold2)
        out = -1.0f;
    else if (_in < -threshold1)
        out = -1.0f + powf (2.0f + 3.0f * _in, 2.0f) / 3.0f;
    else
        out = 2.0f * _in;
        out *= 0.5f;
    
    return out;
}

float DistortionAudioProcessor::exponential(const float& _in)
{
    float out;
    
    if (_in > 0.0f)
        out = 1.0f - expf (-_in);
    else
        out = -1.0f + expf (_in);
    
    return out;
}

float DistortionAudioProcessor::fullWaveRectifier(const float& _in)
{
    float out;
    
    out = fabsf (_in);
    
    return out;

}

float DistortionAudioProcessor::halfWaveRectifier(const float& _in)
{
    float out;
    
    if (_in > 0.0f)
        out = _in;
    else
        out = 0.0f;
    
    return out;
    
}

float DistortionAudioProcessor::ArayaAndSuyama(const float &_in)
{
    float out;
    
    out = (3/2) * (_in) * (1 - pow(_in, 2)/3);
    out = (3/2) * (out) * (1 - pow(out, 2)/3);
    out = (3/2) * (out) * (1 - pow(out, 2)/3);

    return out;
    
}

float DistortionAudioProcessor::doidicSymmetric(const float& _in)
{
    float out;
    
    out =  ( (2*fabsf(_in))  - pow(_in, 2)) * copysignf(1, _in);
    
    return out;
}

float DistortionAudioProcessor::doidicAssymetric(const float& _in)
{
    float out;
    
    if (_in >= -1 && _in < -0.08905f) {
        out = -(0.75)*( 1 - (1 - pow(fabs(_in) - 0.032847, 12)) + 1/3*(fabs(_in) - 0.032847)) + 0.01;
    }
    else if (_in >= -0.08905f && _in < 0.320018)
    {
        out = -6.153 * pow(_in,2) + 3.9375 * _in;
    }
    else if (_in >= 0.320018 && _in <= 1)
    {
        out = 0.630035;
    }
    
    return out;
    
}


//==============================================================================


//==============================================================================

void DistortionAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    auto state = parameters.apvts.copyState();
    std::unique_ptr<XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void DistortionAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName (parameters.apvts.state.getType()))
            parameters.apvts.replaceState (ValueTree::fromXml (*xmlState));
}

//==============================================================================

AudioProcessorEditor* DistortionAudioProcessor::createEditor()
{
    return new DistortionAudioProcessorEditor (*this);
}

bool DistortionAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool DistortionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

//==============================================================================

const String DistortionAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DistortionAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DistortionAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DistortionAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================

int DistortionAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DistortionAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DistortionAudioProcessor::setCurrentProgram (int index)
{
}

const String DistortionAudioProcessor::getProgramName (int index)
{
    return {};
}

void DistortionAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================

// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DistortionAudioProcessor();
}

//==============================================================================
