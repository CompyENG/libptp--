#ifndef LIBPTP_PP_PTPCONTAINER_H_
#define LIBPTP_PP_PTPCONTAINER_H_

namespace PTP {

    class PTPContainer {
        private:
            static const uint32_t default_length = sizeof(uint32_t)+sizeof(uint32_t)+sizeof(uint16_t)+sizeof(uint16_t);
            uint32_t length;
            unsigned char * payload;    // We'll deal with this completely internally
            void init();
        public:
            enum CONTAINER_TYPE {
                CONTAINER_TYPE_COMMAND  = 1,
                CONTAINER_TYPE_DATA     = 2,
                CONTAINER_TYPE_RESPONSE = 3,
                CONTAINER_TYPE_EVENT    = 4
            };
            
            uint16_t type;
            uint16_t code;
            uint32_t transaction_id;    // We'll end up setting this externally
            PTPContainer();
            PTPContainer(uint16_t type, uint16_t op_code);
            PTPContainer(unsigned char * data);
            ~PTPContainer();
            void add_param(uint32_t param);
            void set_payload(unsigned char * payload, int payload_length);
            unsigned char * pack();
            unsigned char * get_payload(int * size_out);  // This might end up being useful...
            uint32_t get_length();  // So we can get, but not set
            void unpack(unsigned char * data);
            uint32_t get_param_n(uint32_t n);
    };
    
}

#endif /* LIBPTP_PP_PTPCONTAINER_H_ */
