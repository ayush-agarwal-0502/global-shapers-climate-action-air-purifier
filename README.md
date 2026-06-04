# global-shapers-climate-action-air-purifier

Documentation for Air purifier project 

This project is under Global Shapers (WEF Affiliated) , Climate Action team. Currently being worked upon by the Global Shapers New Delhi hub (GSND).

I'm Ayush Agarwal, 2 months volunteer to Global shapers for this project, and I hope that this documentation serves well for the next person that inherits the project after me. Good luck on this project ! I have given very very detailed explanation of the decision making behind the project, the files etc. 

# Mission Statement – 
To make clean and healthy air accessible, and improve public health, through affordable, community-driven technology. 

# Goals – 

**Purification efficacy –** The device must be able to have a reasonable impact on air quality in a reasonable amount of time. 


**Cost –** The device must cost lower than the air purifiers currently commercially available in the market. 


**Assembly –** The device must be simple to assemble, since it will be assembled by Self help groups. 


**Scalable –** The parts used must be widely available in the market. 

## First Principles Thinking – Basic version - 
At its simplest, the air purifier systems must pass air through the filter. 
This filter has to be a HEPA certified filter (HEPA = High-Efficiency Particulate Air), since these are widely regarded to be effective, as opposed to local solutions. 
It must be ensured that the filter is HEPA certified (and not “HEPA like”) to ensure validity of the claim that our air purifier works well. 
So a very basic system would look like a fan passing air through the HEPA filter. 


**Choice of HEPA –**

Cylindrical HEPA filters maximize the surface area for air intake, while sheet based HEPA usually have smaller surface area, hence we decided to proceed with cylindrical HEPA. 


**Choice of Fan –**

The fan must be easily available in the market so that anyone can replicate our work. Therefore, solutions involving BLDC motor (Brushless DC) with custom propellors, support structure etc would not serve us well. 
Therefore we decided to proceed with Gaming fans – these are widely available fans, which are typically used to cool GPUs for gamers. 
Other alternative was to utilize CPU fans, which were far cheaper than Gaming fans, however, their speed and efficiency were much lower than the GPU fans. We need a strong motor to be able to push air through the HEPA filter, cheaper motors just don’t create enough pressure to push air through it. 

## IoT version – 
However, to validate the fact that the device indeed works perfectly, we also need to measure AQI. 
Hence we decided to build this version. 
So, there would be requirement for AQI sensor. 
And, we would need to send the data to be stored, therefore there would be requirement for WiFi module like ESP32 NodeMCU, or ESP8266. 
Simple understanding of the system would be that the AQI sensor would measure the AQI, and would send the data to cloud storage using the WIFI module. 


**Providing intuition for the circuit diagram –** 

<img width="1291" height="737" alt="Final_schematic_diagram" src="https://github.com/user-attachments/assets/0d7120c7-53d5-4289-8ba6-aa4d2de15470" />


We are using an Adapter (a device which looks like a phone charger, takes in 220V AC, the voltage commonly available in Indian homes, and converts it to 12V DC, accessible via a cylindrical pin). 
(Future version of it might explore being able to run the system using phone chargers and USB port, however phone chargers typically give out 5V, which was insufficient to run Gaming fans at high speed)
To connect this adapter, we need a 2.1mm Barrel Jack. 
This now serves as 12V Voltage source for our circuit. 
The 12V is fed directly to the motor (the Gaming fan), so that the air purifier works, irrespective of the other parts of circuit working.
We utilize a voltage regulator, to drop the 12V to 5V, since the ESP32 and AQI sensor operate on 5V source (using 12V source will burn the chips, so need to be careful)(Side note – ESP32 takes in 5V, internally steps it down to 3.3V, and then uses it, so some sources may say that ESP32 uses 3.3V)
Also, we would need to cross connect Tx and Rx (Transmitter and Receiver) Pins for ESP32 and AQI sensor. 

# PCB Design – 
The PCB was designed using EagleCAD. 
If the reader has a background in electronics engineering, one can learn PCB design within 3-4 days. 
One can learn PCB design basics at - https://github.com/ayush-agarwal-0502/Electronics-Guidance-Freshers/blob/main/Digisim%20-%20PCB%20Design%20Workshop.pdf

There is a schematic file (containing the circuit diagram), a board file (containing the routing), and Gerber File (what we typically send to manufacturers so that the PCB gets printed). 
The PCB can be visualized at Online Gerber Viewer - https://www.pcbway.com/project/OnlineGerberViewer.html

I have uploaded the PCB files in this repository. 
I have used Female Pinheads – so that the team assembling it can simply take our PCB, buy ESP, AQI sensor, motor etc , and then its simple plug and play. We cannot expect our end customers to be experts in soldering, hence the need for PCB. Doing this ensures even non electronics people can assemble the chip. 

<img width="359" height="431" alt="image" src="https://github.com/user-attachments/assets/b7970c39-6189-4d8a-9ba1-5b08d25cd70c" />

<img width="336" height="434" alt="image" src="https://github.com/user-attachments/assets/5cdf546e-b4cc-496b-bfa7-00086c4f0b4a" />


# Quick start on PCB design – 

* Install EagleCAD. 
* Make the circuit diagram on the schematic file (use “Add Parts” to add parts, use “Net” to add the wires, and you may need to manually add library for ESP32 so that the pins correspond perfectly to the ESP. Also, ports for devices can be added using “Pinhead” part, since these would have to be connected to other chips. 
* Make the board file (top bar – left mid side). Place the parts on the chip. Take care that the ports etc should be on the edges, just like in real chip, imagine as if the other chips were going to be plugged into it, and so leave the gaps accordingly. 
* Add holes in the corners, so that later when the PCB comes we have holes to screw it in. 
* Use autorouter to route the traces (traces are copper lines on PCB which carry the current). Change the trace thickness to 20 mils or more before running the autorouter to avoid heating (mils are a unit of measurement for PCBs).
* Use autorouter to route the traces (traces are copper lines on PCB which carry the current). Change the trace thickness to 20 mils or more before running the autorouter to avoid heating (mils are a unit of measurement for PCBs).
Type POLYGON GND in the command line (top side white textbox), and draw rectangle around the chip, on both top and bottom layer, and declare signal name as GND. Once done, type RATSNEST. This will cover the remaining area with copper, to provide cooling to the chip. 
(Side note – you can use “RIPUP @;” command to remove the ratsnest.) 
* Go to tPlace (layer 21 or something) and use Text tool to add white coloured text to the chip, make sure to label all the pins nicely, as it will help in the assembly. 
* Run ERC and DRC in the command line, these are Electrical Rule Checks and Design Rule checks. If DRC fails, you may be required to manually add traces, and remove the autorouter ones. 
* Go to file, CAM, and make sure of these options – Export as ZIP, convert all Text to CSV etc , and then hit export, and it will generate the Gerber File. 
* Go to the “Assembly” folder in the generated excel file, add remarks, and add instructions to solder Female headers whereever required. 
* Also, right side, there is option to export picture of the PCB, use that instead of taking screenshot. 



# Manufacturing and Supply Chain design - 

Manufacturing is a different beast in itself, I'll keep adding insights here - 

Firstly, we have to look for PCB Manufacturers. 
We dont look for just PCB manufacturers, rather, the correct industry term is PCB turnkey manufacturers, or PCBA (PCB assembly). 
Since we as a social impact group do not have the liberty to hire someone full time to solder components, it is essential that the female connectors, vooltage regulator, barrel jack etc come pre soldered onto the PCB. 

PCBWay is popular, however it is China based, thus additional costs in taxes due to border crossing, therefore I did not go for it. Moreover, "Made in India" would be better. 

I found 2 nice manufactureres - Lion Circuits and PCBPower. Finally we went with LionCircuits as the quote was far lower. 

The manufacturers would require the Gerber Files, and the other files that EagleCAD already produces. 
Additionally, MPN (Manufacturer Part Number) is required for all the components that need to be soldered upon the PCB, these can be verified properly from Mouser Website. 


In manufacturing, we have a concept of MOQ (Minimum Order Quantity). Most manufactureres wont manufacture below 5 pcbs at a time. Also, producing lesser pcbs has high cost per pcb, whereas when we produce at large scale, the cost per pcb decreases significantly. 

Additionally, we have to source the parts (Gaming Fan, NodeMCU ESP32, AQI sensor etc) so that we can order easily when we reach production. 

<img width="500" height="98" alt="image" src="https://github.com/user-attachments/assets/0757c0a9-482e-47aa-9bea-fe500840e2d4" />


