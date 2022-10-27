
The microPython firmware may be installed - flashed with the following scripts:
for ESP32:

esptool.py --chip esp32 --port /dev/ttyUSB0 erase_flash

esptool.py --chip esp32 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x1000 $1

for ESP32C3:

esptool.py --chip esp32c3 --port /dev/ttyUSB0 erase_flash

#esptool.py --chip esp32c3 --port /dev/ttyACM0 --baud 460800 write_flash -z 0x0 $1

esptool.py --chip esp32c3 --port /dev/ttyUSB0 --baud 460800 write_flash -z 0x0 $1

for ESP32S3:

esptool.py --chip esp32s3 --port /dev/ttyACM0 erase_flash

esptool.py --chip esp32s3 --port /dev/ttyACM0 --baud 460800 write_flash -z 0x0 $1


**$1**  is the argument-name of the firmware file


