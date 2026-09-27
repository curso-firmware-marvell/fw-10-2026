import serial
import time
from colorama import Fore

with open("project_config.def") as f:
    #lines = [l.strip() for l in f if l.strip()]
    lines = [l.strip() for l in f ]
for line in lines: 
    if "ADDR_START_INST" in line :
        start_address = int((line.split())[2].replace("_",""),16)
    if "REG_MCU_RESET" in line :
        mcu_reset_address = int((line.split())[2].replace("_",""),16)

ser = serial.Serial(
    #port='/dev/ttyUSB0'
    port='COM14',
    baudrate=115200,
    timeout=1
)

to_send = f"W {hex(mcu_reset_address)[2:]} FF"
print("reseting mcu",to_send)
ser.write(f"{to_send}\n".encode('ascii'))

raw_data = []
with open('fw/fw.hex', 'r') as f:
    address = start_address
    for line in f:
        line = line.strip()
        if not line:
            continue
        
        data_val = int(line,16)

        raw_data.append({"add":address,"val":data_val})
        to_send = f"W {hex(address)[2:].upper()} {hex(data_val)[2:].upper()}"
        print(f"writing: {to_send}")
        ser.write(f"{to_send}\n".encode('ascii'))

        address += 4

print()
print(f"{Fore.YELLOW}-- verifying --{Fore.RESET}")



error_count = 0
for item in raw_data:
    to_send = f"R {hex(item['add'])[2:].upper()}"
    ser.write(f"{to_send}\n".encode('ascii'))

    response = ser.readline()
    response = response.decode(errors='ignore')
    add = int(response.split()[1],16)
    val = int(response.split()[2],16)

    if add != item["add"] or val != item["val"]:
        error_count += 1
        print()
        print(f"{Fore.RED}MISMATCH! expected: add={hex(item['add'])}, val={hex(item['val'])}{Fore.RESET}")
        print(f"{Fore.RED}          received: add={hex(add)        }, val={hex(val)        }{Fore.RESET}")

to_send = f"W {hex(mcu_reset_address)[2:]} 00"
print("des reseting mcu",to_send)
ser.write(f"{to_send}\n".encode('ascii'))

ser.close()


if error_count == 0: print(f"\n{Fore.GREEN}done! {                    Fore.RESET}")
else:                print(f"\n{Fore.RED  }error count: {error_count}{Fore.RESET}")