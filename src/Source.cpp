#include <atomic>
#include <condition_variable>
#include <format>
#include <iostream>
#include <memory>
#include <mutex>
#include <regex>
#include <string>
#include <thread>

import Crystal_Log;
import Crystal_Core.Foundation;
using namespace Crystal;

int main(int argc, char** argv)
{
	
	//Init Log
	Crystal_Log_Init();
	Crystal_Log_Set_Level(Log_Level::Info);

	//Search for devices
	return 0;
}