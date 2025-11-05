import tkinter as tk
from tkinter import scrolledtext
import serial
import threading
import json
import time

word = ""

try:
    arduino = serial.Serial(port='COM10', baudrate=9600, timeout=1)
    time.sleep(2) 
except Exception as e:
    print(f"Error connecting to Arduino: {e}")
    arduino = None

root = tk.Tk()
root.title("Arduino Data Viewer")
root.geometry("400x400")

input_label = tk.Label(root, text="Enter the word:")
input_label.pack()

input_entry = tk.Entry(root, width=30)
input_entry.pack(pady=5)

def set_word():
    global word
    user_input = input_entry.get()
    word = user_input
    print(f"User input: {user_input}")

print_button = tk.Button(root, text="confirm", command=set_word)
print_button.pack(pady=5)

output_text = scrolledtext.ScrolledText(root, width=40, height=10)
output_text.pack(pady=10)

latest_sensor_value = tk.StringVar(value="N/A")

def add():
    global word
    if not word:
        print("Please enter a word first!")
        return

    # Try to load existing data or start new
    try:
        with open("data.json", "r") as f:
            data = json.load(f)
    except (FileNotFoundError, json.JSONDecodeError):
        data = {}

    sensor_value = latest_sensor_value.get()
    values = [int(x) for x in sensor_value.split(',') if x.strip()]

    # ✅ Create new key if missing
    if word not in data:
        data[word] = []

    data[word].append(values)

    with open("data.json", "w") as f:
        json.dump(data, f, indent=4)

    print(f"Added sensor value for '{word}': {values}")

add_button = tk.Button(root, text="ADD", command=add)
add_button.pack(pady=5)

def read_from_serial():
    global latest_sensor_value
    while True:
        if arduino and arduino.in_waiting > 0:
            data = arduino.readline().decode('utf-8').strip()
            if data:
                latest_sensor_value.set(data)
                output_text.insert(tk.END, data + '\n')
                output_text.see(tk.END)
        time.sleep(0.1)

if arduino:
    thread = threading.Thread(target=read_from_serial, daemon=True)
    thread.start()

root.mainloop()

if arduino:
    arduino.close()
