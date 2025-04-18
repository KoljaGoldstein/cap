#include "injector.h"

// cap.exe hoi4.exe dll.dll
int main(int argc, char** argv) {
	if (argc != 3) {
		std::cout << "Wrong syntax: cap.exe <process_> <dll_path>" << std::endl;
		return 0;
	}

	cap::Injector* injector = new cap::Injector(argv[1], argv[2]);
	if (!injector->Inject()) {
		return cap::Error::GetLastError();
	}

	std::cout << "Finished injecting..." << std::endl;
}