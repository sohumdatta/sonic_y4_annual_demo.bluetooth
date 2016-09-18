/* collection of setup files used by the collection C routine */

/* file including path */
const char OUTPUT_FILE[] =  "/home/abbas/sonic_y4_annual_demo/run/latest_emg_data.txt";

const char SHARED_RESOURCE[] =  "/dummy_shared";

/* structure to save the filtered data with the time stamps */
struct filtered_data{
    long sec_elapsed;
    int ms_elapsed;
    int us_elapsed;
    double channels[4];
};

struct shared {
    pthread_mutex_t mutex;  /* access mutex to the data item */
    struct filtered_data filteredData;
};
