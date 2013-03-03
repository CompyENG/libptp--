#ifndef LIBPTP_PP_LVDATA_H_
#define LIBPTP_PP_LVDATA_H_

namespace PTP {
#include "chdk/live_view.h"
    
    class PTPContainer; // Forward delcaration for this is enough
    
    class LVData {
        private:
            PTP::lv_data_header * vp_head;
            PTP::lv_framebuffer_desc * fb_desc;
            uint8_t * payload;
            void init();
            static uint8_t clip(int v);
            static void yuv_to_rgb(uint8_t **dest, uint8_t y, int8_t u, int8_t v);
            
        public:
            LVData();
            LVData(uint8_t * payload, int payload_size);
            ~LVData();
            void read(uint8_t * payload, int payload_size);
            void read(PTPContainer * container);    // Could this make life easier?
            uint8_t * get_rgb(int * out_size, int * out_width, int * out_height, bool skip=false);    // Some cameras don't require skip
            float get_lv_version();
    };
    
}

#endif /* LIBPTP_PP_LVDATA_H_ */
