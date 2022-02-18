#ifndef MACRO_H
#define MACRO_H

#include "ethercat.h"

#define TCP_PORT 2000
#define UDP_PORT 3000
#define GUI_PC_IP "192.168.0.9"
#define RASPBERRY_PI_IP "192.168.0.10"

#define RX_BUFFER_SIZE 1024
#define TX_BUFFER_SIZE 1024
#define PACKET_BUFFER_SIZE 1024


typedef struct
{
    int amplitude;
    int frequency;
}SINUSOIDAL_VELOCITY_INPUT;


// Structure representing the Object Dictionary
typedef struct
{
    int index;
    int sub_index;
    int size;
    int value;
}OBJ;

// PDO Objects Definition
typedef struct PACKED
{
    uint16  control_word;
    int8    mode_of_operation;
    int16   target_torque;
    int32   target_velocity;
    int32   target_position;
    int32   position_offset;
}RX_PDO;
typedef struct PACKED
{
    uint16  status_word;
    int8    mode_of_operation_display;
    int32   position_actual_value;
    int32   velocity_actual_value;
    int16   torque_actual_value;
    int32   current_actual_value;
}TX_PDO;


// State EPOS4 state machine
#define SWITCH_ON_DISABLED     0x40
#define READY_TO_SWITCH_ON     0x21
#define SWITCHED_ON            0x02
#define OPERATION_ENABLED      0x04 
#define QUICKSTOP_ACTIVE       0x07
#define FAULT_REACTION_ACTIVE  0x0F
#define FAULT                  0x08

// Command EPOS4 state machine
#define SHUTDOWN               0x06
#define SWITCH_ON              0x07
#define SWITCH_ON_ENABLE       0x0F
#define DISABLE_VOLTAGE        0x00
#define QUICK_STOP             0X02
#define FAULT_RESET            0X80 

// Modes of operaton
#define PPM 0x01
#define PVM 0x03
#define HMM 0x06
#define CSP 0x08
#define CSV 0x09
#define CST 0x0A


////////////////////////////////////////////////////
///////////////// GUI PC ----> RPI /////////////////
////////////////////////////////////////////////////

#define COMMAND_MODE_TARGET_VELOCITY              0x0001
#define COMMAND_MODE_TARGET_TORQUE                0x0002
#define COMMAND_MODE_BACK_AND_FORTH_VELOCITY      0x0003
#define COMMAND_MODE_SINUSOIDAL_VELOCITY          0x0004


////////////////////////////////////////////////////
///////////////// RPI ----> GUI PC /////////////////
////////////////////////////////////////////////////

// #define COMMAND_CODE_GET_REGISTRATION_PARAMETERS            0XABC0
// #define COMMAND_CODE_GET_FILTER_PARAMETERS                  0XABC1

#endif // MACRO_H
