import serial

port = '/dev/ttyACM4'
baudrate = 115200

with open("tempntc1.txt", "w", buffering=1) as file2:
    with open("tempntc2.txt", "w", buffering=1) as file3:
        with serial.Serial(port, baudrate, timeout = 1) as ser:
            while True:
                temp = str(ser.readline().decode('utf-8').strip())
                print(temp)
                if "NTC1" in temp:
                    file2.write(temp[7:] + '\n')
                if "NTC2" in temp:
                    file3.write(temp[7:] + '\n')
