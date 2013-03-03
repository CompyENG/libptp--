#ifndef LIBPTP_PP_CAMERABASE_H_
#define LIBPTP_PP_CAMERABASE_H_

#include <libusb-1.0/libusb.h>

namespace PTP {
    
    class PTPContainer;

    class CameraBase {
        private:
            libusb_device_handle *handle;
            int usb_error;
            struct libusb_interface_descriptor *intf;
            uint8_t ep_in;
            uint8_t ep_out;
            uint32_t _transaction_id;
            void init();
            
        protected:
            int _bulk_write(unsigned char * bytestr, int length, int timeout=0);
            int _bulk_read(unsigned char * data_out, int size, int * transferred, int timeout=0);
            int get_and_increment_transaction_id(); // What a beautiful name for a function
            
        public:
            CameraBase();
            CameraBase(libusb_device *dev);
            ~CameraBase();
            bool open(libusb_device *dev);
            bool close();
            bool reopen();
            int send_ptp_message(PTPContainer * cmd, int timeout=0);
            void recv_ptp_message(PTPContainer *out, int timeout=0);
            void ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving, PTPContainer *out_resp, PTPContainer *out_data, int timeout=0);
            static libusb_device * find_first_camera();
            int get_usb_error();
    };
}

#endif /* LIBPTP_PP_CAMERABASE_H_ */
