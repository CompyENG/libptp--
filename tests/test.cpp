#include <iostream>
#include <libptp++/libptp++.hpp>

int main(int argc, char * argv[]) {
    PTP::PTPUSB *proto = new PTP::PTPUSB;
    proto->connect_to_first();

    PTP::CHDKCamera *cam = new PTP::CHDKCamera; 
    cam->set_protocol(proto);
    
    std::cout << "Connected." << std::endl;
    
    std::cout << "Version: " << cam->get_chdk_version() << std::endl;

    // Execute a lua script to switch the camera to "Record" mode.
    //  Second parameter, error_code, is NULL, because we don't care if an error
    //  occurs, and we aren't blocking to wait for one.
    uint32_t e;
    cam->execute_lua("switch_mode_usb(1)", &e, true);
    std::cout << "Executed. Error: " << e << std::endl;
    
    cam->execute_lua("shoot()", &e, false);
    std::cout << "Done. Error: " << e << std::endl;
    
    PTP::LVData d;
    cam->get_live_view_data(d);
    std::cout << "Got live view data." << std::endl;

    // The camera is closed automatically when the cam object is destroyed
    delete cam;
    delete proto;
}
