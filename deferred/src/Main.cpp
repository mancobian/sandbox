#include <iostream>
#include "OgreApplication.h"

#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
#define WIN32_LEAN_AND_MEAN
#include "windows.h"
#endif

void PrintUsage()
{
	using namespace std;
	cout << endl;
	cout << "Demo_DeferredLighting v1.0" << endl;
	cout << "==========================" << endl;
	cout << left << endl;
	cout << setw(25) << "Usage:" << "Demo_DeferredLighting [options]" << endl;
	cout << setw(25) << "Options:" << endl;
	cout << setw(25) << "  --scene [filename]";
	cout << setw(55) << "The absolute path of a scene file to load." << endl;
	cout << endl;
}

int main(int argc, char **argv)
{
	try
	{
		// Local vars
		std::string scene_file;

		// Parse command line args
		for (int i=0; i<argc; ++i)
		{
			if (strcmp(argv[i], "--scene") == 0)
			{
				scene_file = argv[++i];
			}
		}
		
		// Create the main app
		tsdc::OgreApplication app;

		// Specify a scene for loading
		if (!scene_file.empty())
			app.SceneName(scene_file);
		
		// Run the application
		app.Start();
	}
	catch (const std::string &e)
	{
		// Display exception message
#if OGRE_PLATFORM == PLATFORM_WIN32 || OGRE_PLATFORM == OGRE_PLATFORM_WIN32
		MessageBoxA(NULL, e.c_str(), "An exception has occurred!", MB_OK | MB_ICONERROR | MB_TASKMODAL);
#else
		fprintf(stderr, "An exception has occurred: %s\n", e.c_str());
#endif

		// Print usage and exit!
		PrintUsage();
	}
	return 0;
}
