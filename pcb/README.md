---------------
Terms of use:
---------------
The design is provided "AS IS", without any warranty of any kind, express or implied, including but not limited to the warranties of merchantability, fitness for a particular purpose and non-infringement. In no event shall the authors or copyright holders be liable for any claim, damages or other liability, whether in an action of contract, tort or otherwise, arising from, out of or in connection with the design or the use or other dealings this product.

---------------
PCB Use
---------------
This PCB was designed to work with the SensESP Sensor framework specifically for the use with I2C or OneWire sensors. Whilst the design allows for 3 sensors to be connected on the same PCB, when using I2C only one sensor is supported by SensESP. When using OneWire, multiple sensors can be used on one PCB. 

---------------
PCB Notes
---------------

Works best with Wemos-D1-Mini PRO because the USB connector sits at the top so even if mounted straight on the board, you have enough space to connected a 90-degree USB connector ("Up Angled")

Power can be supplied by:
- USB to Wemos
- Through Buck Converter (MP1584, $2.00 on ebay)
- 5V to P7

Sensor ports:
- J5 (VCC,GND,D1,D2) is connected through the 8 switch DIP to 3 4-pin headers (J2,J3,J4) which all have VCC,GND,D1 and D2 which are connected in parallel (see dip switches below)

OneWire
- For OneWire, I suggest you use the D1 Pin on the Wemos. So if you are starting with the sample code use in your SensESP code:

  DallasTemperatureSensors* dts = new DallasTemperatureSensors(D1);

Dip Switch:
- D1 - 5V to Wemos Chip On/Off. Using this allows you measure adjust the voltage of your buck converter. When doing so - note that the Capacitor (will need charging / de-charging for every adjustment made)
- D2 - Supply 3.3v to VCC on above 3 headers
- D3 - Supply 5v to VCC on above header
- D4 - LED on/off
- D5-D8: "Open/Close" J5-> J2,J3,J4 connections. The through here is that if you have a sensors where VCC, GND, D1 (SCL) and D2 (SCA) don't line up to this order, you close the switch and modify the order by (and wiring) J5 to J3 in a different order

Resistors:
R1: 330k for LED
R2: 4.7k for One Wire (not used for I2C)

Diodes:
3 x IN5817 Schottky diodes to protect: 
- (D6) JP7, 5v going back to MP1584
- (D4) 5v going back to 3.3v pin on Wemos
- (D5) 3.3v going back to 5v pin on Wemos

D4+D5 are there protect from SW1 and SW2 both being open at the same time

Capacitor
C3: 470uF / 25v to regulate supply to Wemos. 

---------------
Other Parts (all prices correct at time of writing, June 2020)
---------------

- Plastic project boxes, bought on eBay (picture enclosed): 5 boxes for £6.00 (£1.20 each)
- Mounting Screws: M3 x 6mm
- Black Plastic Holder Diode / Holder LED Clip Bezel Mount: 3mm or 5mm (depending on LED size): eBay: £0.10 each
- AWG20 Cable for LED + Power
- Heat String for LED
- Dupont Connectors for LED to PCB connection

---------------
Manufacturing
---------------

- I used JLCPCB but there are many options
- Uploaded (Zipped) Gerber directory to manufacturer to order product
- I paid $2.00 for 5 PCB's plus (cheapest shipping option) $6.29 Shipping. Total $8.29 / 5 = $1.66 per PCB (excluding parts)
- They arrived within 2 weeks

---------------
Directory Content
---------------

Gerbers/ 		Directory with individual Gerber Files
Gerbers.zip		Zipped Gerber Directory - used to upload to manufacturer
Manufacturing.pdf	Sample document to show order details
PCB Empty and Build.jpg	Screenshot of mounted and empty pcb
PlasticBox.png		Screenshot of the box used to mount the PCB
README.txt		This file
Bill-of-Materials.pdf	List of components used on the PCB
SensESP.fzz		Fritzing file with the PCB Bread boad, Schematic and PCB design



