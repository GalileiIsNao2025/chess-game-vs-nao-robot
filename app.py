from flask import Flask, render_template, request, jsonify
import subprocess
import threading
import time
import os
import webbrowser
import chess

app = Flask(__name__)

# ==============================================================
# Percorso a Stockfish
# ==============================================================
STOCKFISH_PATH = r"C:\..."    # sostituire con percorso a stockfish

# ==============================================================
# Variabili globali
# ==============================================================
moves_uci = []                  # Lista mosse UCI
moves_formatted = []            # Lista mosse formattate
cached_stockfish_move = None
cached_stockfish_formatted = None
last_user_move = None           # Ultima mossa utente ricevuta

# ==============================================================
# ROUTE
# ==============================================================

@app.route("/")
def index():
    return render_template("index.html")


@app.route("/post_user_move", methods=["POST"])
def post_user_move():
    global last_user_move, moves_uci, moves_formatted, cached_stockfish_move, cached_stockfish_formatted
    uci = request.json.get("uci")
    formatted = request.json.get("formatted")

    if uci not in moves_uci:
        last_user_move = formatted
        moves_uci.append(uci)
        moves_formatted.append(formatted)
        cached_stockfish_move = None
        cached_stockfish_formatted = None
        print("[SERVER] Mossa utente (UCI):", uci)
        print("[SERVER] Mossa utente (formattata):", formatted)
        return jsonify({"status": "ok"})
    return jsonify({"status": "duplicate"})


@app.route("/get_user_move", methods=["GET"])
def get_user_move():
    global last_user_move
    move = last_user_move
    last_user_move = None
    return jsonify({"move": move})


@app.route("/getmove", methods=["GET"])
def get_best_move():
    global cached_stockfish_move, cached_stockfish_formatted, moves_uci, moves_formatted

    if cached_stockfish_move:
        return jsonify({"move": cached_stockfish_move, "formatted": cached_stockfish_formatted})

    board = chess.Board()
    for uci in moves_uci:
        move_obj = chess.Move.from_uci(uci)
        if move_obj in board.legal_moves:
            board.push(move_obj)
        else:
            print("[WARNING] Ignorata mossa illegale:", uci, "in", board.fen())

    # Avvio Stockfish
    sf = subprocess.Popen([STOCKFISH_PATH],
                          stdin=subprocess.PIPE,
                          stdout=subprocess.PIPE,
                          stderr=subprocess.PIPE,
                          universal_newlines=True)
    sf.stdin.write("uci\n")
    sf.stdin.write(f"position startpos moves {' '.join(moves_uci)}\n")
    sf.stdin.write("go movetime 2000\n")
    sf.stdin.flush()

    best_move = None
    while True:
        line = sf.stdout.readline().strip()
        if line.startswith("bestmove"):
            best_move = line.split()[1]
            break
    sf.kill()

    move_obj = chess.Move.from_uci(best_move)
    formatted = best_move[:2] + "x" + best_move[2:] if board.is_capture(move_obj) else best_move

    cached_stockfish_move = best_move
    cached_stockfish_formatted = formatted

    print("[SERVER] Mossa Stockfish (UCI):", best_move)
    print("[SERVER] Mossa Stockfish (formattata):", formatted)

    return jsonify({"move": best_move, "formatted": formatted})


@app.route("/confirm_move", methods=["POST"])
def confirm_move():
    global moves_uci, moves_formatted, cached_stockfish_move, cached_stockfish_formatted
    uci = request.json.get("uci")
    formatted = request.json.get("formatted")
    if uci not in moves_uci:
        moves_uci.append(uci)
        moves_formatted.append(formatted)
    cached_stockfish_move = None
    cached_stockfish_formatted = None
    return jsonify({"status": "ok"})


@app.route("/is_game_over", methods=["GET"])
def is_game_over():
    board = chess.Board()
    for uci in moves_uci:
        move_obj = chess.Move.from_uci(uci)
        if move_obj in board.legal_moves:
            board.push(move_obj)
        else:
            print("[ERRORE] Mossa illegale:", uci, "in", board.fen())

    game_over = False
    result = ""
    if board.is_checkmate():
        game_over = True
        result = "scaccomatto"
    elif board.is_stalemate() or board.is_insufficient_material() \
         or board.can_claim_threefold_repetition() or board.can_claim_fifty_moves():
        game_over = True
        result = "patta"

    return jsonify({"game_over": game_over, "result": result})


# ==============================================================
# Utility per aprire il browser
# ==============================================================
def open_browser():
    time.sleep(1)
    webbrowser.open("http://localhost:5000")


# ==============================================================
# MAIN
# ==============================================================
if __name__ == "__main__":
    if os.environ.get("WERKZEUG_RUN_MAIN") == "true":
        threading.Thread(target=open_browser).start()
    app.run(host="0.0.0.0", port=5000, debug=True, use_reloader=False)
