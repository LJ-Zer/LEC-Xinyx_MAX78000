# LEC-Xinyx_MAX78000 (LAB-EARTH + C Xinyx Unlocked 2024 ADI MAX78000 Source Codes) 
LAB-EARTH + C as a hardware itself is a seamless and compact version of a typical weather station which features relevant and highly-demand systems, providing information about the temperature, humidity, atmospheric pressure, wind speed, wind direction, rain predictions, air quality, and cloud formations.

## The following folders are the modified source codes from the libraries of Maxim SDK.
LAB-EARTH + C is a project contains a feature of "Cloud Sensing", basically it uses neural network or a machine vision to determine different types of clouds, which is deployed in ADI MAX78000 MCU. Since this is unfinished project, the developer leverage this kind of data transmission , the data should be transmit using LoRa module. A lot of things should be consider in transmitting using LoRa module, first is the librarry used, the source code from LoRa should be translated into source code that MAX78000 accepts. Second, the data transmission capability of LoRa, in this case we leverage the SX1262 and chunking type of tranmission should be leverage since transmitting an image is too complex to do.

## Image-Decoder folder:
This folder contains several scripts from processing a grayscale Hexadecimal data from MAX78000 MCU, serial decoder, data cleaner, up to transmission using GitHub uploader.

## Reference-1_LoRa folder:
This folder is one of the open source reference from other developer that leverage LoRa module to transmit data.

## cloud_types folder:
This folder is generated folder from Maxim SDK, the main script is already edited based on the continous image capturing while printing the hexadecimal data in grayscale. All the weights, and libraries that is used to embed the model are complete in this folder.


