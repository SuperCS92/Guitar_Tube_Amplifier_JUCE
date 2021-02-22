

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

DistortionAudioProcessorEditor::DistortionAudioProcessorEditor (DistortionAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p)
{
    const Array<AudioProcessorParameter*> parameters = processor.getParameters();
    int comboBoxCounter = 0;

    int editorHeight = 2 * editorMargin;
    for (int i = 0; i < parameters.size(); ++i) {
        if (const AudioProcessorParameterWithID* parameter =
                dynamic_cast<AudioProcessorParameterWithID*> (parameters[i])) {

            if (processor.parameters.parameterTypes[i] == "Slider") {
                Slider* aSlider;
                sliders.add (aSlider = new Slider());
                aSlider->setTextValueSuffix (parameter->label);
                aSlider->setTextBoxStyle (Slider::TextBoxLeft,
                                          false,
                                          sliderTextEntryBoxWidth,
                                          sliderTextEntryBoxHeight);

                SliderAttachment* aSliderAttachment;
                sliderAttachments.add (aSliderAttachment =
                    new SliderAttachment (processor.parameters.apvts, parameter->paramID, *aSlider));

                components.add (aSlider);
                editorHeight += sliderHeight;
            }

            //======================================

            else if (processor.parameters.parameterTypes[i] == "ToggleButton") {
                ToggleButton* aButton;
                toggles.add (aButton = new ToggleButton());
                aButton->setToggleState (parameter->getDefaultValue(), dontSendNotification);

                ButtonAttachment* aButtonAttachment;
                buttonAttachments.add (aButtonAttachment =
                    new ButtonAttachment (processor.parameters.apvts, parameter->paramID, *aButton));

                components.add (aButton);
                editorHeight += buttonHeight;
            }

            //======================================

            else if (processor.parameters.parameterTypes[i] == "ComboBox") {
                ComboBox* aComboBox;
                comboBoxes.add (aComboBox = new ComboBox());
                aComboBox->addListener(this);
                aComboBox->setEditableText (false);
                aComboBox->setJustificationType (Justification::left);
                aComboBox->addItemList (processor.parameters.comboBoxItemLists[comboBoxCounter++], 1);

                ComboBoxAttachment* aComboBoxAttachment;
                comboBoxAttachments.add (aComboBoxAttachment =
                    new ComboBoxAttachment (processor.parameters.apvts, parameter->paramID, *aComboBox));

                components.add (aComboBox);
                editorHeight += comboBoxHeight;
            }

            //======================================

            Label* aLabel;
            labels.add (aLabel = new Label (parameter->name, parameter->name));
            aLabel->attachToComponent (components.getLast(), true);
            addAndMakeVisible (aLabel);

            components.getLast()->setName (parameter->name);
            components.getLast()->setComponentID (parameter->paramID);
            addAndMakeVisible (components.getLast());
        }
    }

    //======================================

    editorHeight += components.size() * editorPadding;
    setSize (editorWidth, editorHeight);
}

DistortionAudioProcessorEditor::~DistortionAudioProcessorEditor()
{
}

//==============================================================================

void DistortionAudioProcessorEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

void DistortionAudioProcessorEditor::resized()
{
    Rectangle<int> r = getLocalBounds().reduced (editorMargin);
    r = r.removeFromRight (r.getWidth() - labelWidth);

    for (int i = 0; i < components.size(); ++i) {
        if (Slider* aSlider = dynamic_cast<Slider*> (components[i]))
            components[i]->setBounds (r.removeFromTop (sliderHeight));

        if (ToggleButton* aButton = dynamic_cast<ToggleButton*> (components[i]))
            components[i]->setBounds (r.removeFromTop (buttonHeight));

        if (ComboBox* aComboBox = dynamic_cast<ComboBox*> (components[i]))
            components[i]->setBounds (r.removeFromTop (comboBoxHeight));

        r = r.removeFromBottom (r.getHeight() - editorPadding);
    }
}

//==============================================================================

void DistortionAudioProcessorEditor::lockInputSlider()
{
    for (int i = 0 ;i <= components.size() - 1; i++) {
      
        Component* parameter = components[i];
        
        if (parameter->getName() == "Input gain") {
            Slider* aSlider = dynamic_cast<Slider*> (components[i]);
            
            aSlider->setRange(-60.0f, 6.0f);
            aSlider->setValue(0.0f);

        }
        
        if (parameter->getName() == "Tone") {
            Slider* aSlider = dynamic_cast<Slider*> (components[i]);
            
            aSlider->setRange(-24.0f, 8.0f);
            aSlider->setValue(8.0);
        }
    }
}

void DistortionAudioProcessorEditor::stdInputSlider()
{
    for (int i = 0 ;i <= components.size() - 1; i++) {
        
        Component* parameter = components[i];
        
        if (parameter->getName() == "Input gain") {
            Slider* aSlider = dynamic_cast<Slider*> (components[i]);
            
            aSlider->setRange(-60.0f, 24.0f);
            aSlider->setValue(12.0f);

        }
    }
}

void DistortionAudioProcessorEditor::doidicSymInputSlider()
{
    for (int i = 0 ;i <= components.size() - 1; i++) {
        
        Component* parameter = components[i];
        
        if (parameter->getName() == "Input gain") {
            Slider* aSlider = dynamic_cast<Slider*> (components[i]);
            
            aSlider->setRange(-60.0f, 21.5836f);
            aSlider->setValue(0);
        }
        
        if (parameter->getName() == "Tone") {
            Slider* aSlider = dynamic_cast<Slider*> (components[i]);
            
            aSlider->setValue(12.0);
        }
    }
}

void DistortionAudioProcessorEditor::doidicAssyInputSlider()
{
    for (int i = 0 ;i <= components.size() - 1; i++) {
        
        Component* parameter = components[i];
        
        if (parameter->getName() == "Input gain") {
            Slider* aSlider = dynamic_cast<Slider*> (components[i]);
            
            aSlider->setRange(-60.0f, 0.0f);
            aSlider->setValue(-12.0f);
        }
        
        if (parameter->getName() == "Tone") {
            Slider* aSlider = dynamic_cast<Slider*> (components[i]);
            
            aSlider->setValue(12.0);
        }
    }
}

void DistortionAudioProcessorEditor::comboBoxChanged (ComboBox *comboBoxThatHasChanged)
{
    for (int i = 0 ;i <= components.size() - 1; i++)
    {
        
        Component* parameter = components[i];
        
        if (parameter->getName() == "Distortion type")
        {
            ComboBox* aComboBox = dynamic_cast<ComboBox*> (components[i]);
            
            if (comboBoxThatHasChanged == aComboBox)
            {
                
              if(comboBoxThatHasChanged->getSelectedItemIndex() == 5)
              {
                  lockInputSlider();
              }
              else if (comboBoxThatHasChanged->getSelectedItemIndex() == 6 )
              {
                  doidicSymInputSlider();
              }
             else if (comboBoxThatHasChanged->getSelectedItemIndex() == 7)
             {
                 doidicAssyInputSlider();
             }
              else
              {
                  stdInputSlider();
              }
                
            }
        }
    }
    
   
    
    
}
























