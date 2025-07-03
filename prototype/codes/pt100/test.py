import serial

port = '/dev/ttyACM3'
baudrate = 115200

with open("temppt.txt", "w", buffering=1) as file:
    with serial.Serial(port, baudrate, timeout = 1) as ser:
        while True:
            temp = str(ser.readline().decode('utf-8').strip())
            print(temp)
            if "PT100" in temp:
                file.write(temp[8:] + '\n')
