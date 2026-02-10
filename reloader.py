import os
import subprocess
import signal
import sys
from pynput import keyboard

# --- CONFIGURATION ---
# Updated compile command to include your new page files
COMPILE_CMD = (
    "g++ -std=c++11 "
    "desktop/main.cpp "
    "desktop/sdl_renderer.cpp "
    "desktop/desktop_stubs.cpp "
    # Add your page files here. Comment these out if you haven't created them yet!
    "main/pages/home/home_page.cpp "
    "main/pages/files/files_page.cpp " 
    "-I./desktop -I./main "
    "-lSDL2 -o soil_sim"
)

RUN_CMD = ["./soil_sim"]
# ---------------------

process = None

def kill_sim():
    """Stops the currently running simulator."""
    global process
    if process:
        try:
            # Send SIGTERM to the process
            os.kill(process.pid, signal.SIGTERM)
            process.wait()
        except ProcessLookupError:
            pass
        process = None

def start_sim():
    """Compiles and starts the simulator."""
    global process
    print("🚀 Compiling...")
    
    # Run the compile command
    result = os.system(COMPILE_CMD)
    
    if result == 0:
        print("✅ Success! Launching Simulator...")
        # Start the simulator non-blocking
        process = subprocess.Popen(RUN_CMD)
    else:
        print("❌ Compilation Failed. Fix errors and press hotkey again.")

def on_reload_trigger():
    """Callback for the hotkey."""
    print("\n⚡ Hotkey detected (Alt+Shift+T)!")
    print("♻️  Reloading...")
    kill_sim()
    start_sim()

def main():
    # 1. Start initially
    start_sim()

    print("\n" + "="*40)
    print("  MANUAL RELOADER RUNNING")
    print("  Press <Alt> + <Shift> + t to reload")
    print("  Press <Ctrl> + c to exit script")
    print("="*40 + "\n")

    # 2. Setup the global hotkey listener
    # This works even if the terminal is not focused
    try:
        with keyboard.GlobalHotKeys({
            '<alt>+<shift>+t': on_reload_trigger
        }) as h:
            h.join()
    except KeyboardInterrupt:
        print("\n👋 Exiting...")
        kill_sim()

if __name__ == "__main__":
    main()