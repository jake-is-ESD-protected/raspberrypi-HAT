#include <mosquittopp.h>
#include <string.h>
#include <iostream>
#include <unistd.h>

class myMosq : public mosqpp::mosquittopp
{
    private:
        const char* host;
        const char* id;
        const char* topic;
        int port;
        int keepalive;

        void on_connect(int rc);
        void on_disconnect(int rc);
        void on_publish(int mid);
    public:
        myMosq(const char *id, const char * _topic, const char *host, int port);
        ~myMosq();
        bool send_message(const char * _message);
};

myMosq::myMosq(const char * _id, const char * _topic, const char* _host, int _port) : mosquittopp(_id)
{
    mosqpp::lib_init();        // Mandatory initialization for mosquitto library
    this->keepalive = 60;    // Basic configuration setup for myMosq class
    this->id = _id;
    this->port = _port;
    this->host = _host;
    this->topic = _topic;
    connect_async(host, port, keepalive);
    loop_start();            // Start thread managing connection / publish / subscribe
};

myMosq::~myMosq() {
    
    loop_stop(true);            // Kill the thread
    mosqpp::lib_cleanup();    // Mosquitto library cleanup
    
}

bool myMosq::send_message(const  char * _message)
{
    int ret = publish(NULL, this->topic, strlen(_message), _message, 1, false);
    return ( ret == MOSQ_ERR_SUCCESS );
}

void myMosq::on_disconnect(int rc) {
    std::cout << ">> myMosq - disconnection(" << rc << ")" << std::endl;
}

void myMosq::on_connect(int rc)
{
    if ( rc == 0 ) {
        std::cout << ">> myMosq - connected with server" << std::endl;
    } else {
        std::cout << ">> myMosq - Impossible to connect with server(" << rc << ")" << std::endl;
    }
}

void myMosq::on_publish(int mid)
{
    std::cout << ">> myMosq - Message (" << mid << ") succeed to be published " << std::endl;
}

int main(void){

    myMosq* myPub = new myMosq("this_ID", "testTopic", "192.168.2.141", 1883);

    for(int i = 0; i < 10; i++){

        myPub->send_message("msg\n");
        usleep(1000000);
    }
    delete myPub;
}