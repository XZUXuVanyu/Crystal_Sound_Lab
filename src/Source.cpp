import Crystal_Log;
using namespace Crystal;

int main(int argc, char** argv)
{
	Crystal_Log_Init();
	Crystal_Log_Set_Level(Log_Level::Warning);
	Crystal_Info_Log("Hello log");
	Crystal_Error_Log("Error");
	return 0;
}