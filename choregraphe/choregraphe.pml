import socket
import json
import time
import urllib2
from naoqi import ALProxy

# ==============================================================
# CONFIGURAZIONE
# ==============================================================
NAO_IP = "127.0.0.1"    # sostituire con IP di NAO
NAO_PORT = 9559

MKR_IP = "10.24.75.180"    # sostituire con IP di Arduino MKR
MKR_PORT = 5005

FLASK_SERVER_URL = "http://10.24.75.76:5000"    # sostituire con url server Flask

# ==============================================================
# Proxy Text-to-Speech
# ==============================================================
tts = ALProxy("ALTextToSpeech", NAO_IP, NAO_PORT)

def parla(msg):
    print("[CHOREGRAPHE]", msg)
    try:
        tts.say(msg)
    except:
        pass

def descrivi_mossa(mossa):
    if "x" in mossa:
        partenza, arrivo = mossa.split("x")
        return "cattura da {} a {}".format(partenza, arrivo)
    elif len(mossa) == 4:
        return "da {} a {}".format(mossa[:2], mossa[2:])
    return mossa

def invia_comando_mkr(etichetta, mossa):
    try:
        comando = "{}:{}".format(etichetta, mossa)
        s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        s.connect((MKR_IP, MKR_PORT))
        s.sendall((comando + "\n").encode("utf-8"))
        risposta = s.recv(1024).decode("utf-8")
        s.close()
        print("[CHOREGRAPHE] Comando inviato:", comando)
        return risposta
    except Exception as e:
        parla("Errore nel collegamento con il microcontrollore")
        return "errore"

def get_user_move():
    try:
        response = urllib2.urlopen(FLASK_SERVER_URL + "/get_user_move")
        result = json.loads(response.read())
        return result.get("move", None)
    except:
        return None

def post_user_move_to_server(move):
    try:
        data = json.dumps({"uci": move.replace("x",""), "formatted": move})
        req = urllib2.Request(FLASK_SERVER_URL + "/post_user_move", data=data)
        req.add_header("Content-Type", "application/json")
        urllib2.urlopen(req)
    except:
        pass

def get_stockfish_move():
    try:
        response = urllib2.urlopen(FLASK_SERVER_URL + "/getmove")
        result = json.loads(response.read())
        return result
    except:
        return None

def confirm_stockfish_move(move):
    try:
        data = json.dumps({"uci": move["move"], "formatted": move["formatted"]})
        req = urllib2.Request(FLASK_SERVER_URL + "/confirm_move", data=data)
        req.add_header("Content-Type", "application/json")
        urllib2.urlopen(req)
    except:
        pass

def is_game_over():
    try:
        response = urllib2.urlopen(FLASK_SERVER_URL + "/is_game_over")
        result = json.loads(response.read())
        return result.get("game_over", False), result.get("result", "")
    except:
        return False, ""

def is_new_move(moves_list, move):
    return move.replace("x","") not in [m.replace("x","") for m in moves_list]

# ==============================================================
# MAIN LOOP
# ==============================================================
def main():
    parla("Ciao, sono pronto a giocare.")
    moves = []

    while True:
        parla("Aspetto la tua mossa.")
        user_move = None
        for _ in range(600):
            new_move = get_user_move()
            if new_move and is_new_move(moves, new_move):
                user_move = new_move
                break
            time.sleep(0.3)  # Polling meno aggressivo

        if not user_move:
            parla("Non ho ricevuto una nuova mossa. Termino qui.")
            break

        uci_user = user_move.replace("x","")
        post_user_move_to_server(user_move)
        moves.append(uci_user)  # Aggiungo solo dopo conferma server
        parla("Hai mosso {}".format(descrivi_mossa(user_move)))
        invia_comando_mkr("utente", user_move)

        game_over, result = is_game_over()
        if game_over:
            parla("Hai vinto per scaccomatto!" if result=="scaccomatto" else "La partita è patta.")
            break

        parla("Ora tocca a me.")
        sf_move = get_stockfish_move()
        if not sf_move or not is_new_move(moves, sf_move["formatted"]):
            parla("Errore nel calcolo della mia mossa. Termino qui.")
            break

        moves.append(sf_move["move"])
        parla("Muovo {}".format(descrivi_mossa(sf_move["formatted"])))
        invia_comando_mkr("nao", sf_move["formatted"])
        confirm_stockfish_move(sf_move)

        game_over, result = is_game_over()
        if game_over:
            parla("Ho dato scaccomatto, ho vinto!" if result=="scaccomatto" else "Patta!")
            break

    parla("Grazie per la partita!")

if __name__ == "__main__":
    main()
