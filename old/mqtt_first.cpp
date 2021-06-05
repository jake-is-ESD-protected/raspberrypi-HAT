
//compile with: g++ mqtt_first.cpp -o mqtt_first -lmosquittopp
//./mqtt_first
#include "mosquittopp.h"
#include <string.h>
#include <iostream>

#include <mosquittopp.h>
#include <cstring>
#include <cstdio>

#define MAX_PAYLOAD 50
#define DEFAULT_KEEP_ALIVE 60
#define CLIENT_ID "Client_ID"
#define BROKER_ADDRESS "192.168.2.141"
#define MQTT_PORT 1883;
#define MQTT_TOPIC "EXAMPLE_TOPIC"
#define PUBLISH_TOPIC "EXAMPLE_TOPIC"
#ifdef DEBUG
    #include <iostream>
#endif

class mqtt_client : public mosqpp::mosquittopp
{
public:
    mqtt_client (const char *id, const char *host, int port);
    ~mqtt_client();

    void on_connect(int rc);
    void on_message(const struct mosquitto_message *message);
    void on_subscribe(int mid, int qos_count, const int *granted_qos);
};


mqtt_client::mqtt_client(const char *id, const char *host, int port) : mosquittopp(id)
{
    int keepalive = DEFAULT_KEEP_ALIVE;
    connect(host, port, keepalive);
}

mqtt_client::~mqtt_client()
{
}

void mqtt_client::on_connect(int rc)
{
    if (!rc)
    {
        #ifdef DEBUG
            std::cout << "Connected - code " << rc << std::endl;
        #endif
    }
}

void mqtt_client::on_subscribe(int mid, int qos_count, const int *granted_qos)
{
    #ifdef DEBUG
        std::cout << "Subscription succeeded." << std::endl;
    #endif
}

void mqtt_client::on_message(const struct mosquitto_message *message)
{
    int payload_size = MAX_PAYLOAD + 1;
    char buf[payload_size];

    if(!strcmp(message->topic, PUBLISH_TOPIC))
    {
        memset(buf, 0, payload_size * sizeof(char));

        /* Copy N-1 bytes to ensure always 0 terminated. */
        memcpy(buf, message->payload, MAX_PAYLOAD * sizeof(char));

        #ifdef DEBUG
            std::cout << buf << std::endl;
        #endif

        // Examples of messages for M2M communications...
        if (!strcmp(buf, "STATUS"))
        {
            snprintf(buf, payload_size, "This is a Status Message...");
            publish(NULL, PUBLISH_TOPIC, strlen(buf), buf);
            #ifdef DEBUG
                std::cout << "Status Request Recieved." << std::endl;
            #endif
        }

        if (!strcmp(buf, "ON"))
        {
            snprintf(buf, payload_size, "Turning on...");
            publish(NULL, PUBLISH_TOPIC, strlen(buf), buf);
            #ifdef DEBUG
                std::cout << "Request to turn on." << std::endl;
            #endif
        }

        if (!strcmp(buf, "OFF"))
        {
            snprintf(buf, payload_size, "Turning off...");
            publish(NULL, PUBLISH_TOPIC, strlen(buf), buf);
            #ifdef DEBUG
                std::cout << "Request to turn off." << std:: endl;
            #endif
        }
    }
}

int main(int argc, char *argv[])
{
    class mqtt_client *iot_client;
    int rc;

    char client_id[] = CLIENT_ID;
    char host[] = BROKER_ADDRESS;
    int port = MQTT_PORT;

    mosqpp::lib_init();

    if (argc > 1)
        strcpy (host, argv[1]);

    iot_client = new mqtt_client(client_id, host, port);

    while(1)
    {
        rc = iot_client->loop();
        if (rc)
        {
            iot_client->reconnect();
        }
        else
            iot_client->subscribe(NULL, MQTT_TOPIC);
    }

    mosqpp::lib_cleanup();

    return 0;
}