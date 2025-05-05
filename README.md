# S8ARIM
STM8 based AutoRange Inductance Meter Project. single sided PCB. with SMD and DIP components.

# Images

Top:
![Alt text](Hardware/3D_View/S8ARIM_3D_Bottom.PNG?raw=true "Title")

Bottom:
![Alt text](Hardware/3D_View/S8ARIM_3D_Top.PNG?raw=true "Title")

# Advantages :
- Fine and High Precision
- Small Size
- Cheap to Build 
- Less Components
- AutoRange

# Specifications :
- Measure Inductance Range : 0.01uH to 999H
- Supply Voltage Range : 3 to 5V DC
- Currect Consumption : 40mA Max
- Measurement Rate : 3/sec for Lx < 500mH and 1/sec for Lx > 500mH

# Program the MCU on PCB
  the "J3" Connector is used for Program MCU (STM8S003F3) with ST-LINK Programmer and SWIM Protocol.
## J3 Pin Description :
  Pin 1 : GND   
  Pin 2 : SWIM  
  Pin 3 : RESET (RST)   
## Note :
  Your need Connect Power Supply to J1 for Programming. 

# Calibration :
  Calibration is required before the first use of the device.
- Connect Power Supply to Inductance Meter
- Connect Lx Probes to together
- Press the Inductance Meter Button
