#include "HTTPhandlers.h"
#include "globalVars.h"

#define SAMPLE_TIME     1000 // milli-sec
#define COMPLETED_FLAG (1UL << 0)
EventFlags threadFlag;

PlatformMutex stdio_mutex;

void print_stats()
{
    mbed_stats_socket_t stats[MBED_CONF_NSAPI_SOCKET_STATS_MAX_COUNT];
    static int num = 0;
    int count;

    memset(stats, 0, sizeof(mbed_stats_socket_t) * MBED_CONF_NSAPI_SOCKET_STATS_MAX_COUNT);
    printf("%-15s%-15s%-15s%-15s%-15s%-15s%-15s\n", "Num", "ID", "State", "Proto", "Sent", "Recv", "Time");
    //while (COMPLETED_FLAG != threadFlag.get()) {
        count = SocketStats::mbed_stats_socket_get_each(&stats[0], MBED_CONF_NSAPI_SOCKET_STATS_MAX_COUNT);
        for (int i = 0; i < count; i++) {
            stdio_mutex.lock();
            printf("\n%-15d", num);
            printf("%-15p", stats[i].reference_id);

            switch (stats[i].state) {
                case SOCK_CLOSED:
                    printf("%-15s", "Closed");
                    break;
                case SOCK_OPEN:
                    printf("%-15s", "Open");
                    break;
                case SOCK_CONNECTED:
                    printf("%-15s", "Connected");
                    break;
                case SOCK_LISTEN:
                    printf("%-15s", "Listen");
                    break;
                default:
                    printf("%-15s", "Error");
                    break;
            }

            if (NSAPI_TCP == stats[i].proto) {
                printf("%-15s", "TCP");
            } else {
                printf("%-15s", "UDP");
            }
            printf("%-15d", stats[i].sent_bytes);
            printf("%-15d", stats[i].recv_bytes);
            printf("%-15lld\n", stats[i].last_change_tick);
            stdio_mutex.unlock();
        }
        num++;
        //ThisThread::sleep_for(SAMPLE_TIME);
    //}
    // Now allow the stats thread to simply exit by itself gracefully.
}

Mutex mutexReqHandlerRoot;

// Requests come in here
void request_handler(HttpParsedRequest* request, ClientConnection* clientConnection) {
    mutexReqHandlerRoot.lock();
    HttpResponseBuilder builder(200, clientConnection);

    if (request->get_method() == HTTP_GET && request->get_url() == "/format") {
        builder.headers["Content-Type"] = "text/html; charset=utf-8";
        builder.sendHeader();

        string body = 
            "<html><head><title>Hello from mbed</title></head>"
            "<body>"
                "<h1>mbed webserver</h1>"
                "<button id=\"toggle\">Format Flash with LittleFS</button>"
                "<script>document.querySelector('#toggle').onclick = function() {"
                    "var x = new XMLHttpRequest(); x.open('POST', '/toggle'); x.send();"
                "}</script>"
            "</body></html>";

        builder.sendBodyString(body);
    } else if(request->get_method() == HTTP_GET && request->get_url() == "/") {
        builder.sendHeaderAndFile(&fs, "/index.html");
    } else if(request->get_method() == HTTP_GET && request->get_url() == "/test.svg") {
        builder.headers["Content-Type"] = "image/svg+xml";
        string out;
        out.reserve(2048);
        char temp[100];

        out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
        out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
        out += "<g stroke=\"black\">\n";
        int y = rand() % 130;
        for (int x = 10; x < 390; x+= 10) {
            int y2 = rand() % 130;
            sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
            out += temp;
            y = y2;
        }
        out += "</g>\n</svg>\n";
        builder.sendHeader(200);
        builder.sendBodyString(out);
    } else if(request->get_method() == HTTP_GET) {
        builder.sendHeaderAndFile(&fs, request->get_url());
    } else if (request->get_method() == HTTP_POST && request->get_url() == "/toggle") {
        printf("toggle LED called\n\n");
        //led = !led;
        builder.sendHeader();
    } else {
        builder.sendHeader(404);
    }
    mutexReqHandlerRoot.unlock();
}

Mutex mutexReqHandlerStatus;
const char* Compilername[] = {
    "ARM",
    "GCC_ARM",
    "IAR"
};

string getCPUIDame(uint32_t cpu_id) 
{
    uint16_t rev = cpu_id & 0xf;
    cpu_id = (cpu_id >> 4) & 0xfff;

    string id;
    id.reserve(32);

    switch (cpu_id) {
        case 0xC20:  
            id = "Cortex-M0";
            break;
        case 0xC60: 
            id = "Cortex-M0+";
            break;
        case 0xC23:
            id = "Cortex-M3";
            break;
        case 0xC24:
            id = "Cortex-M4";
            break;
        case 0xC27:
            id = "Cortex-M7";
            break;
        case 0xD20:
            id = "Cortex-M23";
            break;
        case 0xD21:
            id = "Cortex-M33";
            break;
        default:
            id = "unknown";
    }
    id += " Rev: " + to_string(rev);

    return id;    
}

const char* StateNames[] {
        "Inactive",           /**< NOT USED */
        "Ready",              /**< Ready to run */
        "Running",            /**< Running */
        "WaitingDelay",       /**< Waiting for a delay to occur */
        "WaitingJoin",        /**< Waiting for thread to join. Only happens when using RTX directly. */
        "WaitingThreadFlag",  /**< Waiting for a thread flag to be set */
        "WaitingEventFlag",   /**< Waiting for a event flag to be set */
        "WaitingMutex",       /**< Waiting for a mutex event to occur */
        "WaitingSemaphore",   /**< Waiting for a semaphore event to occur */
        "WaitingMemoryPool",  /**< Waiting for a memory pool */
        "WaitingMessageGet",  /**< Waiting for message to arrive */
        "WaitingMessagePut",  /**< Waiting for message to be send */
        "WaitingInterval",    /**< NOT USED */
        "WaitingOr",          /**< NOT USED */
        "WaitingAnd",         /**< NOT USED */
        "WaitingMailbox",     /**< NOT USED (Mail is implemented as MemoryPool and Queue) */

        /* Not in sync with RTX below here */
        "Deleted",            /**< The task has been deleted or not started */
};

void request_handler_getStatus(HttpParsedRequest* request, ClientConnection* clientConnection) 
{
    mutexReqHandlerStatus.lock();
    HttpResponseBuilder builder(200, clientConnection);
    builder.headers["Content-Type"] = "application/json; charset=utf-8";
    builder.sendHeader();

    string body;
    body.reserve(1024);

    if (request->get_method() == HTTP_GET) {
        if (request->get_filename() == "mem") {
            mbed_stats_heap_t heap_info;
            mbed_stats_heap_get( &heap_info );

            body += "{\"current_size\": ";
            body += to_string(heap_info.current_size);
            body += ", \"max_size\": ";
            body += to_string(heap_info.max_size);
            body += ", \"alloc_cnt\": ";
            body += to_string(heap_info.alloc_cnt);
            body += ", \"reserved_size\": ";
            body += to_string(heap_info.reserved_size);
            body += "}";

            builder.sendBodyString(body);
        } else
        if (request->get_filename() == "cpu") {
            mbed_stats_cpu_t stats;
            mbed_stats_cpu_get(&stats);

            body += "{\"uptime\": ";
            body += to_string(stats.uptime / 1000000);
            body += ", \"idle_time\": ";
            body += to_string(stats.idle_time / 1000000);
            body += ", \"sleep_time\": ";
            body += to_string(stats.sleep_time / 1000000);
            body += ", \"deep_sleep_time\": ";
            body += to_string(stats.deep_sleep_time / 1000000);
            body += "}";

            builder.sendBodyString(body);
        } else
        if (request->get_filename() == "sysinfo") {
            mbed_stats_sys_t stats;
            mbed_stats_sys_get(&stats);

            body += "{\"MBed OS Version\": ";
            body += "\"" + to_string(MBED_MAJOR_VERSION) + "." + to_string(MBED_MINOR_VERSION) + "." + to_string(MBED_PATCH_VERSION) + "\"";
            body += ", \"CPU Id\": ";
            body += "\"" + getCPUIDame(stats.cpu_id) + "\"";
            if ((stats.compiler_id > 1) && (stats.compiler_id < 3))
            body += ", \"Compiler Id\": \"";
            body += Compilername[stats.compiler_id - 1];
            body += "\", \"Compiler Version\": ";
            body += to_string(stats.compiler_version);
            body += "}";

            builder.sendBodyString(body);
        } else
        if (request->get_filename() == "threads") {
            const uint MAX_THREAD_STATS = 32;
            mbed_stats_thread_t *stats = new mbed_stats_thread_t[MAX_THREAD_STATS];
            int count = mbed_stats_thread_get_each(stats, MAX_THREAD_STATS);
    
            body += "[[\"ID\",\"Name\",\"State\",\"Priority\",\"Stack Size\",\"Stack Space\"],";

            for(int i = 0; i < count; i++) {
                body += "[";
                body += to_string(stats[i].id) + ",";
                body += "\"" + string(stats[i].name) + "\",";
                body += "\"" + string(StateNames[stats[i].state]) + "\",";
                body += to_string(stats[i].priority) + ",";
                body += to_string(stats[i].stack_size) + ",";
                body += to_string(stats[i].stack_space);
                if (i < count-1)
                    body += "],";
                else
                    body += "]]";
            }
            
            delete stats;

            builder.sendBodyString(body);
        } else
        if (request->get_filename() == "test") {

            body += "{\"test\": 42}";

            builder.sendBodyString(body);
        } else {
            builder.sendHeader(404);
        }
    }
    else {
        builder.sendHeader(404);
    }
    mutexReqHandlerStatus.unlock();
}
