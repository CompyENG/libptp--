/**
 * @file CameraBase.cpp
 * 
 * @brief The base functionality that PTP communication is built on.
 * 
 * This file contains the CameraBase class, from which PTPCamera and CHDKCamera
 * are extended.  CameraBase is designed to handle all communication with libusb
 * and with setting up communication with the camera, so that the Camera classes
 * can just talk to the camera using the correct protocol.
 */
 
#include <algorithm>
#include <stdint.h>

#include "libptp++.hpp"
#include "CameraBase.hpp"
#include "PTPContainer.hpp"

namespace PTP {
 
/**
 * Creates a new, empty \c CameraBase object.  Can then call
 * \c CameraBase::open to connect to a camera.
 */
CameraBase::CameraBase() {
    this->init();
}

/**
 * Creates a new \c CameraBase object and connects to the camera
 * described by \a dev.
 *
 * @param[in] dev  The \c libusb_device to connect to.
 * @exception PTP::ERR_NO_DEVICE thrown if \a dev is a NULL pointer.
 * @see CameraBase::find_first_camera
 * @see CameraBase::open
 */
CameraBase::CameraBase(libusb_device * dev) {
    this->init();
    
    if(dev == NULL) {
        throw PTP::ERR_NO_DEVICE;
    }
    
    this->open(dev);
}

/**
 * Destructor for a \c CameraBase object.  If connected to a camera, this
 * will release the interface, and close the handle.
 */
CameraBase::~CameraBase() {
    if(this->handle != NULL) {
        libusb_release_interface(this->handle, this->intf->bInterfaceNumber);
        libusb_close(this->handle);
        this->handle = NULL;
    }
}

/**
 * Initialize private and public \c CameraBase variables.
 */
void CameraBase::init() {
    this->handle = NULL;
    this->usb_error = 0;
    this->intf = NULL;
    this->ep_in = 0;
    this->ep_out = 0;
    this->_transaction_id = 0;
}

/**
 * Closes the opened camera object.
 * @return true if successful
 * @todo Check for errors in the calls
 */
bool CameraBase::close() {
    if(this->handle != NULL) {
        libusb_release_interface(this->handle, this->intf->bInterfaceNumber);
        libusb_close(this->handle);
        this->handle = NULL;
    }
    return true;
}

/**
 * Perform a \c libusb_bulk_transfer to the "out" endpoint of the connected camera.
 *
 * @warning Make sure \a bytestr is at least \a length bytes in length.
 * @param[in] bytestr Bytes to write through USB.
 * @param[in] length  Number of bytes to read from \a bytestr.
 * @param[in] timeout The maximum number of seconds to attempt to send for.
 * @return 0 on success, libusb error code otherwise.
 * @exception PTP::ERR_NOT_OPEN if not connected to a camera.
 * @see CameraBase::_bulk_read
 */
int CameraBase::_bulk_write(unsigned char * bytestr, int length, int timeout) {
    int transferred;
    
    if(this->handle == NULL) {
        throw PTP::ERR_NOT_OPEN;
        return 0;
    }
    
    // TODO: Return the amount of data transferred? Check it here? What should we do if not enough was sent?
    return libusb_bulk_transfer(this->handle, this->ep_out, bytestr, length, &transferred, timeout);
}

/**
 * Perform a \c libusb_bulk_transfer to the "in" endpoint of the connected camera.
 *
 * @warning Make sure \a data_out has enough memory allocated to read at least \a size bytes.
 * @param[out] data_out    The data read from the camera.
 * @param[in]  size        The number of bytes to attempt to read.
 * @param[out] transferred The number of bytes actually read.
 * @param[in]  timeout     The maximum number of seconds to attempt to read for.
 * @return 0 on success, libusb error code otherwise.
 * @exception PTP::ERR_NOT_OPEN if not connected to a camera.
 * @see CameraBase::_bulk_read
 */
int CameraBase::_bulk_read(unsigned char * data_out, int size, int * transferred, int timeout) {
    if(this->handle == NULL) {
        throw PTP::ERR_NOT_OPEN;
        return 0;
    }
    
    // TODO: Return the amount of data transferred? We might get less than we ask for, which means we need to tell the calling function?
    return libusb_bulk_transfer(this->handle, this->ep_in, data_out, size, transferred, timeout);
}

/**
 * Send the data contained in \a cmd to the connected camera.
 *
 * @param[in] cmd The \c PTPContainer containing the command/data to send.
 * @param[in] timeout The maximum number of seconds to attempt to send for.
 * @return 0 on success, libusb error code otherwise.
 * @see CameraBase::_bulk_write, CameraBase::recv_ptp_message
 */
int CameraBase::send_ptp_message(PTPContainer * cmd, int timeout) {
    unsigned char * packed = cmd->pack();
    int ret = this->_bulk_write(packed, cmd->get_length(), timeout);
    delete[] packed;
    
    return ret;
}

/**
 * @brief Recives a \c PTPContainer from the camera and returns it.
 *
 * This function works by first reading in a buffer of 512 bytes from the camera
 * to determine the length of the PTP message it will receive.  If necessary, it
 * then makes another \c CameraBase::_bulk_read call to read in the rest of the
 * data.  Finally, \c PTPContainer::unpack is called to place the data in \a out.
 *
 * @warning \a timeout is passed to each call to \c CameraBase::_bulk_read.  Therefore,
 *          this function could take up to 2 * \a timeout seconds to return.
 *
 * @param[out] out A pointer to a PTPContainer that will store the read PTP message.
 * @param[in]  timeout The maximum number of seconds to wait to read each time.
 * @see CameraBase::_bulk_read, CameraBase::send_ptp_message
 */
void CameraBase::recv_ptp_message(PTPContainer *out, int timeout) {
    // Determine size we need to read
	unsigned char * buffer = new unsigned char[512];
    int read = 0;
    this->_bulk_read(buffer, 512, &read, timeout); // TODO: Error checking on response
    uint32_t size = 0;
    if(read < 4) {
        // If we actually read less than four bytes, we can't copy four bytes out of the buffer.
        // Also, something went very, very wrong
        throw PTP::ERR_CANNOT_RECV;
        return;
    }
	std::copy(buffer, buffer + 4, &size);   // The first four bytes of the buffer are the size
    
    // Copy our first part into the output buffer -- so we can reuse buffer
    unsigned char * out_buf = new unsigned char[size];
    if(size < 512) {
		std::copy(buffer, buffer + size, out_buf);
    } else {
		std::copy(buffer, buffer + 512, out_buf);
        // We've already read 512 bytes... read the rest!
        this->_bulk_read(&out_buf[512], size-512, &read, timeout);
    }
    
    if(out != NULL) {
        out->unpack(out_buf);
    }
    
    delete[] out_buf;
}

/**
 * @brief Perform a complete write, and optionally read, PTP transaction.
 * 
 * At minimum, it is required that \a cmd is not \c NULL.  All other containers
 * are checked for NULL values before reading/writing.  Note that this function
 * will also modify \a cmd and \a data to place a generated transaction ID in them,
 * required by the PTP protocol.
 *
 * Although not enforced by this function, \a cmd should be a \c PTPContainer containing
 * a command, and \a data (if given) should be a \c PTPContainer containing data.
 *
 * Note that even if \a receiving is false, PTP requires that we receive a response.
 * If provided, \a out_resp will be populated with the command response, even if
 * \a receiving is false.
 *
 * @warning \c CameraBase::_bulk_read and \c CameraBase::_bulk_write are called multiple
 *          times during the execution of this function, and \a timeout is passed to each
 *          of them individually.  Therefore, this function could take much more than
 *          \a timeout seconds to return.
 *
 * @param[in]  cmd       A \c PTPContainer containing the command to send to the camera.
 * @param[in]  data      (optional) A \c PTPContainer containing the data to be sent with the command.
 * @param[in]  receiving Whether or not to receive data in addition to a response from the camera.
 * @param[out] out_resp  (optional) A \c PTPContainer where the camera's response will be placed.
 * @param[out] out_data  (optional) A \c PTPContainer where the camera's data response will be placed.
 * @param[in]  timeout   The maximum number of seconds each \c CameraBase::_bulk_read or \c CameraBase::_bulk_write
 *                       should attempt to communicate for.
 * @see CameraBase::send_ptp_message, CameraBase::recv_ptp_message
 */
void CameraBase::ptp_transaction(PTPContainer *cmd, PTPContainer *data, bool receiving, PTPContainer * out_resp, PTPContainer * out_data, int timeout) {
    bool received_data = false;
    bool received_resp = false;

    cmd->transaction_id = this->get_and_increment_transaction_id();
    this->send_ptp_message(cmd, timeout);
    
    if(data != NULL) {
        data->transaction_id = cmd->transaction_id;
        this->send_ptp_message(data, timeout);
    }
    
    if(receiving) {
        PTPContainer out;
        this->recv_ptp_message(&out, timeout);
        if(out.type == PTPContainer::CONTAINER_TYPE_DATA) {
            received_data = true;
            // TODO: It occurs to me that pack() and unpack() might be inefficient. Let's try to find a better way to do this.
            if(out_data != NULL) {
                unsigned char * packed = out.pack();
                out_data->unpack(packed);
                delete[] packed;
            }
        } else if(out.type == PTPContainer::CONTAINER_TYPE_RESPONSE) {
            received_resp = true;
            if(out_resp != NULL) {
                unsigned char * packed = out.pack();
                out_resp->unpack(packed);
                delete[] packed;
            }
        }
    }
    
    if(!received_resp) {
        // Read it anyway!
        // TODO: We should return response AND data...
        this->recv_ptp_message(out_resp, timeout);
    }
}

/**
 * @brief Opens the camera specified by \a dev.
 *
 * @todo This is one of the places where we expose the fact that PTP uses libusb as a backend.
 *       It would be nice if we could let the caller completely ignore the underlying protocol.
 * @param[in] dev The \c libusb_device which specifies which device to connect to.
 * @exception PTP::ERR_ALREADY_OPEN if this \c CameraBase already has an open device.
 * @exception PTP::ERR_CANNOT_CONNECT if we cannot connect to the camera specified.
 * @return true if we successfully connect, false otherwise.
 */
bool CameraBase::open(libusb_device * dev) {
    if(this->handle != NULL) {  // Handle will be non-null if the device is already open
        throw PTP::ERR_ALREADY_OPEN;
        return false;
    }

    int err = libusb_open(dev, &(this->handle));    // Open the device, placing the handle in this->handle
    if(err) {
        throw PTP::ERR_CANNOT_CONNECT;
        return false;
    }
    libusb_unref_device(dev);   // We needed this device refed before we opened it, so we added an extra ref. open adds another ref, so remove one ref
    
    struct libusb_config_descriptor * desc;
    int r = libusb_get_active_config_descriptor(dev, &desc);
    
    if (r < 0) {
        this->usb_error = r;
        return false;
    }
    
    int j, k;
    
    for(j = 0; j < desc->bNumInterfaces; j++) {
        struct libusb_interface interface = desc->interface[j];
        for(k = 0; k < interface.num_altsetting; k++) {
            struct libusb_interface_descriptor altsetting = interface.altsetting[k];
            if(altsetting.bInterfaceClass == 6) { // If this has the PTP interface
                this->intf = &altsetting;
                libusb_claim_interface(this->handle, this->intf->bInterfaceNumber); // Claim the interface -- Needs to be done before I/O operations
                break;
            }
        }
        if(this->intf) break;
    }
    
    
    const struct libusb_endpoint_descriptor * endpoint;
    for(j = 0; j < this->intf->bNumEndpoints; j++) {
        endpoint = &(this->intf->endpoint[j]);
        if((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK == LIBUSB_ENDPOINT_IN) &&
            (endpoint->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) == LIBUSB_TRANSFER_TYPE_BULK) {
            this->ep_in = endpoint->bEndpointAddress;
        } else if((endpoint->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) == LIBUSB_ENDPOINT_OUT) {
            this->ep_out = endpoint->bEndpointAddress;
        }
    }
    
    libusb_free_config_descriptor(desc);
    
    // If we haven't detected an error by now, assume that this worked.
    return true;
}

/**
 * @brief Find the first camera which is connected.
 *
 * Asks libusb for all the devices connected to the computer, and returns
 * the first PTP device it can find.  
 * 
 * @todo Exposes the fact that we actually use libusb. Hide this fact in the future.
 * @return A pointer to a \c libusb_device which represents the camera found, or NULL if none found.
 */
libusb_device * CameraBase::find_first_camera() {
    // discover devices
    libusb_device **list;
    libusb_device *found = NULL;
    ssize_t cnt = libusb_get_device_list(NULL, &list);
    ssize_t i = 0, j = 0, k = 0;
    int err = 0;
    if (cnt < 0) {
        return NULL;
    }

    for (i = 0; i < cnt; i++) {
        libusb_device *device = list[i];
        struct libusb_config_descriptor * desc;
        int r = libusb_get_active_config_descriptor(device, &desc);
        
        if (r < 0) {
            return NULL;
        }
        
        for(j = 0; j < desc->bNumInterfaces; j++) {
            struct libusb_interface interface = desc->interface[j];
            for(k = 0; k < interface.num_altsetting; k++) {
                struct libusb_interface_descriptor altsetting = interface.altsetting[k];
                if(altsetting.bInterfaceClass == 6) { // If this has the PTP interface
                    found = device;
                    break;
                }
            }
            if(found) break;
        }
        
        libusb_free_config_descriptor(desc);
        
        if(found) break;
    }
    
    if(found) {
        libusb_ref_device(found);     // Add a reference to the device so it doesn't get destroyed when we free_device_list
    }
    
    libusb_free_device_list(list, 1);   // Free the device list with dereferencing. Shouldn't delete our device, since we ref'd it
    
    return found;
}

/**
 * @brief Returns the last USB error we encountered.
 *
 * @return The \c libusb error we last received.
 */
int CameraBase::get_usb_error() {
    return this->usb_error;
}

/**
 * @brief Retrieves our current transaction ID and increments it
 *
 * @return The current transaction id (starting at 0)
 * @see CameraBase::ptp_transaction
 */
int CameraBase::get_and_increment_transaction_id() {
    uint32_t ret = this->_transaction_id;
    this->_transaction_id = this->_transaction_id + 1;
    return ret;
}

} /* namespace PTP */
