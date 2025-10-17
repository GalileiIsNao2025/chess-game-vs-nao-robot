# Nao Chess Robot – Physical Chessboard Interaction

## Overview
This project enables the **Nao humanoid robot** to play chess against a human opponent on a **real physical chessboard**.  
The robot communicates with a **Flask server** running the Stockfish chess engine, and with an **Arduino MKR WiFi board** that controls a robotic arm to move the pieces.

The system integrates **AI-based gameplay**, **robotic motion**, and **speech interaction**, providing a fully interactive human–robot chess experience.

---

## System Architecture

```
+------------------+          +-------------------+          +-----------------+
|  Web Interface   | <------> |  Flask + Stockfish | <------> |  Nao Robot      |
|  (User Moves)    |          |  (Game Logic API) |          |  (Choregraphe)  |
+------------------+          +-------------------+          +-----------------+
                                                           |
                                                           | TCP Socket
                                                           ▼
                                                  +-----------------+
                                                  |  Arduino MKR    |
                                                  |  (Robotic Arm)  |
                                                  +-----------------+
```

---

## Components

### 1. Flask Server (Python)
- Hosts the chess game logic using the **Stockfish** engine.
- Provides API endpoints for move exchange between human, Nao, and AI.
- Manages game states (checkmate, draw, invalid moves).
- Optionally serves a **web interface** for visualization and control.

**Main dependencies:**
```bash
pip install flask python-chess
```

**Run:**
```bash
python server.py
```

---

### 2. Nao Robot (Choregraphe Python Script)
- Communicates with the Flask server via HTTP requests.
- Sends move commands to the **Arduino MKR** over TCP.
- Uses **ALTextToSpeech** to describe moves verbally.
- Detects game progress and announces results.

**Configurable parameters:**
```python
NAO_IP = "127.0.0.1"          # Replace with actual Nao IP
MKR_IP = "192.168.x.x"        # Arduino MKR IP
FLASK_SERVER_URL = "http://192.168.x.x:5000"
```

**Main actions:**
- `get_user_move()` — retrieves the human move from the server.  
- `get_stockfish_move()` — requests the AI move.  
- `invia_comando_mkr()` — sends movement instructions to the robotic arm.  
- `parla()` — makes Nao speak the move aloud.

---

### 3. Arduino MKR WiFi
- Connects to the same WiFi network as Nao and the Flask server.
- Listens on TCP port **5005** for commands in the form `label:move`  
  (e.g., `nao:e2e4` or `user:e7e5`).
- Forwards commands to a serially connected **robotic arm** or actuator system.
- Sends back acknowledgments (`OK: move`).

**Dependencies:**
- `WiFiNINA` library (for WiFi communication)
- Serial communication with the robotic arm controller

**Example message flow:**
```
Flask → Nao → Arduino MKR → Dobot Arm
```

---

### 4. Robotic Arm
- Receives serial commands from the MKR board to physically move chess pieces.
- Executes calibrated motion sequences for each chess coordinate.
- Provides confirmation feedback to MKR once the move is complete.

---

## How to Run the System

1. **Set up Flask Server**
   - Edit `server.py` and configure the Stockfish path.
   - Run the server and verify it listens on port 5000.

2. **Connect Nao**
   - Load the Choregraphe script.
   - Update IP addresses for Nao, MKR, and Flask server.
   - Run the main script to start interaction.

3. **Configure Arduino MKR**
   - Flash the provided `MKR_WiFi.ino` sketch.
   - Connect to the WiFi network.
   - Verify IP address (shown via serial monitor).

4. **Start the Game**
   - Open the web interface or physical board.
   - Human makes the first move.
   - Nao announces the move and responds with its own.
   - The robotic arm moves the pieces accordingly.

---

## Repository Structure

```
/
├── server/                 # Flask + Stockfish server code
├── nao/                    # Choregraphe Python scripts
├── arduino/                # Arduino MKR WiFi firmware
├── web/                    # Optional web interface files
└── README.md               # Project documentation
```

---

## Technologies Used
- **Naoqi SDK (Python)** – Robot control and speech
- **Flask (Python)** – Web and API framework
- **Stockfish Engine** – Chess AI backend
- **Arduino MKR WiFi 1010** – Networked microcontroller
- **Dobot Arm / Custom Arm** – Chess piece manipulation
- **WiFiNINA Library** – Network stack for Arduino

---

## License
This project is released under the **MIT License**, except for third-party components:
- **Stockfish** – licensed under **GPLv3**
- **NAOqi SDK** – proprietary by SoftBank Robotics

---

## Credits
Developed as part of a collaborative robotics project integrating AI and physical interaction.

**Contributors:**
- Nao Robotics Development Team
- Arduino Integration Engineers
- AI & Vision Research Unit
