#include <string>
#include <iostream>

#include "ptpcam.hpp"
#include "libptp++.hpp"

int main(int argc, char * argv[]) {
	std::string input;
	PTP::CHDKCamera cam;

	do {
		std::cout << "chdk > ";
		std::getline(std::cin, input);

		if(input[0] == 'h') {
			std::cout << show_help();
		} else if(input[0] == 'r') {
			reconnect(cam);
		} else if(input == "version") {
			std::cout << "Version: " << chdk_version(cam);
		} else if(input == "shutdown") {
			shutdown(cam);
		} else if(input == "reboot") {
			reboot(cam);
		} else if(input.compare(0, 6, "reboot") == 0) {
			reboot(cam, input.substr(7));
		} else if(input == "reboot-fi2") {
			reboot_fi2(cam);
		} else if(input[0] == 'm' || input.compare(0, 6, "memory") == 0) {
			if(input.find('-') > 0) {

			}
		}

	} while(input[0] != 'q');

	return 0;
}