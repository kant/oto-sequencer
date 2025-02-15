/**
 * Sequencer stores a set of sequences each of which stores a step
 * By Matthew Yee-King 2020
 * Porbably should separate into header and cpp at some point
 */

#pragma once 

#include <vector>
#include <iostream>
#include <string>
#include <functional>
#include <map>
#include "MidiUtils.h"

/** default spec for a Step's data, 
 * so data[0] specifies length, 
 * data[1] specifies velocity 
 * and data[2] is the first note
 * 
*/
class Step{
  
  public:
  // these should be in an enum probably
    const static int channelInd{0};
    const static int lengthInd{1};
    const static int velInd{2};
    const static int note1Ind{3};
  
    Step();
    /** returns a copy of the data stored in this step*/
    std::vector<double> getData() const;
    /** get the memory address of the data in this step for direct access*/
    std::vector<double>* getDataDirect();
  
    /** sets the data stored in this step */
    void setData(std::vector<double> data);
    /** update one value in the data vector for this step*/
    void updateData(unsigned int dataInd, double value);
    /** set the callback function called when this step is triggered*/
    void setCallback(std::function<void(std::vector<double>*)> callback);
    /** return the callback for this step*/
    std::function<void(std::vector<double>*)> getCallback();
    /** trigger this step, causing it to pass its data to its callback*/
    void trigger();
    /** toggle the activity status of this step*/
    void toggleActive();
    /** returns the activity status of this step */
    bool isActive() const;
  private: 
    std::vector<double> data;
    bool active;
    std::function<void(std::vector<double>*)> stepCallback;
};

/** need this so can have a Sequencer data member in Sequence*/
class Sequencer;

/** use to define the type of a sequence. 
 * midiNote sends midi notes out
 * samplePlayer triggers internal samples
 * transposer transposes another sequence 
 **/
enum class SequenceType {midiNote, drumMidi, samplePlayer, transposer, lengthChanger, tickChanger};

class Sequence{
  public:
    Sequence(Sequencer* sequencer, unsigned int seqLength = 16, unsigned short midiChannel = 1);
    /** go to the next step */
    void tick();
    /** which step are you on? */
    unsigned int getCurrentStep() const;
    /** is this step number valid? */
    bool assertStep(unsigned int step) const;
    /** retrieve a copy of the step data for the sent step */
    std::vector<double> getStepData(int step) const;
    /** get the momory address of the step data for the requested step*/
    std::vector<double>* getStepDataDirect(int step);
    /** set the data for the sent step */
    void setStepData(unsigned int step, std::vector<double> data);
    /** retrieve a copy of the step data for the current step */
    std::vector<double> getCurrentStepData() const;
    /** what is the length of the sequence? Length is a temporary property used
     * to define the playback length. There might be more steps than this
    */
    unsigned int getLength() const;
    /** set the length of the sequence 
     * If it is higher than the current max length, new steps will be created
    */
    void setLength(int length);
    /**
     * Set the permanent tick per step. To apply a temporary
     * change, call setTicksPerStepAdjustment
     */
    void setTicksPerStep(int ticksPerStep);
    /** set a new ticks per step until the sequence hits step 0*/
    void setTicksPerStepAdjustment(int ticksPerStep);
    /** return my permanent ticks per step (not the adjusted one)*/
    int getTicksPerStep() const;
    /** apply a transpose to the sequence, which is reset when the sequence
     * hits step 0 again
     */
    void setTranspose(double transpose);
    /** apply a length adjustment to the sequence. This immediately changes the length.
     * It is reset when the sequence
     * hits step 0 again
     */
    void setLengthAdjustment(int lengthAdjust);

    /** how many steps does this sequence have it total. This is independent of the length. Length can be lower than how many steps*/
    unsigned int howManySteps() const ;
    
    /** update a single data value in a given step*/
    void updateStepData(unsigned int step, unsigned int dataInd, double value);
    /** set the callback for the sent step */
    void setStepCallback(unsigned int step, 
                  std::function<void (std::vector<double>*)> callback);
    std::string stepToString(int step) const;
    /** activate/ deactive the sent step */
    void toggleActive(unsigned int step);
    /** check if the sent step is active */
    bool isStepActive(unsigned int step) const;
    /** set the sequence type */
    void setType(SequenceType type);
    SequenceType getType() const;
  /** add a transpose processor to this sequence. 
     * Normally, a transposer type sequence will call this on a midiNote type seqience
     * to apply a transpose to it 
    */
//    void setStepProcessorTranspose(StepDataTranspose transpose);
    /** deactivate all data processors, e.g. transposers, length adjusters */
    void deactivateProcessors();
    /** clear the data from this sequence. Does not clear step event functions*/
    void reset();

  private:
    /** function called when the sequence ticks and it is SequenceType::midiNote
     * 
    */
    void triggerMidiNoteType(); 
    /** functoin called when the sequence ticks and it is SequenceType::midiDrum*/
    void triggerMidiDrumType(); 
    /** 
     * Called when the sequence ticks and it is a transpose type SequenceType::transposer
    */
    void triggerTransposeType();
    /**
     * Called when the sequence ticks and it is SequenceType::lengthChanger
     */
    void triggerLengthType();
    /**
     * Called when the sequence ticks and it is SequenceType::tickChanger
     */
    void triggerTickType();
    /** provides access to the sequencer so this sequence can change things*/
    Sequencer* sequencer;
    unsigned int currentLength;
    unsigned int currentStep;
    unsigned short midiChannel;
    std::vector<Step> steps;
    SequenceType type;
    // temporary sequencer adjustment parameters that get reset at step 0
    double transpose; 
    int lengthAdjustment;
    int ticksPerStep;
    /** stores the current default for this sequence, whereas ticksperstep 
     * is the temporarily adjusted one 
     */
    int originalTicksPerStep;
    int ticksElapsed;
    /** maps from linear midi scale to general midi drum notes*/
    std::map<int,int> midiScaleToDrum;

};

/** represents a sequencer which is used to store a grid of data and to step through it */
class Sequencer  {
    public:
    /** create a sequencer: channels,stepsPerChannel*/
      Sequencer(unsigned int seqCount = 4, unsigned int seqLength = 16);
      ~Sequencer();

      unsigned int howManySequences() const ;
      unsigned int howManySteps(unsigned int sequence) const ;
      unsigned int getCurrentStep(unsigned int sequence) const;
      SequenceType getSequenceType(unsigned int sequence) const;
      unsigned int getSequenceTicksPerStep(unsigned int sequence) const;

      /** move the sequencer along by one tick */
      void tick();
      /** return a pointer to the sequence with sent id*/
      Sequence* getSequence(unsigned int sequence);
      void setSequenceType(unsigned int sequence, SequenceType type);
       /** set the length of the sequence 
       * If it is higher than the current max length, new steps will be created
       * using callbacks that are copies of the one at the last, previously existant
       * step
       */
      void setSequenceLength(unsigned int sequence, unsigned int length);
      /** reduce the playback (as opposed to total possible) length of the sequence by 1 */
      void shrinkSequence(unsigned int sequence);
      /** increase the length of the sequence by 1, adding new steps in memory if needed, as per setSequenceLength*/
      void extendSequence(unsigned int sequence);
      /** set all callbacks on all sequences to the sent lambda*/
      void setAllCallbacks(std::function<void (std::vector<double>*)> callback);
      /** set a callback lambda for all steps in a sequence*/
      void setSequenceCallback(unsigned int sequence, std::function<void (std::vector<double>*)> callback);
      /** set a lambda to call when a particular step in a particular sequence happens */
      void setStepCallback(unsigned int sequence, unsigned int step, std::function<void (std::vector<double>*)> callback);
      /** update the data stored at a step in the sequencer */
      void setStepData(unsigned int sequence, unsigned int step, std::vector<double> data);
      /** update a single value in the  data 
       * stored at a step in the sequencer */
      void updateStepData(unsigned int sequence, unsigned int step, unsigned int dataInd, double value);
      /** retrieve the data for the current step */
      std::vector<double> getCurrentStepData(int sequence) const;
  
      /** retrieve the data for a specific step */
      std::vector<double> getStepData(int sequence, int step) const;
      /** get the memory address of the data for this step for direct viewing/ editing*/
      std::vector<double>* getStepDataDirect(int sequence, int step);
      void toggleActive(int sequence, int step);
      bool isStepActive(int sequence, int step) const;
      void addStepListener();
      /** wipe the data from the sent sequence*/
      void resetSequence(int sequence);
  /** print out a tracker style view of the sequence */
      std::string toString();

    private:
      bool assertSeqAndStep(unsigned int sequence, unsigned int step) const;
        
      bool assertSequence(unsigned int sequence) const;
      
      /// class data members  
      std::vector<Sequence> sequences;;
};





