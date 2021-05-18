Python Setup: 

Software Configuration: 

Step 1 Dependencies:
pip install python-can
pip install opencv-python
pip install numpy

Step 2 Verify Source Files are in the same directory
Can.py
colors.py
run.py

Step 3
type "python3 run.py" in the command prompt


Step 4) RPI Configuration:
4.1) Add the following line to your /boot/config.txt file:
	 dtoverlay=mcp2515-can0,oscillator=8000000,interrupt=25

	oscillator parameter should be set to the actual crystal frequency found
	on your MCP2515. This frequency can change between modules, and is
	commonly either 16 or 8 MHz. For my project it was 8 MHz

	The interrupt GPIO number used for this project is pin 25


4.2) Optional: Configure the Pi to manually bring up the CAN interfaces by typing the following command
    
	sudo ip link set can0 type can bitrate 250000
	
	This step is optional because it is automatically done when run.py calls Can.py for the first time.


4.3) use ifconfig to get network interface details and verify the can interface is up
	note: If you need to change your bit rate but it's busy. use the following
	command first:
                   ifconfig can0 down

Step 5) Hardware Configuration
5.1) Verify the wiring is connected as per Project_Schematic.pdf
5.2) Verify 120 ohm resistors are connected in parallel to the transmitter and reciever



