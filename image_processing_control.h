#ifndef __image_processing_control__
#define __image_processing_control__

// Default values for FilterImages::ApplyFilters image processing.
// These may be modified at the command line.
#define __THRESHOLD_METHOD__              1     // 0 = fixed, 1 = adaptive
#define __FIXED_THRESH_TOO_BRIGHT_ELIM__  253
#define __FIXED_THRESH_TOO_DIM_ELIM__     230
#define __ADAPTIVE_THRESH_TOO_DIM_ELIM__  230
#define __ADAPTIVE_THRESH_METHOD__        0     // 0 = mean, 1 = gaussian
#define __ADAPTIVE_THRESH_BOX_DIM__       31

#endif // __image_processing_control__
