# server.py
from flask import Flask, render_template, request, jsonify
import subprocess
import threading
import time
import os
import webbrowser

# =====================================================================
# Per la rilevazione del gameover usiamo python-chess (versione >= 1.7)
# =====================================================================
import chess

app = Flask(__name__)

# =====================================================================
# 1) Percorso a Stockfish: modificare in base al vostro PC
# =====================================================================
STOCKFISH_PATH = r"C:\Users\andre\OneDrive\Desktop\stockfish-windows-x86-64-avx2\stockfish\stockfish-windows-x86-64-avx2.exe"

last_user_move = None             # Ultima mossa formattata dall'utente (es. "e2xe4")
cached_stockfish_move = None      # Ultima mossa UCI pura di Stockfish (es. "b8c6")
cached_stockfish_formatted = None # Ultima mossa formatta di Stockfish (es. "b8xc6")
cached_moves_list = None          # Lista di mosse UCI (es. ["e2e4","e7e5",...])


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/post_user_move", methods=["POST"])
def post_user_move():
    """
    Riceve dal browser la mossa formattata dall’utente (es. "e2xe4" o "g1f3"),
    la salva in last_user_move, azzera la cache e la stampa in terminale.
    """
    global last_user_move, cached_stockfish_move, cached_stockfish_formatted, cached_moves_list
    move = request.json.get("move")  # es. "e2xe4" (o "e2e4" se nessuna cattura)
    if move == last_user_move:
        return jsonify({"status": "duplicate"})
    last_user_move = move
    cached_stockfish_move = None
    cached_stockfish_formatted = None
    cached_moves_list = None
    print("[SERVER] Mossa utente (formattata):", move)
    return jsonify({"status": "ok"})


@app.route("/get_user_move", methods=["GET"])
def get_user_move():
    """
    Restituisce l’ultima mossa formattata (es. "e2xe4") e poi la “consuma” (imposta a None).
    """
    global last_user_move
    move = last_user_move
    last_user_move = None
    return jsonify({"move": move})


@app.route("/getmove", methods=["POST"])
def get_best_move():
    """
    Genera la mossa di Stockfish:
      - Riceve una lista di mosse UCI pure (es. ["e2e4","e7e5",...]).
      - Se era già stata calcolata, restituisce il risultato in cache (UCI pura + formattata).
      - Altrimenti lancia Stockfish in UCI, imposta la posizione e chiede "go movetime 2000".
      - Quando Stockfish risponde "bestmove <uci>", ricava la mossa pura.
      - Con python-chess capisce se è cattura e costruisce la stringa formattata (es. "e7xe5").
      - Salva in cache e restituisce JSON {"move": "<uci>", "formatted": "<formattata>"}.
    """
    global cached_stockfish_move, cached_stockfish_formatted, cached_moves_list

    moves = request.json.get("moves", [])  # es. ["e2e4","e7e5",...]
    if moves == cached_moves_list and cached_stockfish_move:
        return jsonify({
            "move": cached_stockfish_move,
            "formatted": cached_stockfish_formatted
        })

    # Avviamo Stockfish in modalità UCI
    sf = subprocess.Popen(
        [STOCKFISH_PATH],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        universal_newlines=True
    )

    sf.stdin.write("uci\n")
    sf.stdin.write(f"position startpos moves {' '.join(moves)}\n")
    sf.stdin.write("go movetime 2000\n")
    sf.stdin.flush()

    best_move = None
    while True:
        line = sf.stdout.readline().strip()
        if line.startswith("bestmove"):
            best_move = line.split()[1]  # es. "b8c6"
            break

    sf.kill()

    # ========== Rileviamo con python-chess se la mossa è cattura ==========
    board = chess.Board()
    for uci in moves:
        board.push_uci(uci)
    move_obj = chess.Move.from_uci(best_move)
    if board.is_capture(move_obj):
        formatted = best_move[:2] + "x" + best_move[2:]
    else:
        formatted = best_move

    # Mettiamo in cache
    cached_stockfish_move = best_move
    cached_stockfish_formatted = formatted
    cached_moves_list = list(moves)

    print("[SERVER] Mossa Stockfish (UCI pura):", best_move)
    print("[SERVER] Mossa Stockfish (formattata):", formatted)
    return jsonify({
        "move": best_move,
        "formatted": formatted
    })


@app.route("/is_game_over", methods=["POST"])
def is_game_over():
    """
    Rileva scaccomatto/patta con python-chess:
      - Riceve lista di mosse UCI pure (es. ["e2e4","e7e5",...]).
      - Ricostruisce la Board e verifica is_checkmate(), is_stalemate(),
        is_insufficient_material(), can_claim_threefold, can_claim_fifty_moves.
      - Se finita, ritorna {"game_over": True, "result": "scaccomatto"} o {"result":"patta"}.
      - Altrimenti {"game_over": False, "result": ""}.
    """
    moves = request.json.get("moves", [])
    board = chess.Board()
    for uci in moves:
        board.push_uci(uci)

    game_over = False
    result = ""

    if board.is_checkmate():
        game_over = True
        result = "scaccomatto"
    elif board.is_stalemate():
        game_over = True
        result = "patta"
    elif board.is_insufficient_material():
        game_over = True
        result = "patta"
    elif board.can_claim_threefold_repetition():
        game_over = True
        result = "patta"
    elif board.can_claim_fifty_moves():
        game_over = True
        result = "patta"

    return jsonify({"game_over": game_over, "result": result})


def open_browser():
    time.sleep(1)
    webbrowser.open("http://localhost:5000")


if __name__ == "__main__":
    if os.environ.get("WERKZEUG_RUN_MAIN") == "true":
        threading.Thread(target=open_browser).start()
    # ------------------------------------------
    # Lancia Flask in ascolto su tutte le interfacce
    # ------------------------------------------
    app.run(host="0.0.0.0", port=5000, debug=True, use_reloader=False)