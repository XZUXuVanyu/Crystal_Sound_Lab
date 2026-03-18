#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>

import Crystal_Log;
import Crystal_Core.Foundation;
using namespace Crystal;

int main(int argc, char** argv)
{
	
	//Init Log
	Crystal_Log_Init();
	Crystal_Log_Set_Level(Log_Level::Info);
	Crystal_Info_Log("Hello!");
	return 0;
}