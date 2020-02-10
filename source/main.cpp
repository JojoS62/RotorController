#include "mbed.h"

#include "globalVars.h"

#include "NTPClient.h"

//#include "network-helper.h"
#include "HttpServer.h"
#include "HttpResponseBuilder.h"
#include "WebsocketHandlers.h"
#include "HTTPHandlers.h"

#include "threadIO.h"

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

// enable app features
#define USE_NTPCLIENT
#define USE_HTTPSERVER
#define USE_TFTPSERVER
//#define USE_MQTT

#define DEFAULT_STACK_SIZE (4096)


#ifdef USE_TFTPSERVER
#include "threadTFTPServer.h"
ThreadTFTPServer  threadTFTPpServer;
#endif

ThreadIO threadIO(50);

#ifdef USE_MQTT
#include "MQTTThreadedClient.h"
Thread msgSender(osPriorityNormal, DEFAULT_STACK_SIZE * 3);
using namespace MQTT;
/*  
    MQTT
*/
static const char * clientID = "mbed-sample";
//static const char * userID = "";
//static const char * password = "";
static const char * topic_1 = "mbed-sample";
static const char * topic_2 = "test";

int arrivedcount = 0;

void messageArrived(MessageData& md)
{
    Message &message = md.message;
    printf("Arrived Callback 1 : qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
    printf("Payload [%.*s]\n", message.payloadlen, (char*)message.payload);
    ++arrivedcount;
}

class CallbackTest
{
    public:
    
    CallbackTest()
        : arrivedcount(0)
    {}
    
    void messageArrived(MessageData& md)
    {
        Message &message = md.message;
        printf("Arrived Callback 2 : qos %d, retained %d, dup %d, packetid %d\n", message.qos, message.retained, message.dup, message.id);
        printf("Payload [%.*s]\n", message.payloadlen, (char*)message.payload);
        ++arrivedcount;
    }
    
    private:
    
    int arrivedcount;
};
#endif

int main() {
    printf("Hello from "  TOSTRING(TARGET_NAME) "\n");
    printf("Mbed OS version: %d.%d.%d\n\n", MBED_MAJOR_VERSION, MBED_MINOR_VERSION, MBED_PATCH_VERSION);

#ifdef COMPONENT_SPIF
    print_SPIF_info();
    printf("\n");
#endif

#if 0
    printf("dir lfs:\n");
    print_dir(&lfs, "/");
    printf("\n");
#endif 

    printf("dir fs:\n");
    print_dir(&fs, "/");
    printf("\n");

    // IO Thread
    threadIO.start();
    
    // Connect to the network with the default networking interface
    // if you use WiFi: see mbed_app.json for the credentials
    NetworkInterface* network = NetworkInterface::get_default_instance();
    if (!network) {
        printf("Cannot connect to the network, see serial output\n");
        return 1;
    } 
    nsapi_error_t connect_status = network->connect();

    if (connect_status != NSAPI_ERROR_OK) {
        printf("Failed to connect to network (%d)\n", connect_status);
        return 2;
    } 	

#ifdef USE_NTPCLIENT
    NTPClient ntp(network);
    time_t timestamp = ntp.get_timestamp();
    set_time(timestamp);
    
    if (timestamp < 0) {
        printf("An error occurred when getting the time. Code: %lld\r\n", timestamp);
    } else {
        printf("Current time is %s\r\n", ctime(&timestamp));
    }
#endif

#ifdef USE_HTTPSERVER	
    HttpServer server(network, 5, 4);               // max 5 threads, 4 websockets

    server.addStandardHeader("Server", "JojoS_Mbed_Server");
    server.addStandardHeader("DNT", "1");

    server.setHTTPHandler("/", &request_handler);
    server.setHTTPHandler("/stats/", &request_handler_getStatus);
    
    server.setWSHandler("/ws/", WSHandler::createHandler);

    nsapi_error_t res = server.start(8080);

    if (res == NSAPI_ERROR_OK) {
        SocketAddress socketAddress;
        network->get_ip_address(&socketAddress);
        printf("Server is listening at http://%s:8080\n", socketAddress.get_ip_address());
    }
    else {
        printf("Server could not be started... %d\n", res);
    }
#endif

#ifdef USE_TFTPSERVER
    threadTFTPpServer.start(network);
#endif

#ifdef USE_MQTT
    float version = 0.6;
    CallbackTest testcb;

    printf("HelloMQTT: version is %.2f\n", version);
    MQTTThreadedClient mqtt(network);


    //const char* hostname = "jojosRPi3-1";
    const char* hostname = "192.168.100.28";
    int port = 1883;

    MQTTPacket_connectData logindata = MQTTPacket_connectData_initializer;
    logindata.MQTTVersion = 3;
    logindata.clientID.cstring = (char *) clientID;
    //logindata.username.cstring = (char *) userID;
    //logindata.password.cstring = (char *) password;
    
    mqtt.setConnectionParameters(hostname, port, logindata);
    mqtt.addTopicHandler(topic_1, messageArrived);
    mqtt.addTopicHandler(topic_2, &testcb, &CallbackTest::messageArrived);

    // Start the data producer
    msgSender.start(mbed::callback(&mqtt, &MQTTThreadedClient::startListener));
    
    int i = 0;
    while(true)
    {
        PubMessage message;
        message.qos = QOS0;
        message.id = 123;
        
        strcpy(&message.topic[0], topic_1);
        sprintf(&message.payload[0], "Testing %d", i);
        message.payloadlen = strlen((const char *) &message.payload[0]);
        mqtt.publish(message);
        
        i++;
        //TODO: Nothing here yet ...
        ThisThread::sleep_for(10000);
    }
#endif

    while(true) {
        ThisThread::sleep_for(10000);
    }
}
