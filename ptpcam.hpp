#include <string>
#include "libptp++.hpp"

std::string show_help(PTP::CHDKCamera &cam);
void reconnect(PTP::CHDKCamera &cam);
std::string chdk_version(PTP::CHDKCamera &cam);
void shutdown(PTP::CHDKCamera &cam);
void reboot(PTP::CHDKCamera &cam, std::string filename="");
void reboot_fi2(PTP::CHDKCamera &cam);