//Include Files
#include "PmodGPIO.h"
#include "sleep.h"
#include "xil_cache.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xgpio.h"
#include "xscugic.h"
#include "xil_exception.h"
#include "xil_printf.h"
#include <stdio.h>
#include "xbasic_types.h"
#include "xil_printf.h"
#include "xparameters.h"
#include "xstatus.h"
#include <time.h>
#include "PmodCAN.h"


//Parameters
#define SWITCH_DEVICE_ID	XPAR_AXI_GPIO_0_DEVICE_ID
#define BUTTON_DEVICE_ID	XPAR_AXI_GPIO_1_DEVICE_ID
#define LED_DEVICE_ID	    XPAR_AXI_GPIO_2_DEVICE_ID

#define CAN_DEVICE_ID       XPAR_PMODCAN_0_AXI_LITE_GPIO_BASEADDR
#define HORN_DEVICE_ID      XPAR_PMODGPIO_0_AXI_LITE_GPIO_BASEADDR

#define SWITCH_CHANNEL      0
#define BUTTON_CHANNEL      1
#define LED_CHANNEL         2

#define ONE_SECOND 		 	1000000
#define FIVE_SECONDS		5000000

//Initialize PMODs
PmodCAN myDevice;
PmodGPIO HORNInst;
//Initialize GPIO
XGpio LEDInst, BTNInst, SWInst;

//declare variables
int btn_value;
int sw_value;

int Switch_1;
int Switch_2;
int Switch_3;
int Switch_4;

int sw1_latched = 0;
int sw2_latched = 0;
int sw3_latched = 0;
int sw4_latched = 0;

int instruction;
int Inst_Num = 0;


/*################################################################
#                   Initialize Function
/*################################################################*/
void Initialize() {

    //Initialize PMOD GPIO
    EnableCaches();
    GPIO_begin(&HORNInst, HORN_DEVICE_ID, 0x00);

    int status;
    // Initialize Switches
    status = XGpio_Initialize(&SWInst, SWITCH_DEVICE_ID);
    if(status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&SWInst, 1, 0xFF);

    // Initialise Push Buttons
    status = XGpio_Initialize(&BTNInst, BUTTON_DEVICE_ID);
    if(status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&BTNInst, 1, 0xFF);
    // Initialise LEDs
    status = XGpio_Initialize(&LEDInst, LED_DEVICE_ID);
    if(status != XST_SUCCESS) return XST_FAILURE;
    XGpio_SetDataDirection(&LEDInst, 1, 0x00);

}

/*################################################################
#                   Print Function
/*################################################################*/
int PrintFunct(CAN_Message message) {
    u8 i;
    int state;

    xil_printf("message:\r\n");

   xil_printf("    %s Frame\r\n", (message.ide) ? "Extended" : "Standard");
   xil_printf("    ID: %03x\r\n", message.id);

   if (message.ide)
      xil_printf("    EID: %05x\r\n", message.eid);

   if (message.rtr)
      xil_printf("    Remote Transmit Request\r\n");

   else
      xil_printf("    Standard Data Frame\r\n");

   xil_printf("    dlc: %01x\r\n", message.dlc);
   xil_printf("    data:\r\n");

   for (i = 0; i < message.dlc; i++)
      xil_printf("        %02x\r\n", message.data[i]);

    state = message.data[0];
    return state;
}

CAN_Message ComposeMessage(int instruction, int Inst_Num) {
   CAN_Message message;
   xil_printf("Inst Num = %d \n\r instruction  = %d\n", instruction, Inst_Num);
   message.id = 0x100;				//Change ID here to determine priority of the message
   message.dlc = 6;					//Data Length Code (only sending 6 data length)
   message.eid = 0x15a;				//Extended Identifier
   message.rtr = 0;					//Remote Transmission
   message.ide = 0;					//Single Identifier Extension
   message.data[0] = Inst_Num; 		//Enable bit
   message.data[1] = instruction;	//Instruction bit

   //Remaining Bits for Future use. Displayed just to verify the message was sent correctly.
   //Also verify the DLC works and only 6 of the 8 bits are received.
   message.data[2] = 0x04;
   message.data[3] = 0x08;
   message.data[4] = 0x10;
   message.data[5] = 0x20;
   message.data[6] = 0x40;
   message.data[7] = 0x80;


   return message;
}

void Can_Initialize(){
    printf("Enabling Can Bus. . . ");
    EnableCaches();
   CAN_begin(&myDevice, XPAR_PMODCAN_0_AXI_LITE_GPIO_BASEADDR,
         XPAR_PMODCAN_0_AXI_LITE_SPI_BASEADDR);
   CAN_Configure(&myDevice, CAN_ModeNormalOperation);
}

void Send_Instruction(int instruction, int Inst_Num) {
    //Until reciever works on PI - Zybo will recieve its own messages ??
    Can_Initialize();
    CAN_Message TxMessage;
    u8 status;
    int count;

   xil_printf("Starting Transmitting Sequence\r\n");
    xil_printf("Waiting to send\r\n");

    do {
    status = CAN_ReadStatus(&myDevice);
    } while ((status & CAN_STATUS_TX0REQ_MASK) != 0); // Wait for buffer 0 to
    TxMessage = ComposeMessage(instruction, Inst_Num);
    xil_printf("sending ");
    PrintFunct(TxMessage);
    CAN_ModifyReg(&myDevice, CAN_CANINTF_REG_ADDR, CAN_CANINTF_TX0IF_MASK, 0);
    xil_printf("requesting to transmit message through transmit buffer 0\r\n");
    CAN_SendMessage(&myDevice, TxMessage, CAN_Tx0);
    CAN_ModifyReg(&myDevice, CAN_CANINTF_REG_ADDR, CAN_CANINTF_TX0IF_MASK, 0);
    do {
        status = CAN_ReadStatus(&myDevice);
        xil_printf("Waiting to complete transmission %d \r\n", count);
        count = count + 1;
        //create a hold in this loop?
    } while (((status & CAN_STATUS_TX0IF_MASK)!= 0) & (count < 1000000)); // Wait for message to
                                                    // transmit successfully

    sleep(1);
    CAN_end(&myDevice);
    DisableCaches();

}


void Clean() {
	xil_printf("Stopping the Device\n");
	CAN_end(&myDevice);
	DisableCaches();
}

void Read_Enable(){
    EnableCaches();
    CAN_begin(&myDevice, XPAR_PMODCAN_0_AXI_LITE_GPIO_BASEADDR,
             XPAR_PMODCAN_0_AXI_LITE_SPI_BASEADDR);
    CAN_Configure(&myDevice, CAN_ModeNormalOperation);
}

int Read_Message() {
    Read_Enable();          //Enable Read

    CAN_Message RxMessage;
    CAN_RxBuffer target;
    u8 status;
    u8 rx_int_mask;
    int state;

    xil_printf("\n\rREADING MESSAGE\n");
    xil_printf("Waiting to receive message\r\n");
    sleep(3);

    do {
        status = CAN_ReadStatus(&myDevice);     //check until message is available

    } while ((status & CAN_STATUS_RX0IF_MASK) != 0
        && (status & CAN_STATUS_RX1IF_MASK) != 0);


    xil_printf("fetching message from receive buffer 0\r\n");
    target = CAN_Rx0;
    rx_int_mask = CAN_CANINTF_RX0IF_MASK;

    CAN_ReceiveMessage(&myDevice, &RxMessage, target);
    CAN_ModifyReg(&myDevice, CAN_CANINTF_REG_ADDR, rx_int_mask, 0);
    xil_printf("received ");
    state = PrintFunct(RxMessage);  //state = message bit 0
    sleep(1);

    //End Read
   CAN_end(&myDevice);
   DisableCaches();

   return state;
}
/*################################################################
#                       Main Function
/*################################################################*/
int main(void) {
	xil_printf("Welcome!\n\r Button Functions: \n");
	xil_printf("Button 1:     Set Instruction to Store new Photo as Default\n");
	xil_printf("Button 2:     Check if Image was ...Vandalized~!\n");
	xil_printf("Button 3:     Stored Image and Current Image\n\r");
	xil_printf("Switch Functions: \n");
	xil_printf("Switch 1:     Silence Alarm\n");
	xil_printf("Switch 2:     Test Horn\n");
	xil_printf("Switch 3:     Send Loaded Instruction to RPi\n\r");


    Initialize();



    while(1){

        //Step 1: Update Input Values
    	btn_value = XGpio_DiscreteRead(&BTNInst, 1);
        sw_value = XGpio_DiscreteRead(&SWInst, 1);
        //mask switches
        Switch_1 = sw_value & 0x1;
        Switch_2 = sw_value & 0x2;
        Switch_3 = sw_value & 0x4;
        Switch_4 = sw_value & 0x8;

        //Button Control
	    switch(btn_value){
            case 1 :                                                    //Store New Photo
                XGpio_DiscreteWrite(&LEDInst, 1, btn_value);
                instruction = 1;
                xil_printf("Instruction Set: Store new Default Image\n\r");
                break;
            case 2 :                                                    //Take Photo
                XGpio_DiscreteWrite(&LEDInst, 1, btn_value);
                instruction = 2;
                xil_printf("Instruction Set: Take Photo\n\r");
                break;
            case 4 :
                XGpio_DiscreteWrite(&LEDInst, 1, btn_value);
                instruction = 3;
                xil_printf("Instruction Set: Display Stored and Current Image\n\r");
                break;
                break;
            case 8 :
                XGpio_DiscreteWrite(&LEDInst, 1, btn_value);
                instruction = 4;
                xil_printf("Instruction Set: Future Use\n\r");
                break;
            default :
                break;

	    }

        //LED Control
        XGpio_DiscreteWrite(&LEDInst, 1, instruction);

        //Switch Control (Place this in interrupt Handler)
        if (Switch_1 == 1){                                          //Silence Horn (Pri 1)
            GPIO_setPin(&HORNInst, 1, 0);
            if (sw1_latched == 0){
                xil_printf("Horn Silenced\n\r");
            }
            GPIO_setPin(&HORNInst, 1, 0);	//Relay OFF
            sw1_latched = 1;
        }
        else if (Switch_1 == 0 && sw1_latched == 1){
            sw1_latched = 0;
            xil_printf("Horn Enabled \n\r");

        }

        if (Switch_2 == 2 && sw2_latched == 0){                                          //Horn Test
            if(sw1_latched){
                xil_printf("Horn can not be tested while silenced \n\r");
            }
            else if (sw1_latched == 0){
                xil_printf("Horn On!\n\r");
                GPIO_setPin(&HORNInst, 1, 1);	//Relay ON
                sleep(.5);
            }

            sw2_latched = 1;
            xil_printf("sw2 Latched");
        }
        if (Switch_2 == 0 && sw2_latched == 1){

            GPIO_setPin(&HORNInst, 1, 0);       //Relay Off
            xil_printf("Horn Off!\n\r");
            sleep(.5);
            sw2_latched = 0;
        }

        if (Switch_3 == 4 && sw3_latched == 0){                 //Send Instruction
            xil_printf("Sending Instruction: \n\r");
            Send_Instruction(instruction, Inst_Num);
            Inst_Num = Inst_Num + 1;							//Increment each time a new unique instruction is sent
            Clean();
            sw3_latched = 1;

            //if an instruction to compare images was sent - wait for read command
            if (instruction == 2){
                int state = Read_Message();
                if (state == 0){
                    xil_printf("No Change");  
                }
                else if (state == 1){
                    xil_printf("Change Occured!");
                    GPIO_setPin(&HORNInst, 1, 1);	//Relay ON
                }
            }
        }
        if (Switch_3 == 0 && sw3_latched == 1){
            sw3_latched = 0;
        }

        //Switch 4 used for reading commands when it's capable
        sleep(2.5);
    }
    return 0;
}

/*################################################################
#                      Pre-exisiting Functions
/*################################################################*/


void EnableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheEnable();
#endif
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheEnable();
#endif
#endif
}

void DisableCaches() {
#ifdef __MICROBLAZE__
#ifdef XPAR_MICROBLAZE_USE_DCACHE
   Xil_DCacheDisable();
#endif
#ifdef XPAR_MICROBLAZE_USE_ICACHE
   Xil_ICacheDisable();
#endif
#endif
}


