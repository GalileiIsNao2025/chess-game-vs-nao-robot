# 🤖♟️ Nao Chess Robot – Scacchiera Fisica Interattiva

## 🧠 Descrizione del Progetto

Questo progetto realizza un **sistema di scacchi interattivo fisico** in cui:
- il robot **Nao** gioca contro un essere umano;
- le **mosse vengono eseguite realmente** su una **scacchiera fisica** tramite un **braccio robotico controllato da Arduino MKR**;
- il motore di gioco è **Stockfish**, integrato in un **server Flask** che gestisce logica e interfaccia;
- Nao comunica con il server e con il microcontrollore per gestire il flusso del gioco e **parlare** con l’utente.

Il sistema permette di giocare una **partita completa di scacchi fisici** tra uomo e robot, con interazione vocale e movimenti automatici dei pezzi.

---

## ⚙️ Architettura del Sistema

               ┌────────────────────┐
               │   Browser / UI     │
               │ (scacchiera web)   │
               └─────────┬──────────┘
                         │ (HTTP)
                         ▼
               ┌────────────────────┐
               │   Flask Server     │
               │ + Stockfish Engine │
               └─────────┬──────────┘
                         │ (HTTP)
                         ▼
               ┌────────────────────┐
               │   Nao Robot        │
               │ (Choregraphe Py)   │
               └─────────┬──────────┘
                         │ (TCP Socket)
                         ▼
               ┌────────────────────┐
               │  Arduino MKR WiFi  │
               │ + Braccio Robotico │
               └────────────────────┘

---

## 🧩 Componenti del Sistema

### 🧠 1. Nao Robot (modulo Choregraphe)
- Comunica con il **server Flask** via HTTP.
- Invia e riceve mosse (utente ↔ AI).
- Descrive a voce le mosse e lo stato della partita.
- Controlla l’**Arduino MKR** via socket TCP.
- Riconosce le mosse dell’utente e le conferma al server.

**File principale:** `nao_chess.py`

**Librerie:**  
`socket`, `json`, `time`, `urllib2`, `naoqi`

**Parametri da configurare:**
```python
NAO_IP = "127.0.0.1"
NAO_PORT = 9559
MKR_IP = "172.17.114.232"
MKR_PORT = 5005
FLASK_SERVER_URL = "http://172.17.114.76:5000"
```

---

### ⚙️ 2. Server Flask con Stockfish
- Gestisce la **logica della partita**.
- Tiene traccia dello stato della scacchiera.
- Interagisce con **Stockfish** per generare le mosse dell’IA.
- Espone API per Nao e per l’interfaccia web.

**API principali:**
| Endpoint | Metodo | Descrizione |
|-----------|---------|-------------|
| `/post_user_move` | POST | Riceve la mossa dell’utente |
| `/getmove` | GET | Restituisce la mossa di Stockfish |
| `/confirm_move` | POST | Conferma la mossa eseguita da Nao |
| `/get_user_move` | GET | Permette a Nao di leggere la mossa dell’utente |
| `/is_game_over` | GET | Verifica lo stato finale della partita |

**Requisiti:**
```bash
pip install flask python-chess
```

---

### 🔌 3. Arduino MKR WiFi + Braccio Robotico
- Riceve comandi via TCP dal robot Nao (porta 5005).
- Ogni comando rappresenta una **mossa fisica** da eseguire.
- Controlla un **braccio robotico** (es. Dobot Magician) per spostare i pezzi.
- Invia risposte di conferma al Nao dopo ogni mossa.

**Esempio di comando ricevuto:**
```
utente:e2e4
nao:e7e5
```

**Codice principale (Arduino MKR):**
```cpp
#include <WiFiNINA.h>

const char* ssid = "SSID_WIFI";
const char* password = "PASSWORD_WIFI";

WiFiServer server(5005);

void setup() {
  Serial.begin(9600);
  Serial1.begin(9600);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connessione in corso...");
  }

  Serial.print("IP MKR: ");
  Serial.println(WiFi.localIP());
  server.begin();
}

void loop() {
  WiFiClient client = server.available();
  if (client) {
    String msg = client.readStringUntil('\n');
    msg.trim();

    if (msg.length() > 0) {
      Serial.println("Ricevuto: " + msg);
      Serial1.print("#" + msg + ";\n"); // invia al braccio robotico
    }

    client.println("OK: " + msg);
    client.stop();
  }
}
```

---

### 🦾 4. Braccio Robotico
- Esegue materialmente le mosse ricevute dall’Arduino MKR.
- Può utilizzare un **gripper magnetico o pinza meccanica**.
- Muove pedine in coordinate predefinite (scacchiera 8x8 calibrata).
- Gestisce catture e depositi.

---

## 🧭 Flusso Operativo del Gioco

1. **Inizio:**  
   Nao saluta e chiede all’utente di iniziare la partita.

2. **Mossa dell’utente:**  
   - L’utente muove una pedina (rilevata via telecamera o interfaccia web).  
   - Il server Flask riceve la mossa e la invia a Nao.  
   - Nao la ripete a voce (“Hai mosso da e2 a e4”) e la inoltra al MKR.

3. **Mossa di Nao:**  
   - Il server elabora la risposta di **Stockfish**.  
   - Nao la annuncia (“Muovo da e7 a e5”).  
   - Arduino MKR riceve il comando e muove fisicamente il pezzo con il braccio robotico.

4. **Ripetizione:**  
   Il ciclo continua finché viene raggiunto **scaccomatto** o **patta**.

5. **Fine partita:**  
   Nao annuncia il risultato (“Scaccomatto! Ho vinto!” o “Partita patta!”).

---

## 💬 Comunicazioni tra Moduli

| Da → A | Tipo | Protocollo | Dati |
|----------|----------|-------------|------|
| Browser → Flask | HTTP | POST / GET | Mosse utente |
| Flask → Nao | HTTP | JSON | Mosse e stato partita |
| Nao → MKR | TCP Socket | Stringa | `nao:e7e5` |
| MKR → Braccio | Serial | Stringa formattata | `#nao:e7e5;` |
| MKR → Nao | TCP | OK / errore |

---

## 📦 Requisiti Tecnici

| Componente | Specifiche |
|-------------|-------------|
| Robot | Nao (Softbank Robotics) |
| Microcontrollore | Arduino MKR WiFi 1010 |
| Braccio robotico | Dobot Magician o simile |
| Motore scacchistico | Stockfish |
| Server | Python + Flask |
| Librerie | `flask`, `python-chess`, `WiFiNINA`, `naoqi` |

---

## 🚀 Avvio del Sistema

1. **Server Flask:**
   ```bash
   python server.py
   ```
   (controlla l’indirizzo IP e la porta 5000)

2. **Arduino MKR:**
   - Carica lo sketch WiFi.  
   - Verifica la connessione e l’IP nel Serial Monitor.

3. **Nao (Choregraphe):**
   - Imposta gli IP corretti di MKR e Flask nel file Python.  
   - Esegui lo script `nao_chess.py`.

4. **Browser:**
   - Apri l’interfaccia web (`http://<IP_FLASK>:5000`).  
   - Inizia a giocare!

---

## 🧾 Licenza

Questo progetto è rilasciato sotto **licenza MIT**, eccetto:
- **Stockfish**, distribuito sotto **GPLv3**;
- **NAOqi SDK**, soggetto alle licenze SoftBank Robotics.

---

## 👨‍💻 Autori

**Progetto realizzato da:**  
Andrea Peerani e team  

**Data:** Ottobre 2025  
**Versione:** 1.0  

---

## 📸 Demo (opzionale)

> *In questa configurazione, Nao annuncia le mosse, il braccio robotico muove i pezzi e la scacchiera fisica visualizza la partita in tempo reale.*
