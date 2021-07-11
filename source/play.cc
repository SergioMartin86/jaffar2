#include "argparse.hpp"
#include "nlohmann/json.hpp"
#include "utils.h"
#include <ncurses.h>
#include <unistd.h>
#include "blastemInstance.h"

// Function to check for keypress taken from https://github.com/ajpaulson/learning-ncurses/blob/master/kbhit.c
int kbhit()
{
  int ch, r;

  // turn off getch() blocking and echo
  nodelay(stdscr, TRUE);
  noecho();

  // check for input
  ch = getch();
  if (ch == ERR) // no input
    r = FALSE;
  else // input
  {
    r = TRUE;
    ungetch(ch);
  }

  // restore block and echo
  echo();
  nodelay(stdscr, FALSE);

  return (r);
}

int getKeyPress()
{
  while (!kbhit())
  {
    usleep(100000ul);
    refresh();
  }
  return getch();
}

int main(int argc, char *argv[])
{
  // Defining arguments
  argparse::ArgumentParser program("jaffar2-play", "1.0");

  program.add_argument("romFile")
    .help("Specifies the path to the genesis rom file (.bin) from which to start.")
    .required();

  program.add_argument("savFile")
    .help("Specifies the path to the blastem state file (.state) from which to start.")
    .required();

  program.add_argument("solutionFile")
    .help("path to the Jaffar solution (.sol) file to run.")
    .required();

  program.add_argument("--reproduce")
    .help("Plays the entire sequence without interruptions")
    .default_value(false)
    .implicit_value(true);

  // Parsing command line
  try
  {
    program.parse_args(argc, argv);
  }
  catch (const std::runtime_error &err)
  {
    fprintf(stderr, "[Jaffar] Error parsing command line arguments: %s\n%s", err.what(), program.help().str().c_str());
    exit(-1);
  }

  // Getting reproduce path
  bool isReproduce = program.get<bool>("--reproduce");

  // Getting romfile path
  auto romFilePath = program.get<std::string>("romFile");

  // Getting savefile path
  std::string saveFilePath = program.get<std::string>("savFile");

  // Loading save file contents
  std::string saveString;
  bool status = loadStringFromFile(saveString, saveFilePath.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not load save state from file: %s\n", saveFilePath.c_str());

  // If sequence file defined, load it and play it
  std::string moveSequence;
  std::string solutionFile = program.get<std::string>("solutionFile");
  status = loadStringFromFile(moveSequence, solutionFile.c_str());
  if (status == false) EXIT_WITH_ERROR("[ERROR] Could not find or read from solution file: %s\n%s \n", solutionFile.c_str(), program.help().str().c_str());

  // Initializing ncurses screen
  initscr();
  cbreak();
  noecho();
  nodelay(stdscr, TRUE);
  scrollok(stdscr, TRUE);

  // Getting move list
  const auto moveList = split(moveSequence, ' ');

  // Getting sequence size
  const int sequenceLength = moveList.size();

  // Printing info
  printw("[Jaffar] Playing sequence file: %s\n", solutionFile.c_str());
  printw("[Jaffar] Sequence Size: %d moves.\n", sequenceLength-1);
  printw("[Jaffar] Generating frame sequence...\n");

  refresh();

  // Initializing replay generating SDLPop Instance
  blastemInstance genBlastem("libblastem.so", false);
  genBlastem.initialize(romFilePath.c_str(), saveFilePath.c_str(), true);

  // Storage for sequence frames
  std::vector<uint8_t*> frameSequence;

  // Saving initial frame
  frameSequence.push_back((uint8_t*) malloc(sizeof(uint8_t) * _STATE_DATA_SIZE));
  genBlastem.saveState(frameSequence[0]);

  // Iterating move list in the sequence
  for (int i = 0; i < sequenceLength; i++)
  {
    genBlastem.playFrame(moveList[i]);

    // Storing new frame
    frameSequence.push_back((uint8_t*) malloc(sizeof(uint8_t) * _STATE_DATA_SIZE));
    genBlastem.saveState(frameSequence[i+1]);
  }

  genBlastem.finalize();

  printw("[Jaffar] Opening BlastEm window...\n");

  // Initializing showing SDLPop Instance
  blastemInstance showBlastem("libblastem.so", false);
  showBlastem.initialize(romFilePath.c_str(), saveFilePath.c_str(), false);

  // Variable for current step in view
  int currentStep = 1;

  // Print command list
  if (isReproduce == false)
  {
   printw("[Jaffar] Available commands:\n");
   printw("[Jaffar]  n: -1 m: +1 | h: -10 | j: +10 | y: -100 | u: +100 \n");
   printw("[Jaffar]  s: quicksave | q: quit  \n");
  }

  // Flag to display frame information
  bool showFrameInfo = true;

  // Interactive section
  int command;
  do
  {
    // Loading requested step
    showBlastem.loadState(frameSequence[currentStep - 1]);
    showBlastem.redraw();

    if (showFrameInfo)
    {
      printw("[Jaffar2] ----------------------------------------------------------------\n");
      printw("[Jaffar2] Current Step #: %d / %d\n", currentStep, sequenceLength);
      printw("[Jaffar2]  + Move: %s\n", moveList[currentStep - 1].c_str());
      printw("[Jaffar2]  + Current Level: %2d\n", showBlastem._state.currentLevel);
      printw("[Jaffar2]  + Current Frame: %d\n", showBlastem._state.currentFrame);
      printw("[Jaffar2]  + [Kid]   Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, Direction: %s, HP: %d/%d\n", showBlastem._state.kidRoom, showBlastem._state.kidPositionX, showBlastem._state.kidPositionY, showBlastem._state.kidFrame, showBlastem._state.kidDirection == 255 ? "L" : "R", showBlastem._state.kidCurrentHP, showBlastem._state.kidMaxHP);
      printw("[Jaffar2]  + [Guard] Room: %d, Pos.x: %3d, Pos.y: %3d, Frame: %3d, Direction: %s, HP: %d/%d\n", showBlastem._state.guardRoom, showBlastem._state.guardPositionX, showBlastem._state.guardPositionY, showBlastem._state.guardFrame, showBlastem._state.guardDirection == 255 ? "L" : "R", showBlastem._state.guardCurrentHP, showBlastem._state.guardMaxHP);
    }

    // Resetting show frame info flag
    showFrameInfo = true;

    // If we're reproducing do not have an interactive interface
    if (isReproduce)
    {
     currentStep = currentStep + 1;
     if (currentStep > sequenceLength) break;
     continue;
    }

    // Get command
    command = getKeyPress();

    // Advance/Rewind commands
    if (command == 'n') currentStep = currentStep - 1;
    if (command == 'm') currentStep = currentStep + 1;
    if (command == 'h') currentStep = currentStep - 10;
    if (command == 'j') currentStep = currentStep + 10;
    if (command == 'y') currentStep = currentStep - 100;
    if (command == 'u') currentStep = currentStep + 100;

    // Correct current step if requested more than possible
    if (currentStep < 1) currentStep = 1;
    if (currentStep > sequenceLength) currentStep = sequenceLength;

    // Quicksave creation command
    if (command == 's')
    {
      // Storing replay file
      std::string saveFileName = "jaffar.sav";

      // Saving frame info to file
      bool status = saveBinToFile(frameSequence[currentStep - 1], sizeof(uint8_t) * _STATE_DATA_SIZE, saveFileName.c_str());
      if (status == true) printw("[Jaffar] State saved in '%s'.\n", saveFileName.c_str());
      if (status == false) printw("[Jaffar] Error saving file '%s'.\n", saveFileName.c_str());

      // Do no show frame info again after this action
      showFrameInfo = false;
    }

  } while (command != 'q');

  // Ending ncurses window
  endwin();
}
