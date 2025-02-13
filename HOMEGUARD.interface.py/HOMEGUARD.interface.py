import tkinter as tk
from tkinter import messagebox
import serial
import threading

# Configurar porta serial
arduino = serial.Serial('COM5', 9600, timeout=1)  # Substitua 'COM8' pela porta correta do Arduino

# Variável global para rastrear o sensor ativo
active_sensor = None
current_data = []  # Buffer para armazenar os dados de corrente/tensão/energia

def read_arduino_data():
    """Lê os dados do Arduino e retorna como string."""
    if arduino.in_waiting > 0:
        try:
            data = arduino.readline().decode('utf-8', errors='ignore').strip()  # Ignora bytes inválidos
            return data
        except UnicodeDecodeError:
            print("Erro de decodificação: dados inválidos recebidos")
            return None
    return None

def update_sensor_data():
    """Atualiza continuamente os dados na interface com base no sensor ativo."""
    global active_sensor, current_data
    while True:
        data = read_arduino_data()
        if data:
            try:
                # Se o sensor ativo for "temperature" e a mensagem contiver "Temperatura"
                if active_sensor == "temperature" and "Temperatura" in data:
                    label_data["text"] = f"{data}"
                    
                    # Verifica se a temperatura excede 33 °C e adiciona alerta
                    if "Celsius:" in data:
                        try:
                            temp_celsius = float(data.split("Celsius:")[1].split()[0])
                            if temp_celsius > 33.00:
                                label_data["text"] += "\nALERTA!! ALTA TEMPERATURA!!"
                        except (ValueError, IndexError):
                            print("Erro ao processar temperatura")

                # Se o sensor ativo for "light" e a mensagem contiver "Luz"
                elif active_sensor == "light" and "Luz" in data:
                    # Valida se há ":" na string antes de dividir
                    label_data["text"] = f"Intensidade da Luz: {data}"

                # Se o sensor ativo for "current" e a mensagem contiver "Corrente", "Tensao" ou "Energia"
                elif active_sensor == "current" and ("Corrente" in data or "Tensao" in data or "Energia" in data):
                    current_data.append(data)  # Armazena as linhas de corrente/tensão/energia
                    if len(current_data) == 4:  # Exibe após acumular todos os dados
                        label_data["text"] = "\n".join(current_data)
                        current_data.clear()
            except IndexError:
                # Ignora mensagens malformadas
                print(f"Mensagem malformada recebida: {data}")

# Funções para cada sensor
def show_temperature_data():
    """Mostra e atualiza apenas os dados de temperatura."""
    global active_sensor
    active_sensor = "temperature"

def show_light_data():
    """Mostra e atualiza apenas os dados de luminosidade."""
    global active_sensor
    active_sensor = "light"

def show_current_data():
    """Mostra e atualiza apenas os dados de corrente/tensão/energia."""
    global active_sensor, current_data
    active_sensor = "current"
    current_data = []  # Inicializa o buffer para dados de corrente

# Configurar interface Tkinter
root = tk.Tk()
root.title("HOME GUARD")
root.geometry("400x300")
root.configure(bg="#00008b")

# Títulos e botões
tk.Label(root, text="Monitor Home Guard", fg="#ffffff", bg="#00008b", font=("Helvetica")).pack(pady=10)

frame_buttons = tk.Frame(root)
frame_buttons.pack(pady=100)
frame_buttons.configure(bg="#00008b")
button_temp = tk.Button(frame_buttons, text="Mostrar Temperatura", command=show_temperature_data, width=30, height=5, bg="#ffd700")
button_temp.grid(row=0, column=0, padx=10)

button_light = tk.Button(frame_buttons, text="Mostrar Luminosidade", command=show_light_data, width=30, height=5, bg="#ffd700")
button_light.grid(row=0, column=1, padx=10)

button_current = tk.Button(frame_buttons, text="Mostrar Corrente/Tensão", command=show_current_data, width=30, height=5, bg="#ffd700")
button_current.grid(row=0, column=2, padx=10)

# Área para exibir os dados
label_data = tk.Label(root, text="Dados dos Sensores aparecerão aqui", fg="#ffffff", bg="#00008b", wraplength=350, justify="center", font=("Helvetica", 12))
label_data.pack(pady=20)

# Thread para atualizar dados continuamente
thread = threading.Thread(target=update_sensor_data, daemon=True)
thread.start()

# Iniciar a interface Tkinter
root.mainloop()
