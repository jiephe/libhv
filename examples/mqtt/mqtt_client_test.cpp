/*
 * mqtt client
 *
 * @build   make examples
 *
 * @test    bin/mqtt_client_test 127.0.0.1 1883 topic payload
 *
 */

#include "mqtt_client.h"
using namespace hv;

/*
 * @test    MQTTS
 * #define  TEST_SSL 1
 *
 * @build   ./configure --with-mqtt --with-openssl && make clean && make
 *
 */
#define TEST_SSL        0
#define TEST_AUTH       0
#define TEST_RECONNECT  1
#define TEST_QOS        0

#include "hv.h"
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/prctl.h>

int main(int argc, char** argv) {
    const char* host = argv[1];
    int port = atoi(argv[2]);

    // client_id
    char client_id[64];
    snprintf(client_id, sizeof(client_id), "mqtt_pub_%ld", hv_getpid());

    MqttClient cli;
    cli.setID(client_id);

#if 0
    mqtt_message_t will;
    memset(&will, 0, sizeof(will));
    will.topic = "pm_ai_offline_topic";
    will.payload = "This is a will.";
    cli.setWill(&will);
#endif

    pid_t mid = static_cast<pid_t>(::syscall(SYS_gettid));

    cli.onConnect = [mid](MqttClient* cli) {

        assert(mid == static_cast<pid_t>(::syscall(SYS_gettid)));
        printf("************ connected!\n");
        cli->subscribe("$SYS/brokers/+/clients/+/connected");
        cli->subscribe("$SYS/brokers/+/clients/+/disconnected");
        cli->subscribe("$queue/t/111");
        cli->subscribe("$queue/t/222");
        cli->subscribe("$queue/t/333");
        cli->subscribe("$queue/t/555");
    };

    cli.onMessage = [](MqttClient* cli, mqtt_message_t* msg) {
        printf("*********topic: %.*s\n", msg->topic_len, msg->topic);
        printf("**********payload: %.*s\n", msg->payload_len, msg->payload);
    };

    cli.onClose = [](MqttClient* cli) {
        printf("************* disconnected!\n");
    };

#if TEST_AUTH
    cli.setAuth("test", "123456");
#endif

#if TEST_RECONNECT
    reconn_setting_t reconn;
    reconn_setting_init(&reconn);
    reconn.min_delay = 1000;
    reconn.max_delay = 10000;
    reconn.delay_policy = 2;
    cli.setReconnect(&reconn);
#endif

    int ssl = 0;
#if TEST_SSL
    ssl = 1;
#endif
    cli.connect(host, port, ssl);

#if 0
    std::thread t1([&cli]{
        hv_msleep(1500);
        int index = 0;
        while (true) {
            std::string sindex = std::string("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") + std::to_string(index++);
            cli.publish("t/111", sindex.c_str());
            hv_msleep(10);
        }
    });

    std::thread t2([&cli]{
        hv_msleep(1500);
        int index = 0;
        while (true) {
            std::string sindex = std::string("bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb") + std::to_string(index++);
            cli.publish("t/222", sindex.c_str());
            hv_msleep(10);
        }
    });

    std::thread t3([&cli]{
        hv_msleep(1500);
        int index = 0;
        while (true) {
            std::string sindex = std::string("cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc") + std::to_string(index++);
            cli.publish("t/333", sindex.c_str());
            hv_msleep(10);
        }
    });

    std::thread t5([&cli]{
        hv_msleep(1500);
        int index = 0;
        while (true) {
            std::string sindex = std::string("eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee") + std::to_string(index++);
            cli.publish("t/555", sindex.c_str());
            hv_msleep(10);
        }
    });
    #endif

    cli.run();
    return 0;
}
