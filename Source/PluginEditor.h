

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================

class DistortionAudioProcessorEditor : public AudioProcessorEditor,
public ComboBox::Listener
{
public:
    //==============================================================================

    DistortionAudioProcessorEditor (DistortionAudioProcessor&);
    ~DistortionAudioProcessorEditor();

    //==============================================================================

    void paint (Graphics&) override;
    void resized() override;
    void comboBoxChanged (ComboBox *comboBoxThatHasChanged) override;

    void lockInputSlider(); //Sets the input slider range between (-60.0, 7.6) dBs, to ensure that distortionTypeArayaSuyama dont overload.
    void stdInputSlider();  //Sets the input slider range back to (-60.0, 24.0)dBs, where all the others distortion types should work fine
    void doidicSymInputSlider();  //Sets the input slider range between (-60.0, 23.52) dBs, to ensure that doidicSym distortion dont overload
     void doidicAssyInputSlider();  //Sets the input slider range between (-60.0, 0) dBs, to ensure that tha the input signal never exceed the range  of work of the doidic Assymetric distortion namely [-1,1]

    
private:
    //==============================================================================

    DistortionAudioProcessor& processor;

    enum {
        editorWidth = 500,
        editorMargin = 10,
        editorPadding = 10,

        sliderTextEntryBoxWidth = 100,
        sliderTextEntryBoxHeight = 25,
        sliderHeight = 25,
        buttonHeight = 25,
        comboBoxHeight = 25,
        labelWidth = 100,
    };

    //======================================

    OwnedArray<Slider> sliders;
    OwnedArray<ToggleButton> toggles;
    OwnedArray<ComboBox> comboBoxes;

    OwnedArray<Label> labels;
    Array<Component*> components;

    typedef AudioProcessorValueTreeState::SliderAttachment SliderAttachment;
    typedef AudioProcessorValueTreeState::ButtonAttachment ButtonAttachment;
    typedef AudioProcessorValueTreeState::ComboBoxAttachment ComboBoxAttachment;

    OwnedArray<SliderAttachment> sliderAttachments;
    OwnedArray<ButtonAttachment> buttonAttachments;
    OwnedArray<ComboBoxAttachment> comboBoxAttachments;

    //==============================================================================

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DistortionAudioProcessorEditor)
};

//==============================================================================
