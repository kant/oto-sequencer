#include <functional>
#include <iostream>
#include <stdio.h>  
#include <fstream>
#include <string>

#include "../lib/ml/rapidLib.h"

#include "SimpleClock.h"
#include "Sequencer.h"
#include "SequencerUtils.h"
#include "RapidLibUtils.h"
#include "MidiUtils.h"

#include <unistd.h>
#include <termios.h>

// getch with no echo and instant response
// https://stackoverflow.com/a/912796/1240660
char getch() {
  char buf = 0;
  struct termios old = {0};
  if (tcgetattr(0, &old) < 0)
          perror("tcsetattr()");
  old.c_lflag &= ~ICANON;
  old.c_lflag &= ~ECHO;
  old.c_cc[VMIN] = 1;
  old.c_cc[VTIME] = 0;
  if (tcsetattr(0, TCSANOW, &old) < 0)
          perror("tcsetattr ICANON");
  if (read(0, &buf, 1) < 0)
          perror ("read()");
  old.c_lflag |= ICANON;
  old.c_lflag |= ECHO;
  if (tcsetattr(0, TCSADRAIN, &old) < 0)
          perror ("tcsetattr ~ICANON");
  return (buf);
}

bool drawing = false;
void redraw(Sequencer& seqr, SequencerEditor& seqEditor)
{ 
  if (drawing)
  {
    return;
  }
  drawing = true;
    std::cout << "\x1B[2J\x1B[H";
    std::string disp = SequencerViewer::toTextDisplay(9, 13, &seqr, &seqEditor);
    std::cout << disp << std::endl;

    std::string port{"/dev/ttyACM0"};
    ofstream serial_bus;
    serial_bus.open (port);
    serial_bus << disp << "\t"; // last character triggers the redraw
    serial_bus.close();
    drawing = false; 
}

int main()
{
    const std::map<char, double> key_to_note =
    {
      { 'z', 48},
      { 's', 49},
      { 'x', 50},
      { 'd', 51},
      { 'c', 52},
      { 'f', 53},
      { 'v', 54},
      { 'b', 55},
      { 'h', 56},
      { 'n', 57},
      { 'j', 59},
      { 'm', 60}
    };
    std::map<char, double>::iterator it;
    
    MidiUtils midiUtils;
    midiUtils.interactiveInitMidi();
    midiUtils.allNotesOff();
  
    SimpleClock clock{};

    //NaiveStepDataReceiver midiStepReceiver;
    Sequencer seqr{16, 8};
    SequencerEditor seqEditor{&seqr};
   
    // set up a midi note triggering callback 
    // on all steps
    seqr.setAllCallbacks(
        [&midiUtils, &clock](std::vector<double>* data){
          if (data->size() >= 3)
          {
            double channel = data->at(Step::channelInd);
            double offTick = clock.getCurrentTick() + data->at(Step::lengthInd);
            // make the length quantised by steps
            double noteVolocity = data->at(Step::velInd);
            double noteOne = data->at(Step::note1Ind);
            midiUtils.playSingleNote(channel, noteOne, noteVolocity, offTick);            
          }
        }
    );

  // tick the sequencer and send any queued notes
    clock.setCallback([&seqr, &seqEditor, &midiUtils, &clock](){
      //std::cout << "main.cpp clock callback " << clock.getCurrentTick() << std::endl;
      midiUtils.sendQueuedMessages(clock.getCurrentTick());
      seqr.tick();
      redraw(seqr, seqEditor);    
    });

    // this will map joystick x,y to 16 sequences
    rapidLib::regression network = NeuralNetwork::getMelodyStepsRegressor();
    int clockIntervalMs = 125;  
    clock.start(clockIntervalMs);
    char input {1};
    bool escaped = false;
    while (input != 'q')
    {
      input = getch();
      if (!escaped)
      {
        switch(input)
        {
          case '\033': // first escape character cursor key?
            escaped = true;
            continue;
          case '\t': 
            seqEditor.cycleEditMode();
            continue;
          case ' ':
            seqEditor.cycleAtCursor();
            continue;
          case '-':
            clockIntervalMs += 5;
            clock.stop();
            clock.start(clockIntervalMs);
            continue;
          case '=':
            clockIntervalMs -= 5;
            clock.stop();
            clock.start(clockIntervalMs);
            continue;
          case '\n':
            //seqEditor.cycleMode();
            seqEditor.enterAtCursor();
            continue;
          case (wchar_t)(127):
            seqEditor.enterNoteData(0);
            continue;
        }// send switch on key
        // now check all the piano keys
        bool key_note_match{false};
        for (const std::pair<char, double>& key_note : key_to_note)
        {
          if (input == key_note.first) 
          { 
           // std::cout << "match" << key_note.second << std::endl;
            key_note_match = true;
            seqEditor.enterNoteData(key_note.second);
            redraw(seqr, seqEditor);
            break;// break the for loop
          }
        }
        //if (key_note_match) continue; // continue the while loop
      } // end if !escapted
      if (escaped){
        switch(input){
          case '[': 
            continue;
          case 'A':
            // up
            seqEditor.moveCursorUp();
            escaped = false;
            redraw(seqr, seqEditor);
            continue;
          case 'D':
            // left
            seqEditor.moveCursorLeft();
            escaped = false;
            redraw(seqr, seqEditor);
            continue;
          case 'C':
            // right
            seqEditor.moveCursorRight();
            escaped = false;
            redraw(seqr, seqEditor);
            continue;
          case 'B':
            // down
            seqEditor.moveCursorDown();
            escaped = false;
            redraw(seqr, seqEditor);
            continue;     
        }
      }
    }
  clock.stop();

  midiUtils.allNotesOff();
  return 0;
}


