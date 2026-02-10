import os
import subprocess
import signal
import sys
from pynput import keyboard

# --- CONFIGURATION ---
# Simple, fast compile command
COMPILE_CMD = (
    "g++ -std=c++11 "
    "desktop/main.cpp "
    "desktop/sdl_renderer.cpp "
    "desktop/desktop_stubs.cpp "
    "main/pages/home/home_page.cpp "
    "main/pages/files/files_page.cpp " 
    "-I./desktop -I./main "
    "-lSDL2 -o soil_sim"
)

RUN_CMD = ["./soil_sim"]
# ---------------------

process = None

def kill_sim():
    global process
    if process:
        try:
            os.kill(process.pid, signal.SIGTERM)
            process.wait()
        except:
            pass
        process = None

def start_sim():
    global process
    print("🚀 Compiling...")
    if os.system(COMPILE_CMD) == 0:
        print("✅ Success!")
        process = subprocess.Popen(RUN_CMD)
    else:
        print("❌ Compilation Failed.")

def on_reload_trigger():
    print("\n♻️  Reloading...")
    kill_sim()
    start_sim()

def main():
    start_sim()
    try:
        with keyboard.GlobalHotKeys({'<alt>+<shift>+t': on_reload_trigger}) as h:
            h.join()
    except KeyboardInterrupt:
        kill_sim()

if __name__ == "__main__":
    main()