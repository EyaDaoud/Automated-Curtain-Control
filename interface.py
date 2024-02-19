import tkinter as tk
from tkinter import simpledialog
from datetime import datetime
import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
from tkinter import Scale


# Function to handle LDR value change
def ldr_value_changed(value):
   ldr_value_label.config(text=f"Brightness required: {value}")
   publish_command("lum",value)


# Function to publish in the broker
def publish_message(topic,message):
    topic = topic_entry.get()
    message = message_entry.get()
    host = host_entry.get()
    publish.single(topic, message, hostname=host)


def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed with QoS", granted_qos[0])


window = tk.Tk()
window.title("Automated Curtains Control")

curtain_state = tk.StringVar()
curtain_state.set("closed")
is_room_bright = tk.IntVar()
p = tk.StringVar()
m = tk.StringVar()
m.set("Automated Mode activated")
auto_mode = tk.BooleanVar()
auto_mode.set(True)


#callback function to read from the broker
def on_message(client, userdata, msg):
    
    global curtain_state
    global is_room_bright
    
    if msg.topic == "etat" :
        curtain_state.set(msg.payload.decode("utf-8"))
        update_curtain_status()
        
    elif msg.topic == "lum" :
        received_value = int(msg.payload.decode("utf-8"))
        is_room_bright.set(received_value)
        update_brightness_status()
        
    elif msg.topic == "possible" :
       p.set (msg.payload.decode("utf-8"))
       update_poss()

    elif msgtopic == "mode" :
       m.set (msg.payload.decode("utf-8"))
       update_mode()
       
      
# Function to subscribe to topics
def subscribe_topic():
    subscribe_topic = subscribe_entry.get()
    subscribe_client.subscribe(subscribe_topic)

               
# Function to publish in the MQTT broker    
def publish_command(topic,command):
    publish.single(topic, command, hostname="broker.hivemq.com")  # Remplacez "broker.example.com" par l'adresse de votre broker MQTT


# Function to activate automatic mode
def toggle_auto_mode():
    if auto_mode.get():
        publish_command("auto","ON")
    else:
        pass


def open_curtains ():
    publish_command("manuell","ON")
    auto_mode.set(False)


def close_curtains():
    publish_command("manuell","OFF")
    auto_mode.set(False)
    
   
def set_auto_schedule():
    print("Auto schedule set")


def update_opening_time():
    new_opening_time = simpledialog.askstring("Edit Opening Hour", "Entrez la nouvelle heure d'ouverture (HH:MM)")
    if new_opening_time:
        print("Nouvelle heure d'ouverture:", new_opening_time)
        publish_command("time_ouv",new_opening_time)
        

def update_closing_time():
    new_closing_time = simpledialog.askstring("Edit Closing Hour", "Entrez la nouvelle heure de fermeture (HH:MM)")
    if new_closing_time:
        print("Nouvelle heure de fermeture:", new_closing_time)
        publish_command("time_ferm",new_closing_time)


def update_status():
    current_time = datetime.now().strftime("%H:%M:%S")
    current_date = datetime.now().strftime("%Y-%m-%d")
    status_label.config(text=f"Current Time: {current_time}\nCurrent Date: {current_date}")


def update_curtain_status1():
    curtain_status1_label.config(text=f"Curtains state:")
    

def update_curtain_status():
    curtain_status_label.config(text=f"{curtain_state.get()}")


def update_brightness_status():
    if is_room_bright.get() < 800:
        br = "Low"
    elif is_room_bright.get() >= 800 and is_room_bright.get() <2000:
        br ="Meduim"
    elif is_room_bright.get() >= 2000:
        br ="High"
        
    brightness_status_label.config(text=f" {br} : {is_room_bright.get()}")


def update_brightness_status1():
   brightness_status1_label.config(text=f"Room Brightness : ") 


def update_poss() :
   possible_label.config(text=f"{p.get()}")

   
def update_mode() :
   mod_label.config(text=f"{m.get()}")

   
def switch_to_manual():
    mode.set("manuel")


# Set background color 
window.configure(bg="#FFE4C4")


# Create mode variable
mode = tk.StringVar()
mode.set("manuel")  

# Create widgets
title_label = tk.Label(window, text="Automated Curtains Control", font=("Helvetica", 16, "bold"), bg="#FFE4C4", fg="#9E4244")

status_label = tk.Label(window, text="", font=("Helvetica", 12), justify='left', bg="#FFE4C4", fg="#333333")
update_status()

mode_label = tk.Label(window, text="Mode:", font=("Helvetica", 12, "bold"), bg="#FFE4C4", fg="#333333")

auto_mode = tk.BooleanVar()
auto_mode.set(False)

mode_switch = tk.Checkbutton(window, text="Automatic", variable=auto_mode, command=toggle_auto_mode, bg="#FFE4C4", fg="#333333", font=("Helvetica", 12))
mode_switch.select()

manual_control_label = tk.Label(window, text="Manual Control", font=("Helvetica", 12, "bold"), bg="#9E4244", fg="#FFFFFF")


open_button = tk.Button(window, text="Open Curtains", command= open_curtains , bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold"))

close_button = tk.Button(window, text="Close Curtains", command= close_curtains, bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold")) 

auto_schedule_title = tk.Label(window, text="Schedule Configuration", font=("Helvetica", 12, "bold"), bg="#9E4244", fg="#FFFFFF")


configure_opening_button = tk.Button(window, text="Edit Opening Hour", command=update_opening_time, bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold"))

configure_closing_button = tk.Button(window, text="Edit Closing Hour", command=update_closing_time, bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold"))  

curtain_status1_label = tk.Label(window, text="", font=("Helvetica", 12, "bold"), justify='left', bg="#FFE4C4", fg="#333333")
update_curtain_status1()

curtain_status_label = tk.Label(window, text="", font=("Helvetica", 12), justify='left', bg="#FFE4C4", fg="#333333")
update_curtain_status()

brightness_status1_label = tk.Label(window, text="", font=("Helvetica", 12, "bold"), justify='left', bg="#FFE4C4", fg="#333333")
update_brightness_status1()

brightness_status_label = tk.Label(window, text="", font=("Helvetica", 12), justify='left', bg="#FFE4C4", fg="#333333")
update_brightness_status()

possible_label = tk.Label(window, text="", font=("Helvetica", 12), justify='left', bg="#FFE4C4", fg="#333333")
update_poss()

mod_label = tk.Label(window, text="", font=("Helvetica", 12, "bold"), justify='left', bg="#FFE4C4", fg="#333333")
update_mode()

ldr_scale = Scale(window, from_=0, to=4031, orient='horizontal', font=("Helvetica", 12), length=200, showvalue=0, command=ldr_value_changed)

ldr_value_label = tk.Label(window, text="Brightness Required: ", font=("Helvetica", 12, "bold"), bg="#FFE4C4", fg="#333333")

# Arrange widgets using grid
manual_control_label.grid(row=3, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

auto_schedule_title.grid(row=5, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

ldr_value_label.grid(row=11, column=0, padx=(90,0), pady=10, sticky="n")

title_label.grid(row=0, column=0, columnspan=2, padx=10, pady=10)

status_label.grid(row=1, column=0, columnspan=2, padx=10, pady=10)

mode_label.grid(row=2, column=0, padx=10, pady=5)

mode_switch.grid(row=2, column=1, padx=10, pady=5)

manual_control_label.grid(row=3, column=0, columnspan=2, padx=10, pady=5)

open_button.grid(row=4, column=0, padx=(0,0), pady=5)

close_button.grid(row=4, column=1, padx=(0,0), pady=5)

auto_schedule_title.grid(row=5, column=0, columnspan=2, padx=10, pady=5)

configure_opening_button.grid(row=6, column=0, padx=(0,0), pady=5)

configure_closing_button.grid(row=6, column=1, padx=(0,20), pady=5)

curtain_status1_label.grid(row=7, column=0, columnspan=1, padx=(170, 0), pady=10 )

curtain_status_label.grid(row=7, column=1, columnspan=1, padx=(0, 180), pady=10 )

brightness_status1_label.grid(row=8, column=0, columnspan=1, padx=(180, 0), pady=10)

brightness_status_label.grid(row=8, column=1, columnspan=1, padx=(0,180), pady=10)

mod_label.grid(row=9, column=0, columnspan=2, padx=10, pady=10)

possible_label.grid(row=10, column=0, columnspan=2, padx=10, pady=10)

ldr_scale.grid(row=11, column=1,columnspan=2, padx=(0,90), pady=10, sticky="n")

# Update status every second
window.after(1000, update_status)

# Bind buttons to switch to manual mode when clicked in automatic mode
open_button.config(command=lambda: [open_curtains(), switch_to_manual()])

close_button.config(command=lambda: [close_curtains(), switch_to_manual()])


# Set up MQTT client for subscribing
subscribe_client = mqtt.Client()
subscribe_client.on_subscribe = on_subscribe
subscribe_client.on_message = on_message
subscribe_client.connect("broker.hivemq.com", 1883, 60)
subscribe_client.subscribe("etat")
subscribe_client.subscribe("lum")
subscribe_client.subscribe("manuell")
subscribe_client.subscribe("auto")
subscribe_client.subscribe("time_ouv")
subscribe_client.subscribe("time_ferm")
subscribe_client.subscribe("possible")
subscribe_client.subscribe("mode")

# Start the MQTT loop for the subscriber client
subscribe_client.loop_start()


# Start the GUI event loop
window.mainloop()
