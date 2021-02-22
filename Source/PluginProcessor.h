

#pragma once

#define _USE_MATH_DEFINES
#include <cmath>

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginParameter.h"

//==============================================================================

class DistortionAudioProcessor : public AudioProcessor
{
public:
    //==============================================================================

    DistortionAudioProcessor();
    ~DistortionAudioProcessor();

    //==============================================================================

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void processBlock (AudioSampleBuffer&, MidiBuffer&) override;

    //==============================================================================






    //==============================================================================

    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================

    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    //==============================================================================

    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect () const override;
    double getTailLengthSeconds() const override;

    //==============================================================================

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================






    //==============================================================================

    StringArray distortionTypeItemsUI = {
        "Hard clipping",
        "Soft clipping",
        "Exponential",
        "Full-wave rectifier",
        "Half-wave rectifier",
        "Araya&Suyama System",
        "Doidic Symmetric",
        "Doidic Assymmetric"
    };

    enum distortionTypeIndex {
        distortionTypeHardClipping = 0,
        distortionTypeSoftClipping,
        distortionTypeExponential,
        distortionTypeFullWaveRectifier,
        distortionTypeHalfWaveRectifier,
        distortionTypeArayaSuyama,
        distortionTypeDoidicSymmetric,
        distortionTypeDoidicAssymetric
    };

    
    //=================================================================
    
    float hardClipping(const float& _in);
    float softClipping(const float& _in);
    float exponential(const float& _in);
    float fullWaveRectifier(const float& _in);
    float halfWaveRectifier(const float& _in);
    float ArayaAndSuyama(const float& _in);
    float doidicSymmetric(const float& _in);
    float doidicAssymetric(const float& _in);
    
    //======================================

    class Filter : public IIRFilter
    {
    public:
        void updateCoefficients (const double discreteFrequency,
                                 const double gain) noexcept
        {
            jassert (discreteFrequency > 0);

            double tan_half_wc = tan (discreteFrequency / 2.0);
            double sqrt_gain = sqrt (gain);

            coefficients = IIRCoefficients (/* b0 */ sqrt_gain * tan_half_wc + gain,
                                            /* b1 */ sqrt_gain * tan_half_wc - gain,
                                            /* b2 */ 0.0,
                                            /* a0 */ sqrt_gain * tan_half_wc + 1.0,
                                            /* a1 */ sqrt_gain * tan_half_wc - 1.0,
                                            /* a2 */ 0.0);

            setCoefficients (coefficients);
        }
    };

    OwnedArray<Filter> filters;
    void updateFilters();

   // dsp::Oversampling<float>* oversampledBlock = new dsp::Oversampling<float> (2,2, dsp::Oversampling<float>::FilterType::filterHalfBandFIREquiripple);
    
    //======================================

    PluginParametersManager parameters;

    PluginParameterComboBox paramDistortionType;
    PluginParameterLinSlider paramInputGain;
    PluginParameterLinSlider paramOutputGain;
    PluginParameterLinSlider paramTone;
    

private:
    //==============================================================================
    
    enum
    {
        inputGainIndex = 0,
        waveshaperIndex,
        outputGainIndex
    };


    dsp::Gain<float> inputGain, outputGain;

    std::array<dsp::WaveShaper<float, std::function<float(float)>>, 8> waveShapers {
        { { [this](float _in){ return hardClipping(_in); } },
          { [this](float _in){ return softClipping(_in); } },
          { [this](float _in){ return exponential(_in); } },
          { [this](float _in){ return fullWaveRectifier(_in); } },
          { [this](float _in){ return halfWaveRectifier(_in); } },
          { [this](float _in){ return ArayaAndSuyama(_in); } },
          { [this](float _in){ return doidicSymmetric(_in); } },
          { [this](float _in){ return doidicAssymetric(_in); } }
            
        } };
    
    dsp::DryWetMixer<float> mixer { 10 };
    dsp::Oversampling<float> oversampler { 2, 3, dsp::Oversampling<float>::filterHalfBandPolyphaseIIR, true, false };

    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionAudioProcessor)
};
