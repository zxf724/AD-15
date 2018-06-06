
/**
 * *********************************************************************
 *             Copyright (c) 2016 temp. All Rights Reserved.
 * @file protocols.c
 * @version V1.0
 * @date 2016.4.1
 * @brief ��������������ͨѶЭ�麯���ļ�.
 *
 * *********************************************************************
 * @note
 *
 * *********************************************************************
 * @author ����
 * 2016.12.30 ���Ӷ���ʧ������
 */



/* Includes ------------------------------------------------------------------*/
#include "user_comm.h"


/* Private typedef -----------------------------------------------------------*/
typedef struct
{
    Qos qos;
    char const *topic;
    Arrived_t callback;
} SubScribe_t;

/* Private define ------------------------------------------------------------*/
#define mqttPar     (WorkParam.mqtt)

/* Private macros ------------------------------------------------------------*/

/* Private variables ---------------------------------------------------------*/
static osMessageQId MQTT_SendQId;

static uint8_t txBuffer[MQTT_TX_BUFF_SIZE], rxBuffer[MQTT_RX_BUFF_SIZE];

static uint16_t Publish_Fail = 0, Connect_Fail = 0;
static uint32_t tsPingOut = 0;

static MQTTClient mClient;
static Network mNetwork;
static MQTTPacket_connectData connectData = MQTTPacket_connectData_initializer;

static SubScribe_t Subs[MAX_MESSAGE_HANDLERS];
static uint8_t DataFlowCnt = 0;

static MQTT_TaskPollFun pollFun;
static MQTT_MsgDataFun sendfailFun;

/* Private function prototypes -----------------------------------------------*/
void MQTT_Conn_Task(void const *argument);
void messageArrived(MessageData *data);
static void Manager_MQTT(void);
static void MQTT_SendPoll(void);
static void MQTT_ReSubscribe(void);
static BOOL Connect_MQTT(void);
static BOOL Disconnect_MQTT(void);
static void mqtt_Console(int argc, char *argv[]);


/* Exported functions --------------------------------------------------------*/

/**
 * Э�鴦���ʼ��
 */
void MQTT_Conn_Init(void)
{

    osMessageQDef(MQTT_SendQ, MQTT_SEND_Q_SIZE, void *);
    MQTT_SendQId = osMessageCreate(osMessageQ(MQTT_SendQ), NULL);

    osThreadDef(mqtt_conn, MQTT_Conn_Task, MQTT_TASK_PRIO, 0, MQTT_TASK_STK_SIZE);
    osThreadCreate(osThread(mqtt_conn), NULL);

    CMD_ENT_DEF(mqtt, mqtt_Console);
    Cmd_AddEntrance(CMD_ENT(mqtt));

    DBG_LOG("Protocl init OK.");
}

/**
 * MQTT���ù��Ӻ���
 * @param poll    ������ѯ���Ӻ���
 * @param msgdata ���ݷ���ʧ�ܹ��Ӻ���
 */
void MQTT_SetHookFun(MQTT_TaskPollFun poll, MQTT_MsgDataFun sendfail)
{
    MCPU_ENTER_CRITICAL();
    pollFun = poll;
    sendfailFun = sendfail;
    MCPU_EXIT_CRITICAL();
}

/**
 * Э�鴦�������
 * @param argument
 */
void MQTT_Conn_Task(void const *argument)
{
    TWDT_DEF(mqttConTask, 60000);
    TWDT_ADD(mqttConTask);
    TWDT_CLEAR(mqttConTask);

    NetworkInit(&mNetwork);
    MQTTClientInit(&mClient, &mNetwork, MQTT_TIMEOUT_DEF, txBuffer, sizeof(txBuffer), rxBuffer, sizeof(rxBuffer));

    GPRS_SetOnOff(TRUE);
    osDelay(20000);
    TWDT_CLEAR(mqttConTask);
    Get_Service_IP();

    DBG_LOG("Protocl task start.");
    while (1) {
        TWDT_CLEAR(mqttConTask);

        Manager_MQTT();
        if (pollFun != NULL) {
            pollFun();
        }
        if (System_SockIsConnected(NULL, NULL) > 0) {
            MQTTYield(&mClient, 10);
            MQTT_SendPoll();
        }
        osDelay(2);
    }
}

/**
 * MQTT����״̬��ѯ,�����ӷ���0�������ӷ���MQTTʹ�õ�����·��
 */
BOOL MQTT_IsConnected(void)
{
    return (BOOL)mClient.isconnected;
}

/**
 * MQTT���ݴ���״̬��ѯ
 */
BOOL MQTT_IsDataFlow(void)
{
    return (BOOL)DataFlowCnt;
}

/**
 * �����ĵ���Ϣ
 * @param data ��Ϣ����
 */
void messageArrived(MessageData *data)
{
    int i = 0;

#if MQTT_DEBUG > 0

    DBG_LOG("Receive New message%.*s, payload:",
            data->topicName->lenstring.len,
            data->topicName->lenstring.data);
#if MQTT_DEBUG > 1
    CMD_HEX_Print(data->message->payload, data->message->payloadlen);
#endif
#endif

    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
        if (Subs[i].topic != NULL) {
            if (strncmp(Subs[i].topic, data->topicName->lenstring.data, data->topicName->lenstring.len) == 0
                && Subs[i].callback != NULL) {
                Subs[i].callback(data->message->payload, data->message->payloadlen);
            }
        }
    }
}

/**
 * Э�鴦����������
 * @param dat  ����ָ��
 * @param len  ���ݳ���
 */
int16_t MQTT_SendData(uint8_t *dat, uint16_t len)
{
    int16_t rc = -1;

    if (System_SockIsConnected(NULL, NULL) > 0 && System_SockIsLock() == FALSE) {
        /*��������ָʾ��*/
        DataFlowCnt += 1;

        rc = System_SockSend(dat, len);

#if MQTT_DEBUG > 1
        if (rc != 0) {
            DBG_LOG("MQTT send data, rc:%d", rc);
        }
#if MQTT_DEBUG > 2
        CMD_HEX_Print(dat, len);
#endif
#endif
    }
    return rc;
}

/**
 * Э�鴦���������
 * @param dat  ���ݶ�����ָ��
 * @param len  �������ĳ���
 * @return ����ʵ�ʶ����ĳ���
 */
int16_t MQTT_ReadData(uint8_t *dat, uint16_t len)
{
    int16_t rc = 0;

    if (System_SockIsLock() == FALSE) {
        rc = System_SockRecv(dat, len);

        if (rc > 0) {
#if MQTT_DEBUG > 1
            DBG_LOG("MQTT read data, len:%d", rc);
#if MQTT_DEBUG > 2
            CMD_HEX_Print(dat, len);
#endif
#endif
            DataFlowCnt += 1;
        }
    }
    return rc;
}

/**
 * ����MQTT��Ϣ
 * @param topic   ������������
 * @param qos     ��Ϣ���������ȼ�
 * @param payload ��Ϣ��
 * @param len     ��Ϣ�峤��
 * @return ���ط������
 */
BOOL Publish_MQTT(char const *topic, Qos qos, uint8_t *payload, uint16_t len)
{
    MQTTMessage *pmsg;
    osStatus res = osOK;
    uint8_t *pBuf = NULL, toplen;
    char *top = NULL;

    toplen = strlen(topic) + 5;
    pBuf = MMEMORY_ALLOC(len + sizeof(MQTTMessage) + toplen);
    if (pBuf != NULL) {
        memset(pBuf, 0, len + sizeof(MQTTMessage) + toplen);
        pmsg = (MQTTMessage *)pBuf;
        pmsg->qos = qos;
        pmsg->retained = 0;
        pmsg->payload = pBuf + sizeof(MQTTMessage);
        pmsg->payloadlen = len;

        memcpy(pmsg->payload, payload, len);

        top = (char *)pmsg->payload + pmsg->payloadlen;
        strcpy(top, topic);

        res = osMessagePut(MQTT_SendQId, (uint32_t)pBuf, 1000);
        if (res == osOK) {
            DBG_DBG("MQTT message Q put OK.");
            return TRUE;
        } else {
            DBG_WAR("MQTT message Q fault:%d", (int)res);
            MMEMORY_FREE(pBuf);
        }
    }
    return FALSE;
}

/**
 * ����MQTT��Ϣ������MQTT����֮ǰ���ô˺���
 * @param topic ���ĵ�������
 * @param qos   ��Ϣ���������ȼ�
 * @param fun   ������Ϣ�ص���ָ��
 * @return ���ض��Ľ��
 */
BOOL Subscribe_MQTT(char const *topic, Qos qos, Arrived_t fun)
{
    uint8_t i = 0;

    if (topic != NULL && fun != NULL) {
        for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
            if (Subs[i].topic == NULL) {
                Subs[i].topic = topic;
                Subs[i].qos = qos;
                Subs[i].callback = fun;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* Private function prototypes -----------------------------------------------*/

/**
 * MQTT���ӹ���
 */
static void Manager_MQTT(void)
{
    BOOL pingout = FALSE;
    static int8_t pathS = 0, flowcnt = 0;
    static uint32_t tsDataFlow = 0, tsConnect_Fail = 0;

    char *addr = NULL;
    int8_t path = 0;
    uint16_t port = 0;

    /*���紫���������������ָʾ��*/
    if (DataFlowCnt > 0 && flowcnt == 0) {
        flowcnt = 1;
        TS_INIT(tsDataFlow);
    }
    if (TS_IS_OVER(tsDataFlow, 100) && DataFlowCnt > 0) {
        DataFlowCnt = 0;
        flowcnt = 0;
    }

    /*�������ͱ仯���ߵ���ʱӦ�Ͽ�MQTT����������״̬�Ǽٵ�*/
    path = System_SockIsConnected(&addr, &port);
    if (pathS != path || addr != mqttPar.MQTT_Server || port != mqttPar.MQTT_Port) {
        pathS = path;
        if (mClient.isconnected) {
            MCPU_ENTER_CRITICAL();
            mClient.isconnected = 0;
            MCPU_EXIT_CRITICAL();
        }
    }

    /*����ʧ�ܻ�������Ӧ��ʱ�Ͽ�����*/
    if (mClient.ping_outstanding == 0) {
        tsPingOut = HAL_GetTick();
    } else if (HAL_GetTick() - tsPingOut >= (MQTT_TIMEOUT_DEF + 5000)) {
        pingout = TRUE;
    }
    if (Publish_Fail > 0 || pingout > 0) {
        portENTER_CRITICAL();
        Publish_Fail = 0;
        mClient.ping_outstanding = 0;
        portEXIT_CRITICAL();
        if (mClient.isconnected) {
            Disconnect_MQTT();
        }
    }

    /*��ʱ��������socket������ǩȨ*/
    if (TS_IS_OVER(tsConnect_Fail, CONNECT_FAIL_TIMEOUT * 1000)) {
        TS_INIT(tsConnect_Fail);
        if (path == 0) {
            Connect_Fail++;
        }
    }

    /*�������ʧ�ܺ����¼�Ȩ*/
    if (Connect_Fail >= CONNECT_FAIL_REAUTH) {
        if (mClient.isconnected) {
            Disconnect_MQTT();
        }
        Connect_Fail = 0;
        DBG_LOG("CONNECT_FAIL_REAUTH.");
        Get_Service_IP();
    }

    if (mClient.isconnected == 0) {
        /*socketδռ��ʱ���ӵ�������*/
        if (System_SockIsLock() == FALSE) {
            System_SetSocket(mqttPar.MQTT_Server, mqttPar.MQTT_Port);
        }
        /*��������*/
        if (path > 0 && addr == mqttPar.MQTT_Server && port == mqttPar.MQTT_Port) {
            /*������ʼ��*/
            connectData.MQTTVersion = 4;
            connectData.keepAliveInterval = MQTT_PING_INVT_DEF;
            connectData.clientID.cstring = WorkParam.mqtt.MQTT_ClientID;
            connectData.username.cstring = WorkParam.mqtt.MQTT_UserName;
            connectData.password.cstring = "123456";
            connectData.cleansession = 1;
            connectData.willFlag = 0;
            /*����MQTT*/
            if (Connect_MQTT()) {
                DBG_LOG("System socket path:%d.", path);
                DBG_LOG("MQTT-clientId:%s, username:%s", mqttPar.MQTT_ClientID, mqttPar.MQTT_UserName);
                DBG_LOG("MQTT Ping invter %d s", MQTT_PING_INVT_DEF / 2);
                Connect_Fail = 0;
                MQTT_ReSubscribe();
            } else {
                Connect_Fail++;
            }
        }
    }
}

/**
 * MQTT������ѯ
 * @param id   MQTT��ID
 * @return �������ӽ���
 */
static void MQTT_SendPoll(void)
{
    int rc = 0, fail = 0;
    char *top = NULL;
    osEvent evt;
    MQTTMessage *pmsg;

    rc = rc;
    evt = osMessageGet(MQTT_SendQId, 2);
    if (evt.status == osEventMessage) {
        pmsg = evt.value.p;

        if (pmsg->payloadlen > 0) {
            top = (char *)pmsg->payload + pmsg->payloadlen;

            if (MQTT_IsConnected()) {
                if ((rc = MQTTPublish(&mClient, top, pmsg)) != SUCESS) {
                    Publish_Fail++;
                    fail++;
                } else {
                    Publish_Fail = 0;
                }
#if MQTT_DEBUG > 0
                DBG_LOG("MQTT publish top:%s, return:%d", top, rc);
#if MQTT_DEBUG > 1
                CMD_HEX_Print(pmsg->payload, pmsg->payloadlen);
#endif
#endif
            } else {
                fail++;
#if MQTT_DEBUG > 0
                DBG_LOG("MQTT publish fail, MQTT not connect.");
#endif
            }
            if (fail > 0) {
                if (sendfailFun != NULL) {
                    sendfailFun(top, (uint8_t *)(pmsg->payload), pmsg->payloadlen);
                }
            }
            MMEMORY_FREE(evt.value.p);
        }
    }
}

/**
 * MQTT���¶�������
 */
static void MQTT_ReSubscribe(void)
{
    int i, rc;

    portENTER_CRITICAL();
    for (i = 0; i < MAX_MESSAGE_HANDLERS; ++i) {
        mClient.messageHandlers[i].topicFilter = 0;
    }
    portEXIT_CRITICAL();

    /*�Ѷ��ĵ��������¶���*/
    for (i = 0; i < MAX_MESSAGE_HANDLERS; i++) {
        if (Subs[i].topic != NULL) {
            rc = MQTTSubscribe(&mClient, Subs[i].topic, Subs[i].qos, messageArrived);
#if MQTT_DEBUG > 0
            DBG_LOG("MQTT subscribe %s, return:%d", Subs[i].topic, rc);
#endif
            if (rc != 0) {
                Publish_Fail++;
                break;
            }
        }
    }
}

/**
 * ������MQTT����
 * @param id   MQTT��ID
 * @return �������ӽ��
 */
static BOOL Connect_MQTT(void)
{
    int rc = 0;

    /*����ʧ��ʱ����һ��*/
    rc = MQTTConnect(&mClient, &connectData);
    if (rc != SUCESS) {
        rc = MQTTConnect(&mClient, &connectData);
    }
    if (rc != SUCESS) {
        DBG_LOG("Return code from MQTT connect is %d", rc);
    } else {
        DBG_LOG("MQTT Connected");
    }
    return (BOOL)!rc;
}

/**
 * �Ͽ�MQTT������
 * @return ���ضϿ����
 */
static BOOL Disconnect_MQTT(void)
{
    int rc;

    if ((rc = MQTTDisconnect(&mClient)) != SUCESS) {
        DBG_LOG("Return code from MQTT disconnect is %d", rc);
    } else {
        DBG_LOG("MQTT Disonnected");
    }

    return (BOOL)!rc;
}

/**
 * ��������
 * @param argc ����������
 * @param argv �����б�
 */
static void mqtt_Console(int argc, char *argv[])
{
    Qos qos;

    argv++;
    argc--;
    if (strcmp(argv[0], "publish") == 0) {
        qos = (Qos)uatoi(argv[2]);
        if (Publish_MQTT(argv[1], qos, (uint8_t *)argv[3], strlen(argv[3]))) {
            DBG_LOG("MQTT publish %s OK.", argv[1]);
        }
    } else if (strcmp(argv[0], "status") == 0) {
        DBG_LOG("MQTT client connect:%d, ping out:%d, Net path:%d.",
                mClient.isconnected,
                mClient.ping_outstanding,
                System_SockIsConnected(NULL, NULL));
    } else if (strcmp(argv[0], "ping") == 0) {
        TimerInit(&mClient.ping_timer);
        TimerCountdownMS(&mClient.ping_timer, 100);
        DBG_LOG("test ping OK.");
    } else if (strcmp(argv[0], "disconnect") == 0) {
        Disconnect_MQTT();
        DBG_LOG("test MQTT disconnect.");
    } else if (strcmp(argv[0], "connect") == 0) {
        Connect_MQTT();
        DBG_LOG("test MQTT connect.");
    }
}
