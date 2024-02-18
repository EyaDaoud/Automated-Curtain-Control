import tkinter as tk
from tkinter import simpledialog
from datetime import datetime
import paho.mqtt.publish as publish
import paho.mqtt.client as mqtt
from tkinter import Scale

# Function to handle LDR value change
def ldr_value_changed(value):
   ldr_value_label.config(text=f"Luminosité voulue: {value}")
   publish_command("lum",value)

def publish_message(topic,message):
    # Get values from textboxes
    topic = topic_entry.get()
    message = message_entry.get()
    host = host_entry.get()

    # Publish the message using the HiveMQ public broker
    publish.single(topic, message, hostname=host)

def on_subscribe(client, userdata, mid, granted_qos):
    print("Subscribed with QoS", granted_qos[0])


window = tk.Tk()
window.title("Système de rideaux automatisés")

curtain_state = tk.StringVar()
is_room_bright = tk.IntVar()

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



def subscribe_topic():
    subscribe_topic = subscribe_entry.get()
    subscribe_client.subscribe(subscribe_topic)
                          
    
def publish_command(topic,command):
    # Envoyer la commande au broker MQTT
    publish.single(topic, command, hostname="broker.hivemq.com")  # Remplacez "broker.example.com" par l'adresse de votre broker MQTT


def toggle_auto_mode():
    if auto_mode.get():
        # Si le bouton est coché, publier "ON" sur le topic "auto"
        publish_command("auto","ON")
    else:
        # Si le bouton est décoché, vous pouvez choisir de publier quelque chose d'autre ou ne rien publier
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
    new_opening_time = simpledialog.askstring("Modifier l'heure d'ouverture", "Entrez la nouvelle heure d'ouverture (HH:MM)")
    if new_opening_time:
        print("Nouvelle heure d'ouverture:", new_opening_time)
        publish_command("time_ouv",new_opening_time)
        

def update_closing_time():
    new_closing_time = simpledialog.askstring("Modifier l'heure de fermeture", "Entrez la nouvelle heure de fermeture (HH:MM)")
    if new_closing_time:
        print("Nouvelle heure de fermeture:", new_closing_time)
        publish_command("time_ferm",new_closing_time)

def update_status():
    current_time = datetime.now().strftime("%H:%M:%S")
    current_date = datetime.now().strftime("%Y-%m-%d")
    status_label.config(text=f"Current Time: {current_time}\nCurrent Date: {current_date}")

def update_curtain_status():
    curtain_status_label.config(text=f"État du Rideau: {curtain_state.get()}")

def update_brightness_status():
   
    
    if is_room_bright.get() < 800:
        br = "faible"
    elif is_room_bright.get() >= 800 and is_room_bright.get() <2000:
        br ="moyenne"
    elif is_room_bright.get() >= 2000:
        br ="élevé"
        
    brightness_status_label.config(text=f"Luminosité de la pièce: {br}: {is_room_bright.get()}")
   
def switch_to_manual():
    mode.set("manuel")

# Create main window
#window = tk.Tk()
#window.title("Système de rideaux automatisés")

# Set background color to nude
window.configure(bg="#FFE4C4")  # Nude color for the background▬

# Global variables
#curtain_state = "fermé"  # Initial state

# Create mode variable
mode = tk.StringVar()
mode.set("manuel")  # Initial mode

# Create widgets
title_label = tk.Label(window, text="Système de rideaux automatisés", font=("Helvetica", 16, "bold"), bg="#FFE4C4", fg="#9E4244")

status_label = tk.Label(window, text="", font=("Helvetica", 12), justify='left', bg="#FFE4C4", fg="#333333")
update_status()

mode_label = tk.Label(window, text="Mode:", font=("Helvetica", 12, "bold"), bg="#FFE4C4", fg="#333333")

auto_mode = tk.BooleanVar()
auto_mode.set(False)

mode_switch = tk.Checkbutton(window, text="Automatique", variable=auto_mode, command=toggle_auto_mode, bg="#FFE4C4", fg="#333333", font=("Helvetica", 12))

manual_control_label = tk.Label(window, text="Contrôle Manuel", font=("Helvetica", 12, "bold"), bg="#9E4244", fg="#FFFFFF")
manual_control_label.grid(row=3, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

open_button = tk.Button(window, text="Ouvrir Rideaux", command= open_curtains , bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold"))  
close_button = tk.Button(window, text="Fermer Rideaux", command= close_curtains, bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold")) 

auto_schedule_title = tk.Label(window, text="Configuration de l'horaire automatique", font=("Helvetica", 12, "bold"), bg="#9E4244", fg="#FFFFFF")
auto_schedule_title.grid(row=5, column=0, columnspan=2, sticky="ew", padx=10, pady=5)

configure_opening_button = tk.Button(window, text="Modifier Heure d'ouverture", command=update_opening_time, bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold"))  
configure_closing_button = tk.Button(window, text="Modifier Heure de fermeture", command=update_closing_time, bg="#FFFFFF", fg="#333333", font=("Helvetica", 12, "bold"))  

curtain_status_label = tk.Label(window, text="", font=("Helvetica", 12), justify='left', bg="#FFE4C4", fg="#333333")
update_curtain_status()

brightness_status_label = tk.Label(window, text="", font=("Helvetica", 12), justify='left', bg="#FFE4C4", fg="#333333")
update_brightness_status()


ldr_scale = Scale(window, from_=0, to=4031, orient='horizontal', font=("Helvetica", 12), length=200, showvalue=0, command=ldr_value_changed)

ldr_value_label = tk.Label(window, text="Luminosité voulue: ", font=("Helvetica", 12, "bold"), bg="#FFE4C4", fg="#333333")
ldr_value_label.grid(row=10, column=0, padx=(5, 10), pady=10, sticky="n")

# Arrange widgets using grid
title_label.grid(row=0, column=0, columnspan=2, padx=10, pady=10)

status_label.grid(row=1, column=0, columnspan=2, padx=10, pady=10)

mode_label.grid(row=2, column=0, padx=10, pady=5)
mode_switch.grid(row=2, column=1, padx=10, pady=5)

manual_control_label.grid(row=3, column=0, columnspan=2, padx=10, pady=5)

open_button.grid(row=4, column=0, padx=10, pady=5)
close_button.grid(row=4, column=1, padx=10, pady=5)

auto_schedule_title.grid(row=5, column=0, columnspan=2, padx=10, pady=5)

configure_opening_button.grid(row=6, column=0, padx=10, pady=5)
configure_closing_button.grid(row=6, column=1, padx=10, pady=5)

curtain_status_label.grid(row=7, column=0, columnspan=2, padx=10, pady=10)
brightness_status_label.grid(row=8, column=0, columnspan=2, padx=10, pady=10)

ldr_scale.grid(row=10, column=1,columnspan=2, padx=(5, 10), pady=10, sticky="n")

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
subscribe_client.subscribe("ldr")

# Start the MQTT loop for the subscriber client
subscribe_client.loop_start()


# Start the GUI event loop
window.mainloop()
