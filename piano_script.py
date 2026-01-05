import mido
import serial
import time

song = 'C:/Users/ssarr/Downloads/tchaikovsky_sleeping_beauty_act-1_6_valse_(c)yogore.mid'
swan_lake = 'C:/Users/ssarr/Downloads/tchaikovsky_swan_lake_act-1_2_valse_(c)yogore.mid'
lieb = 'C:/Users/ssarr/Downloads/liebestraume_3_(c)oguri.mid'
arab = 'C:/Users/ssarr/Downloads/debussy_63503a_arabesque_2_e_major_(nc)smythe.mid'
liebestraume = 'C:/Users/ssarr/Downloads/liebestraume_3_(c)oguri.mid'

serial_port = 'COM4'
baud_rate = 115200

ser = serial.Serial(serial_port, baud_rate)
time.sleep(2)

mid = mido.MidiFile(liebestraume)

for msg in mid.play():
    if msg.type == 'note_on' or msg.type == 'note_off':
        note = msg.note
        velocity = msg.velocity if msg.type == 'note_on' else 0
        message = f'{note}:{velocity}\n'
        ser.write(message.encode())

ser.close()