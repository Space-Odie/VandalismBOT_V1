
from picamera import PiCamera
from time import sleep
import cv2
import numpy as np
import can

import os
import subprocess
import signal

#Import Read Script
from Can import *
from colors import *

#Set GPIO LEDs

#Configure tx/rx LEDs
tx_led = 18                     #Transmit not operational at the moment
rx_led = 16
GPIO.setmode(GPIO.BOARD)
GPIO.setup(tx_led, GPIO.OUT)
GPIO.setup(rx_led, GPIO.OUT)

GPIO.setwarnings(False)
GPIO.output(tx_led,False)
GPIO.output(rx_led, False)

#Set Global Variables
pwd = '/home/pi/Desktop/OpenCV/'    #initialize directory
camera = PiCamera()                 #initialize camera module
bus = can.interface.Bus(channel='can0', bustype='socketcan_native',
                        bitrate=125000) #initialize bus module


def Take_Picture(n):
    current_image = 'image%s.jpg' % n       #Create Unique File Name based on Instruction Number
    camera.start_preview()                  #Start Camera Mode
    sleep(2)                                #two second delay
    camera.capture(pwd + current_image)     #Take Photo
    camera.stop_preview()                   #Stop Camera Mode
    print(f'Picture {current_image} was {c.OKGREEN} Succesfully {c.ENDC}Taken')

    return current_image

def Store_Picture(current_image):

    stored_image = cv2.imread(str(current_image))
    cv2.imwrite('Stored_Img.png', stored_image)
    print(f"Stored {c.OKBLUE}{current_image}{c.ENDC} as {c.OKBLUE}Stored_Image.png{c.ENDC}\n")
    return stored_image


#Check if image is same as stored image
def Compare(image):

    imageA = cv2.imread('Stored_Img.png')
    imageB = cv2.imread(image)
    err = np.sum((imageA.astype("float") - imageB.astype("float")) ** 2)
    err /= float(imageA.shape[0] * imageA.shape[1])
    print(err)
    if err > 4000:
        return 1
    else:
        return 0

def Initialize(n):
    print(f"{c.OKBLUE}Initializing . . . {c.ENDC}")

   #TODO: Only run this below code if 'STORED_IMG.png does not exisit
    Store_Picture(Take_Picture(n))

def main():
    print("\n\r Vandalism Decetor is Online\n")
    pic_count = 0
    Current_ID = 'N\A'  # No Value on start up
    Initialize(pic_count)       # Initalize camera with default photo
    try:
        while (1):
            ID, Instruction = Can_Rx()    # Waits for Message before continueing
           # print(f"Instruction Number  was Recieved = {ID}")
           # print(f"Instruction was Recieved = {Instruction}")
           # print(type(Instruction))
            if ID != Current_ID:
                Current_ID = ID             # Assign the ID of the instruction to variable

            #Instruction be 1,2,3,4

                #Instruction = 1
                if Instruction == '0':
                    print(f"Starting Instruction {c.BOLD}0 {c.ENDC} \n")
                    pass                                            #Do Nothing

                if Instruction == '1':                              #Store New Image
                    print(f"Starting Instruction {c.BOLD}1{c.ENDC} \n")
                    Store_Picture(Take_Picture(Current_ID))         #Assign ID num to Picture

                if Instruction == '2':                              #Take Photo + Compare
                    status = Compare(Take_Picture(Current_ID))      #Assign ID num to Picture
                    Can_Tx(status)                                  #Send Status to Zybo
                    if status == 1:
                        print(f'{c.WARNING}Picture has changed!!!{c.ENDC}\n')
                    if status == 0:
                        print(f'No Change Occured')


                    #If Compare = 1 (Send digital output for zybo to turn on relay) -- End if here
                            #Zybo Print out "Pictures not same"
                            #Have LED's Blink even when silenced until it's reset (Button 4?)
                    #If Compare = 0 (Pictures the same) Have Lights blink twice on LEDs and print out "Pictures Same"

                if Instruction == '3':                            #Display Stored Image + Current Image
                    current_img = Take_Picture(Current_ID)                       #Current Image

                    #Check if Image is already pulled up and close it if it is
                    try:
                        Disp_Stored.kill()
                    except:
                        pass
                    try:
                        Disp_Current.kill()
                    except:
                        pass

                    Disp_Stored  = subprocess.Popen(['feh', 'Stored_Img.png'])   #Display Stored Image
                    Disp_Current = subprocess.Popen(['feh', current_img])        #Display Current Image
                    sleep(15)                                                    #Takes a while to pull up the images - around 15 seconds.

                if Instruction == '4':     #Send Data
                    print("Transmitting Log of Time Stamps - That Vandalism Occured")
                    #TODO: Implement this once Transmitting starts working
                    #Can_Tx()

                #TODO: KILL Project
                    #os.kill(os.getpgid(feh.pid), signal.SIGTERM)
            sleep(2.5)  #Sleep before checking for another read instruction
    except KeyboardInterrupt:
    #Catch keyboard interrupt
        GPIO.output(tx_led,False)
        GPIO.output(rx_led,False)
        print('\n\rKeyboard interrtupt')


if __name__ == "__main__": main()
