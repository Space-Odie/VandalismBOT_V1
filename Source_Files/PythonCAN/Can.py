
import can
import time
import os
import RPi.GPIO as GPIO

from colors import *

#Initialize Pi
#Configure tx/rx LEDs
GPIO.setwarnings(False)
tx_led = 18                     #Transmit not operational at the moment
rx_led = 16
GPIO.setmode(GPIO.BOARD)
GPIO.setup(tx_led, GPIO.OUT)
GPIO.setup(rx_led, GPIO.OUT)


GPIO.output(tx_led,False)
GPIO.output(rx_led, False)



def Can_Rx():
    print('\n\rEnabling CAN Rx')
    os.system("sudo /sbin/ip link set can0 up type can bitrate 250000")
    time.sleep(0.1)

    #Attempt to Connect to Channel can0
    try:
        bus = can.interface.Bus(channel='can0', bustype='socketcan_native')
    except OSError:
        print(f'{c.FAIL}Cannot find PiCAN board.{c.ENDC}')
        exit()
    print(f'{c.OKGREEN}Ready!{c.ENDC} \nWaiting for Next Instruction')


    try:
        GPIO.output(rx_led, True)   #Turn LED ON - Signal Ready to Recieve an Instruction
        message = bus.recv()        # Wait until a message is received. to continue

        a = '{0:f} {1:x} {2:x} '.format(message.timestamp, message.arbitration_id, message.dlc)
        s=''

        for i in range(message.dlc ):

            s +=  '{0:x} '.format(message.data[i])  #Entire bit string
        Inst_Num = '{0:x}'.format(message.data[0])      #Instruction Number
        Instruction = '{0:x}'.format(message.data[1])   #Instruction Position
        message = a + s
        #print(' {}'.format(a+s))                    #print time stamp // message id // dlc + data
        print(f'{c.PURP}{message}{c.ENDC}')
        #Blink LED to show reading occured succesfully
        time.sleep(.25)
        GPIO.output(rx_led, False)


        os.system("sudo /sbin/ip link set can0 down")   #End Recieving
        return Inst_Num, Instruction


    except KeyboardInterrupt:
            #Catch keyboard interrupt
            os.system("sudo /sbin/ip link set can0 down")
            print('\n\rKeyboard interrtupt')


def Can_Tx(val1):
    count = 0
    print('\n\r Enabling CAN Tx')
    os.system("sudo ip link set can0 up type can bitrate 250000")
    time.sleep(0.25)
    for i in range(3):
        try:
            bus = can.interface.Bus(channel='can0', bustype='socketcan_native')
        except OSError:
            print('Cannot find PiCAN board.')
            GPIO.output(tx_led,False)
            exit()
        try:
            GPIO.output(tx_led,True)
            print(f'{c.OKBLUE}Sending Message{c.ENDC}')
            msg = can.Message(arbitration_id=0x7de,data=[val1,0x01,0x02, 0x03, 0x04, 0x05,0x06, 0xff],extended_id=False)
            bus.send(msg)
            time.sleep(.5)
            #print(msg)
            print(f'{c.PURP}{msg}{c.ENDC}')
            GPIO.output(tx_led,False)
        except KeyboardInterrupt:
            GPIO.output(tx_led,False)
            print('\n\rKeyboard interrtupt')

