#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>

#include "ethercat.h"

#define PI  3.14159265359
#define DEADLINE 900000 //deadline in ns
#define EC_TIMEOUTMON 500

#define PORT 2000
#define BUF_SIZE 1024
char buffer[BUF_SIZE] = { 0, };
char mode[4];
char value1[8];
char value2[8];
int  valueIdx = 4;


char IOmap[4096];
OSAL_THREAD_HANDLE thread1, thread2;
int expectedWKC;
boolean needlf;
volatile int wkc;
boolean inOP;
uint8 currentgroup = 0;
int dorun = 1;
int64 toff, g_delta;


struct sched_attr
{
    uint32 size;
    uint32 sched_policy;
    uint64 sched_flags;

    /* used by: SCHED_NORMAL, SCHED_BATCH */
    int32  sched_nice;

    /* used by: SCHED_FIFO, SCHED_RR */
    uint32 sched_priority;

    /* used by: SCHED_DEADLINE (nsec) */
    uint64 sched_runtime;
    uint64 sched_deadline;
    uint64 sched_period;
};

/* sched_policy */
#define SCHED_DEADLINE 6

/* helpers */
#define gettid() syscall(__NR_gettid)
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
    return syscall(__NR_sched_setattr, pid, attr, flags);
}
int sched_getattr(pid_t pid, struct sched_attr *attr, unsigned int size, unsigned int flags)
{
    return syscall(__NR_sched_getattr, pid, attr, size, flags);
}


/** Structure represents the Object Dictionary **/
typedef struct
{
    int index;
    int sub_index;
    int size;
    int value;
} objST;

/********* PDO Objects Definition *********/
#define EPOS4 1

typedef struct PACKED
{
    uint16  control_word;
    int8    mode_of_operation;
    int16   target_torque;
    int32   target_velocity;
    int32   target_position;
    int32   position_offset;
} RxEpos4;
typedef struct PACKED
{
    uint16  status_word;
    int8    mode_of_operation_display;
    int32   position_actual_value;
    int32   velocity_actual_value;
    int16   torque_actual_value;
    int32   current_actual_value;
} TxEpos4;

RxEpos4* rxPDO;
TxEpos4* txPDO;


/** Function for PDO mapping **/
void PdoMapping(uint16 slave)
{
    uint8 PDO_abled = 0;
    uint8 RxPDOs_number = 0;
    uint8 TxPDOs_number = 0;

    /* <0x1600> Receive PDO Mapping 1 */
    objST RxPDO1 = { 0x1600, 0x01, sizeof(uint32), 0x60400010 };   // 0x60400010 : control_word               UInt16
    objST RxPDO2 = { 0x1600, 0x02, sizeof(uint32), 0x60600008 };   // 0x60600008 : mode_of_operation          Int8
    objST RxPDO3 = { 0x1600, 0x03, sizeof(uint32), 0x60710010 };   // 0x60710010 : target_torque              Int16
    objST RxPDO4 = { 0x1600, 0x04, sizeof(uint32), 0x60ff0020 };   // 0x60ff0020 : target_velocity            Int32
    objST RxPDO5 = { 0x1600, 0x05, sizeof(uint32), 0x607a0020 };   // 0x607a0020 : target_position            Int32
    objST RxPDO6 = { 0x1600, 0x06, sizeof(uint32), 0x60B00020 };   // 0x60B00020 : position_offset            Int32
    // objST RxPDO7 = { 0x1600, 0x07, sizeof(), 0x };
    // objST RxPDO8 = { 0x1600, 0x08, sizeof(), 0x };
    objST SM2_choose_RxPDO = { 0x1C12, 0x01, sizeof(uint16), RxPDO1.index };

    /* <0x1A00> Transmit PDO Mapping 1 */
    objST TxPDO1 = { 0x1A00, 0x01, sizeof(uint32), 0x60410010 };   // 0x60410010 : status_word                UInt16
    objST TxPDO2 = { 0x1A00, 0x02, sizeof(uint32), 0x60610008 };   // 0x60610008 : mode_of_operation_display  Int8
    objST TxPDO3 = { 0x1A00, 0x03, sizeof(uint32), 0x60640020 };   // 0x60640020 : position_actual_value      Int32
    objST TxPDO4 = { 0x1A00, 0x04, sizeof(uint32), 0x606C0020 };   // 0x606C0020 : velocity_actual_value      Int32
    objST TxPDO5 = { 0x1A00, 0x05, sizeof(uint32), 0x60770010 };   // 0x60770010 : torque_actual_value        Int16
    objST TxPDO6 = { 0x1A00, 0x06, sizeof(uint32), 0x30D10220 };   // 0x30D10220 : current_actual_value       Int32
    // objST TxPDO7 = { 0x1A00, 0x07, sizeof(), 0x };
    // objST TxPDO8 = { 0x1A00, 0x08, sizeof(), 0x };
    objST SM3_choose_TxPDO = { 0x1C13, 0x01, sizeof(uint16), TxPDO1.index };

    // 1. Write the value “0” (zero) to subindex 0x00 (disable PDO).
    ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(PDO_abled), &PDO_abled, EC_TIMEOUTSAFE);
    ec_SDOwrite(slave, 0x1A00, 0x00, FALSE, sizeof(PDO_abled), &PDO_abled, EC_TIMEOUTSAFE);
    ec_SDOwrite(slave, 0x1C12, 0x00, FALSE, sizeof(PDO_abled), &PDO_abled, EC_TIMEOUTSAFE);
    ec_SDOwrite(slave, 0x1C13, 0x00, FALSE, sizeof(PDO_abled), &PDO_abled, EC_TIMEOUTSAFE);

    // 2. Modify the desired objects in subindex 0x01...0x0n.
    ec_SDOwrite(slave, RxPDO1.index, RxPDO1.sub_index, FALSE, RxPDO1.size, &(RxPDO1.value), EC_TIMEOUTSAFE);
    RxPDOs_number++;
    ec_SDOwrite(slave, RxPDO2.index, RxPDO2.sub_index, FALSE, RxPDO2.size, &(RxPDO2.value), EC_TIMEOUTSAFE);
    RxPDOs_number++;
    ec_SDOwrite(slave, RxPDO3.index, RxPDO3.sub_index, FALSE, RxPDO3.size, &(RxPDO3.value), EC_TIMEOUTSAFE);
    RxPDOs_number++;
    ec_SDOwrite(slave, RxPDO4.index, RxPDO4.sub_index, FALSE, RxPDO4.size, &(RxPDO4.value), EC_TIMEOUTSAFE);
    RxPDOs_number++;
    ec_SDOwrite(slave, RxPDO5.index, RxPDO5.sub_index, FALSE, RxPDO5.size, &(RxPDO5.value), EC_TIMEOUTSAFE);
    RxPDOs_number++;
    ec_SDOwrite(slave, RxPDO6.index, RxPDO6.sub_index, FALSE, RxPDO6.size, &(RxPDO6.value), EC_TIMEOUTSAFE);
    RxPDOs_number++;
    // ec_SDOwrite(slave, RxPDO7.index, RxPDO7.sub_index, FALSE, RxPDO7.size, &(RxPDO7.value), EC_TIMEOUTSAFE);
    // RxPDOs_number++;
    // ec_SDOwrite(slave, RxPDO8.index, RxPDO8.sub_index, FALSE, RxPDO8.size, &(RxPDO8.value), EC_TIMEOUTSAFE);
    // RxPDOs_number++;

    ec_SDOwrite(slave, TxPDO1.index, TxPDO1.sub_index, FALSE, TxPDO1.size, &(TxPDO1.value), EC_TIMEOUTSAFE);
    TxPDOs_number++;
    ec_SDOwrite(slave, TxPDO2.index, TxPDO2.sub_index, FALSE, TxPDO2.size, &(TxPDO2.value), EC_TIMEOUTSAFE);
    TxPDOs_number++;
    ec_SDOwrite(slave, TxPDO3.index, TxPDO3.sub_index, FALSE, TxPDO3.size, &(TxPDO3.value), EC_TIMEOUTSAFE);
    TxPDOs_number++;
    ec_SDOwrite(slave, TxPDO4.index, TxPDO4.sub_index, FALSE, TxPDO4.size, &(TxPDO4.value), EC_TIMEOUTSAFE);
    TxPDOs_number++;
    ec_SDOwrite(slave, TxPDO5.index, TxPDO5.sub_index, FALSE, TxPDO5.size, &(TxPDO5.value), EC_TIMEOUTSAFE);
    TxPDOs_number++;
    ec_SDOwrite(slave, TxPDO6.index, TxPDO6.sub_index, FALSE, TxPDO6.size, &(TxPDO6.value), EC_TIMEOUTSAFE);
    TxPDOs_number++;
    // ec_SDOwrite(slave, TxPDO7.index, TxPDO7.sub_index, FALSE, TxPDO7.size, &(TxPDO7.value), EC_TIMEOUTSAFE);
    // RxPDOs_number++;
    // ec_SDOwrite(slave, TxPDO8.index, TxPDO8.sub_index, FALSE, TxPDO8.size, &(TxPDO8.value), EC_TIMEOUTSAFE);
    // RxPDOs_number++;

    // 3. Write the No. of mapped object n in subindex 0x00
    ec_SDOwrite(slave, 0x1600, 0x00, FALSE, sizeof(RxPDOs_number), &(RxPDOs_number), EC_TIMEOUTSAFE);
    ec_SDOwrite(slave, 0x1A00, 0x00, FALSE, sizeof(TxPDOs_number), &(TxPDOs_number), EC_TIMEOUTSAFE);

    // 4. Make the changes effective
    PDO_abled = 1;
    ec_SDOwrite(slave, SM2_choose_RxPDO.index, SM2_choose_RxPDO.sub_index, FALSE, SM2_choose_RxPDO.size, &(SM2_choose_RxPDO.value), EC_TIMEOUTSAFE);
    ec_SDOwrite(slave, SM3_choose_TxPDO.index, SM3_choose_TxPDO.sub_index, FALSE, SM3_choose_TxPDO.size, &(SM3_choose_TxPDO.value), EC_TIMEOUTSAFE);
    ec_SDOwrite(slave, 0x1C12, 0x00, FALSE, sizeof(PDO_abled), &(PDO_abled), EC_TIMEOUTSAFE);
    ec_SDOwrite(slave, 0x1C13, 0x00, FALSE, sizeof(PDO_abled), &(PDO_abled), EC_TIMEOUTSAFE);
}

void velocityMode(RxEpos4* rxPDO, int32 iVelocity)
{
    rxPDO->mode_of_operation = 0x09;
    rxPDO->target_velocity = iVelocity;
}
void torqueMode(RxEpos4* rxPDO, int16 iTorque)
{
    rxPDO->mode_of_operation = 0x0A;
    rxPDO->target_torque = iTorque;
}


/** Function for parameter setup and PdoMapping call **/
int SetupParam(uint16 slave)
{
    // objST period = { 0x60C2, 0x01, sizeof(uint8), (uint8)1 };
    //
    // /* SETUP of default values */
    // retval = ec_SDOwrite(slave, period.index, period.sub_index, FALSE, sizeof(uint8), &(period.value), EC_TIMEOUTSAFE);
    //
    //  /* Check that the parameters are set correctly */
    // int retval;
    // uint8 period_value;
    // retval = ec_SDOread(slave, period.index, period.sub_index, FALSE, &(period.size), &period_value, EC_TIMEOUTSAFE);
    // printf("period=%u,size=%d,return=%d\n", period_value, period.size, retval);

    /* PDO mapping */
    PdoMapping(EPOS4);

    printf("\nEPOS4 slave %d set\n", slave);
    return 1;
}

/** Function that updates the deadline to synchronize ecatthread **/
void RefreshDeadline(struct sched_attr *attr, uint64 addtime)
{
    attr->sched_runtime = 0.95 * addtime;
    attr->sched_deadline = addtime;
    attr->sched_period = addtime;

    if (sched_setattr(gettid(), attr, 0))
    {
        perror("sched_setattr failed");
        exit(1);
    }
}

/** PI calculation to get linux time synced to DC time **/
void ec_sync(int64 reftime, int64 cycletime, int64 *offsettime)
{
    static int64 integral = 0;
    int64 delta;

    delta = (reftime) % cycletime;
    if (delta > (cycletime / 2)) { delta = delta - cycletime; }
    if (delta > 0) { integral++; }
    if (delta < 0) { integral--; }
    *offsettime = -(delta / 100) - (integral / 20);
    g_delta = delta;
}

void errorHandling(char *errmsg)
{
    fputs(errmsg, stderr);
    fputc('\n', stderr);
    exit(1);
}

void readString(int socketFD, char *buffer, int length)
{
    memset(buffer, 0, BUF_SIZE);
    read(socketFD, (void *)buffer, length);
}


OSAL_THREAD_FUNC simpletest(char *ifname)
{
    int i;

    needlf = FALSE;
    inOP = FALSE;

    printf("Starting simple test\n");

    /* initialise SOEM, bind socket to ifname */
    if (ec_init(ifname))
    {
        printf("ec_init on %s succeeded.\n", ifname);

        /* find and auto-config slaves */
        if (ec_config_init(FALSE) > 0)
        {
            printf("%d slaves found and configured.\n", ec_slavecount);
            printf("\nName: %s EEpMan: %d eep_id: %d State %d\n\r", ec_slave[EPOS4].name, ec_slave[EPOS4].eep_man, ec_slave[EPOS4].eep_id, ec_slave[EPOS4].state);

            /* link slave specific setup to preOP->safeOP hook */
            ec_slave[EPOS4].PO2SOconfig = SetupParam;

            /* Locate DC slaves, measure propagation delays. */
            ec_configdc();

            // maps the previously mapped PDOs into the local buffer
            ec_config_map(&IOmap);

            /* wait for all slaves to reach SAFE_OP state */
            ec_statecheck(0, EC_STATE_SAFE_OP, EC_TIMEOUTSTATE);
            printf("\nSlaves mapped, state to SAFE_OP.\n");

            /* connect struct pointers to slave I/O pointers */
            rxPDO = (RxEpos4*)(ec_slave[EPOS4].outputs);
            txPDO = (TxEpos4*)(ec_slave[EPOS4].inputs);

            expectedWKC = (ec_group[0].outputsWKC * 2) + ec_group[0].inputsWKC;
            printf("Calculated workcounter %d\n", expectedWKC);

            printf("\nRequest operational state for all slaves\n");

            /* send one processdata cycle to init SM in slaves */
            ec_send_processdata();
            ec_receive_processdata(EC_TIMEOUTRET);

            /* Call ec_dcsync0() to synchronize the slave and master clock */
            ec_dcsync0(EPOS4, TRUE, DEADLINE, 0); // SYNC0 on slave 1

            /* request OP state for all slaves */
            ec_slave[0].state = EC_STATE_OPERATIONAL;
            ec_writestate(0);

            /* wait for all slaves to reach OP state */
            ec_statecheck(0, EC_STATE_OPERATIONAL, EC_TIMEOUTSTATE * 5);

            /* Now we have a system up and running, all slaves are in state operational */
            if (ec_slave[0].state == EC_STATE_OPERATIONAL)
            {
                printf("\nOperational state reached for all slaves.\n\n");
                inOP = TRUE;
                dorun = 1; // activate cyclic rt process


                struct sockaddr_in server_addr, client_addr;
                int client_addr_size = sizeof(client_addr);
                int server_sock, client_sock;

                if ((server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
                    errorHandling("socket() error");

                int option = 1;
                setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof(option));
                server_addr.sin_family = AF_INET;
                server_addr.sin_addr.s_addr = INADDR_ANY;
                server_addr.sin_port = htons(PORT);

                if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
                    errorHandling("bind() error");

                if (listen(server_sock, 5) < 0)
                    errorHandling("listen() error");

                // printf("accept wait...\n");

                if ((client_sock = accept(server_sock, (struct sockaddr *)&client_addr, (socklen_t *)&client_addr_size)) < 0)
                    errorHandling("accept() error");

                // printf("client accepted!\n");

                int flag = fcntl(client_sock, F_GETFL, 0);
                fcntl(client_sock, F_SETFL, flag | O_NONBLOCK);

                int retval, length;

                /* acyclic loop 10ms */
                while (1)
                {
                    /* 길이정보 수신하여 length에 저장 */
                    retval = read(client_sock, &length, sizeof(length));
                    if (retval < 0)
                    {
                        if (errno != EAGAIN)
                        {
                            printf("\n\nread() error\n");
                            printf("----- Socket close -----\n");
                            break;
                        }
                    }
                    else if (retval == 0)
                    {
                        printf("\n\n----- Socket close -----\n");
                        break;
                    }
                    else
                    {
                        /* length만큼 문자열 수신 */
                        readString(client_sock, buffer, length);
                        memcpy(mode, buffer, valueIdx);
                        memcpy(value1, &buffer[4], 8);
                        memcpy(value2, &buffer[12], 8);
                    }
                    osal_usleep(10000);
                }
                close(client_sock);
                dorun = 0;
                inOP = FALSE;
                sleep(1);
            }
            else
            {
                printf("Not all slaves reached operational state.\n");
                ec_readstate();
                for (i = 1; i <= ec_slavecount; i++)
                {
                    if (ec_slave[i].state != EC_STATE_OPERATIONAL)
                    {
                        printf("Slave %d State=0x%2.2x StatusCode=0x%4.4x : %s\n", i, ec_slave[i].state, ec_slave[i].ALstatuscode, ec_ALstatuscode2string(ec_slave[i].ALstatuscode));
                    }
                }
            }
            printf("Request safe operational state for all slaves\n");
            ec_slave[0].state = EC_STATE_SAFE_OP;
            /* request SAFE_OP state for all slaves */
            ec_writestate(0);
        }
        else
        {
            printf("No slaves found!\n");
        }
    }
    else
    {
        printf("No socket connection on %s\nExecute as root\n", ifname);
    }
}


/** RT EtherCAT thread **/
OSAL_THREAD_FUNC ecatthread()
{
    double t_loopStart, t_lastLoopStart, t_taskEnd;
    struct timespec ts;

    printf("\nRT ecatthread started [%ld]\n", gettid());

    struct sched_attr attr;
    attr.size = sizeof(attr);
    attr.sched_policy = SCHED_DEADLINE;
    attr.sched_flags = 0;
    attr.sched_nice = 0;
    attr.sched_priority = 0;

    if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
    {
        printf("mlockall failed: %m\n");
        pthread_cancel(pthread_self());
    }

    toff = 0;
    dorun = 0;
    ec_send_processdata();
    int cnt = 0;
    int cwFlag = 1;
    while (1)
    {
        /* calculate next cycle start */
        RefreshDeadline(&attr, (uint64)(DEADLINE + toff));
        /* wait to cycle start */
        sched_yield();

        if (dorun)
        {
            /********************** LOOP TIME MEASUREMENT **********************/
            clock_gettime(CLOCK_MONOTONIC, &ts);
            t_loopStart = ts.tv_nsec;
            printf("[Loop time: %.4lfms] |                        \r", (t_loopStart - t_lastLoopStart) / 1000000.0);
            t_lastLoopStart = t_loopStart;

            /*************************** LOOP TASKS ***************************/
            ec_send_processdata();
            wkc = ec_receive_processdata(EC_TIMEOUTRET);

            /* BEGIN USER CODE */
            if ((txPDO->status_word & 0x08)) // Fault state
            {
                rxPDO->control_word = 0x80; // Fault reset
            }
            else if ((txPDO->status_word & 0x40)) // Switch on disabled state
            {
                rxPDO->control_word = 0x06; // Shut down
            }
            else if (!(txPDO->status_word & 0x02)) // Not switched on
            {
                rxPDO->control_word = 0x07; // Switch on
            }
            else if (!(txPDO->status_word & 0x04)) // Not operation enabled
            {
                rxPDO->control_word = 0x0f; // Operation enable
            }
            else if (!strcmp(mode, "v001"))
            {
                velocityMode(rxPDO, atoi(value1));
            }
            else if (!strcmp(mode, "t001"))
            {
                torqueMode(rxPDO, atoi(value1));
            }
            else if (!strcmp(mode, "v002"))
            {
                rxPDO->mode_of_operation = 0x09;
                rxPDO->target_velocity = cnt;
                if (cwFlag)
                {
                    if (cnt < 20000)
                        cnt += 10;
                    else
                        cwFlag = 0;
                }
                else
                {
                    if (cnt > -20000)
                        cnt -= 10;
                    else
                        cwFlag = 1;
                }
            }
            else if (!strcmp(mode, "s001"))
            {
                rxPDO->mode_of_operation = 0x09;
                rxPDO->target_velocity = atoi(value1) * sin((2 * PI * atoi(value2)) / 1000 * cnt++);
                if (cnt == 20000)
                    cnt = 0;
            }

            printf("| [Received: %20s] ", (char*)buffer);
            printf("| [Velocity: %6d], [Torque: %5d] ", txPDO->velocity_actual_value, txPDO->torque_actual_value);
            fflush(stdout);

            if (ec_slave[0].hasdc)
            {
                /* calculate toff to get linux time and DC synced */
                ec_sync(ec_DCtime, DEADLINE, &toff);
            }

            /********************** TASK TIME MEASUREMENT **********************/
            clock_gettime(CLOCK_MONOTONIC, &ts);
            t_taskEnd = ts.tv_nsec;
            printf("| [Task time: %.4lfms], ", (t_taskEnd - t_loopStart) / 1000000.0);
        }

    }

    printf("End motor test, close socket\n");
    ec_close();
}


OSAL_THREAD_FUNC ecatcheck()
{
    int slave;

    while (1)
    {

        if (inOP && ((wkc < expectedWKC) || ec_group[currentgroup].docheckstate))
        {
            if (needlf)
            {
                needlf = FALSE;
                printf("\n");
            }

            /* one ore more slaves are not responding */
            ec_group[currentgroup].docheckstate = FALSE;
            ec_readstate();

            for (slave = 1; slave <= ec_slavecount; slave++)
            {
                if ((ec_slave[slave].group == currentgroup) && (ec_slave[slave].state != EC_STATE_OPERATIONAL))
                {
                    ec_group[currentgroup].docheckstate = TRUE;

                    if (ec_slave[slave].state == (EC_STATE_SAFE_OP + EC_STATE_ERROR))
                    {
                        printf("ERROR : slave %d is in SAFE_OP + ERROR, attempting ack.\n", slave);
                        ec_slave[slave].state = (EC_STATE_SAFE_OP + EC_STATE_ACK);
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state == EC_STATE_SAFE_OP)
                    {
                        printf("WARNING : slave %d is in SAFE_OP, change to OPERATIONAL.\n", slave);
                        ec_slave[slave].state = EC_STATE_OPERATIONAL;
                        ec_writestate(slave);
                    }
                    else if (ec_slave[slave].state > EC_STATE_NONE)
                    {
                        if (ec_reconfig_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d reconfigured\n", slave);
                        }
                    }
                    else if (!ec_slave[slave].islost)
                    {
                        /* re-check state */
                        ec_statecheck(slave, EC_STATE_OPERATIONAL, EC_TIMEOUTRET);

                        if (ec_slave[slave].state == EC_STATE_NONE)
                        {
                            ec_slave[slave].islost = TRUE;
                            printf("ERROR : slave %d lost\n", slave);
                        }
                    }
                }

                if (ec_slave[slave].islost)
                {
                    if (ec_slave[slave].state == EC_STATE_NONE)
                    {
                        if (ec_recover_slave(slave, EC_TIMEOUTMON))
                        {
                            ec_slave[slave].islost = FALSE;
                            printf("MESSAGE : slave %d recovered\n", slave);
                        }
                    }
                    else
                    {
                        ec_slave[slave].islost = FALSE;
                        printf("MESSAGE : slave %d found\n", slave);
                    }
                }
            }

            if (!ec_group[currentgroup].docheckstate)
                printf("OK : all slaves resumed OPERATIONAL.\n");
        }

        osal_usleep(10000);
    }
}

int main(int argc, char *argv[])
{
    printf("SOEM (Simple Open EtherCAT Master)\n< Motor test >\n");

    if (argc > 1)
    {
        /* create RT thread */
        osal_thread_create(&thread1, 128000, &ecatthread, NULL);

        /* create thread to handle slave error handling in OP */
        osal_thread_create(&thread1, 128000, &ecatcheck, NULL);

        /* start acyclic part */
        simpletest(argv[1]);
    }
    else
    {
        ec_adaptert *adapter = NULL;
        printf("Usage: motor_test ifname\nifname = eth0 for example\n");

        printf("\nAvailable adapters:\n");
        adapter = ec_find_adapters();

        while (adapter != NULL)
        {
            printf("    - %s  (%s)\n", adapter->name, adapter->desc);
            adapter = adapter->next;
        }

        ec_free_adapters(adapter);
    }

    printf("End program\n");
    return (0);
}