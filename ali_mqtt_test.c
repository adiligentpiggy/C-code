#include "MQTTAsync.h"

#include <signal.h>
#include <memory.h>
#include <stdlib.h>
#include <unistd.h>
#include <openssl/hmac.h>
#include <openssl/bio.h>

volatile int connected = 0;
char *topic;
char *userName;
char *passWord;

int messageDeliveryComplete(void *context, MQTTAsync_token token) {
    /* not expecting any messages */
    printf("send message %d success\n", token);
    return 1;
}

int messageArrived(void *context, char *topicName, int topicLen, MQTTAsync_message *m) {
    /* not expecting any messages */
    printf("recv message from %s ,body is %s\n", topicName, (char *) m->payload);
    return 1;
}

void onConnectFailure(void *context, MQTTAsync_failureData *response) {
    connected = 0;
    printf("connect failed, rc %d\n", response ? response->code : -1);
    MQTTAsync client = (MQTTAsync) context;
}

void onSubcribe(void *context, MQTTAsync_successData *response) {
    printf("subscribe success \n");
}

void onConnect(void *context, MQTTAsync_successData *response) {
    connected = 1;
    printf("connect success \n");
    MQTTAsync client = (MQTTAsync) context;
    //do sub when connect success
    MQTTAsync_responseOptions sub_opts = MQTTAsync_responseOptions_initializer;
    sub_opts.onSuccess = onSubcribe;
    int rc = 0;
    if ((rc = MQTTAsync_subscribe(client, topic, 1, &sub_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to subscribe, return code %d\n", rc);
    }
}

void onDisconnect(void *context, MQTTAsync_successData *response) {
    connected = 0;
    printf("connect lost \n");
}

void onPublishFailure(void *context, MQTTAsync_failureData *response) {
    printf("Publish failed, rc %d\n", response ? -1 : response->code);
}

int success = 0;

void onPublish(void *context, MQTTAsync_successData *response) {
    printf("send success %d\n", ++success);
}


void connectionLost(void *context, char *cause) {
    connected = 0;
    MQTTAsync client = (MQTTAsync) context;
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    int rc = 0;

    printf("Connecting\n");
    conn_opts.keepAliveInterval = 10;
    conn_opts.cleansession = 1;
    conn_opts.username = userName;
    conn_opts.password = passWord;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    conn_opts.ssl = NULL;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
}


int main(int argc, char **argv) {
    MQTTAsync_disconnectOptions disc_opts = MQTTAsync_disconnectOptions_initializer;
    MQTTAsync client;
    //MQTT 使用的 Topic，其中第一级父 Topic 需要在 MQ 控制台先创建
    topic = "YTP_PIS_TOPIC_SO_INF/p2p/GID_PIPD@@@/1234567890000";
      //MQTT 的接入域名，在 MQ 控制台购买付费实例后即分配该域名
    char *host = "tcp://10.54.18.158";
    //MQTT 客户端分组 Id，需要先在 MQ 控制台创建
    char *groupId = "GID_PIPD";
    //MQTT 客户端设备 Id，用户自行生成，需要保证对于所有 TCP 连接全局唯一
    char *deviceId = "00002";
    //帐号 AK
    char *accessKey = "0Ks8XmJ0g4HDfiXR";
    //帐号 SK
    char *secretKey = "pL7NA2ncEmD3HZ6PTODqnSBkMwc3NA";
    //服务端口，使用 MQTT 协议时设置1883,其他协议参考文档选择合适端口
    int port = 1883;
    //QoS，消息传输级别，参考文档选择合适的值
    int qos = 1;
    //cleanSession，是否设置持久会话，如果需要离线消息则 cleanSession 必须是 false，QoS 必须是1
    int cleanSession = 0;
    int rc = 0;
    char tempData[100];
    int len = 0;
    HMAC(EVP_sha1(), secretKey, strlen(secretKey), groupId, strlen(groupId), tempData, &len);
    char resultData[100];
    int passWordLen = EVP_EncodeBlock((unsigned char *) resultData, tempData, len);
    resultData[passWordLen] = '\0';
    printf("passWord is %s", resultData);
    userName = accessKey;
    passWord = resultData;
    //1.create client
    MQTTAsync_createOptions create_opts = MQTTAsync_createOptions_initializer;
    create_opts.sendWhileDisconnected = 0;
    create_opts.maxBufferedMessages = 10;
    char url[100];
    sprintf(url, "%s:%d", host, port);
    char clientIdUrl[64];
    sprintf(clientIdUrl, "%s@@@%s", groupId, deviceId);
    rc = MQTTAsync_createWithOptions(&client, url, clientIdUrl, MQTTCLIENT_PERSISTENCE_NONE, NULL, &create_opts);
    rc = MQTTAsync_setCallbacks(client, client, connectionLost, messageArrived, NULL);
    //2.connect to server
    MQTTAsync_connectOptions conn_opts = MQTTAsync_connectOptions_initializer;
    conn_opts.keepAliveInterval = 90;
    conn_opts.cleansession = cleanSession;
    conn_opts.username = userName;
    conn_opts.password = passWord;
    conn_opts.onSuccess = onConnect;
    conn_opts.onFailure = onConnectFailure;
    conn_opts.context = client;
    conn_opts.ssl = NULL;
    conn_opts.automaticReconnect = 1;
    conn_opts.connectTimeout = 3;
    if ((rc = MQTTAsync_connect(client, &conn_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start connect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    //3.publish msg
    MQTTAsync_responseOptions pub_opts = MQTTAsync_responseOptions_initializer;
    pub_opts.onSuccess = onPublish;
    pub_opts.onFailure = onPublishFailure;
    for (int i = 0; i < 1000; i++) {
        do {
            char data[100];
            sprintf(data, "hello mqtt demo");
            rc = MQTTAsync_send(client, topic, strlen(data), data, qos, 0, &pub_opts);
            sleep(1);
        } while (rc != MQTTASYNC_SUCCESS);
    }
    sleep(1000);
    disc_opts.onSuccess = onDisconnect;
    if ((rc = MQTTAsync_disconnect(client, &disc_opts)) != MQTTASYNC_SUCCESS) {
        printf("Failed to start disconnect, return code %d\n", rc);
        exit(EXIT_FAILURE);
    }
    while (connected)
        sleep(1);
    MQTTAsync_destroy(&client);
    return EXIT_SUCCESS;
}
