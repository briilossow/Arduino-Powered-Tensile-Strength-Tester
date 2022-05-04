# Arduino-Powered Tensile Strength Tester

Designed for ECE442 - Systems Analysis & Design | University at Albany, SUNY  
Developers: Brii Lossow, Alex Karampelas, & Omar Elmejjati

## Repository Contents

**Reports**: Contains written reports that describe system results and how to replicate the system  
**Source Code:** Contains the code that implements system logic; please ensure necessary libraries are installed for best results  
**Presentations:** Contains materials used for presenting the system and its data  
**Miscellaneous:** Contains the STL files used for the clamps as well as a user's manual on operating the machine

## System Components

**Arduino Mega 2560**  
Part Number: A000067  
Cost: $40  
Purchase: https://www.amazon.com/dp/B0046AMGW0?psc=1&ref=ppx_yo2ov_dt_b_product_details

**Elegoo 2.8" TFT Touch Screen with SD Card Socket**  
Part Number: EL-SM-004  
Cost: $17  
Purchase: https://www.amazon.com/dp/B01EUVJYME?psc=1&ref=ppx_yo2ov_dt_b_product_details

**20kg Load Cell**  
Part Number: ADA-4543  
Cost: $4  
Purchase: https://www.adafruit.com/product/4543

**32GB microSD Card**  
Any microSD card can be used, but we used this one:  
Cost: $7  
Purchase: https://www.amazon.com/Lexar-Micro-microSDHC-Memory-Adapter/dp/B08XQ8V2QJ/ref=sr_1_14?crid=132OEXNERZRYN&keywords=32gb+microsd&qid=1651631882&sprefix=32gb+microsd%2Caps%2C78&sr=8-14

**SpiderWire 50lb Capacity Fishing Line**  
Cost: $13  
Purchase: https://www.amazon.com/Spiderwire-SCS15G-125-Braided-Stealth-Superline/dp/B00LDYMB04/ref=sr_1_5?crid=NG1XCG469RJC&keywords=spiderwire+fishing+line+50lb&qid=1651631935&sprefix=spiderwire+fishing+line+50lb%2Caps%2C64&sr=8-5

**3/32" Vinyl-Encased Steel Cable**  
Part Number: AC6000B  
Cost: $0.45 / ft, 3 ft used
Purchase: https://www.lowes.com/pd/Blue-Hawk-3-32-in-x-1-ft-Vinyl-Coated-Cable/1001250410

**1" Pulley**  
Part Number: N100-308  
Cost: $4  
Purchase: https://www.lowes.com/pd/National-Hardware-N100-308-V3201-1-in-Swivel-Single-Pulley-in-Nickel/1002255360

**9/16" Rope Loop**  
Part Number: N100-342  
Cost: $3  
Purchase: https://www.lowes.com/pd/National-Hardware-N100-342-2056-9-16-in-Rope-Loops-in-Stainless-Steel/1002256376

**Piezo Buzzer**  
Part Number: ADA-1536  
Cost: $1  
Purchase: https://www.adafruit.com/product/1536

## Software Dependencies

Note that no software dependencies are required to run the system, as it is self-contained and does not require a computer connection.

For reconstruction,  
**HX711 Arduino Library:** https://github.com/bogde/HX711  
**Adafruit GFX Library:** https://github.com/adafruit/Adafruit-GFX-Library  
**Adafruit TFTLCD Library:** https://github.com/adafruit/TFTLCD-Library  

*Note: The Elegoo & Touch Screen libraries are included on the disk that comes with the touch screen - we've added them here for convenient use*

**Elegoo GFX Library:** https://github.com/briilossow/Arduino-Powered-Tensile-Strength-Tester/blob/main/Miscellaneous/Elegoo%20Libraries/Elegoo_GFX.zip  
**Elegoo TFTLCD Library:** https://github.com/briilossow/Arduino-Powered-Tensile-Strength-Tester/blob/main/Miscellaneous/Elegoo%20Libraries/Elegoo_TFTLCD.zip  
**Touch Screen Library:** https://github.com/briilossow/Arduino-Powered-Tensile-Strength-Tester/blob/main/Miscellaneous/Elegoo%20Libraries/TouchScreen.zip  

If you run into problems using the SD card reader, install the Adafruit SD Library in place of the SD library included with the Arduino IDE to allow software SPI assignments: https://github.com/adafruit/SD
