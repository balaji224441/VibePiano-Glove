"""
VibePiano Glove - PC Piano Sound Engine
=======================================

Reads NOTE_ON / NOTE_OFF messages from the ESP32 over USB serial
and plays the corresponding WAV file using Windows sound playback.

Requirements:
    pip install pyserial

Expected project structure:

VibePiano-Glove/
├── Python_Code/
│   └── piano_sound_engine.py
└── audio/
    ├── C3.wav
    ├── D3.wav
    ├── E3.wav
    ├── F3.wav
    ├── G3.wav
    ├── C4.wav
    ├── D4.wav
    ├── E4.wav
    ├── F4.wav
    ├── G4.wav
    ├── C5.wav
    ├── D5.wav
    ├── E5.wav
    ├── F5.wav
    └── G5.wav

Change SERIAL_PORT if your ESP32 appears on a different COM port.
"""

import csv
import os
import time

import serial
import winsound


# ============================================================
# USER SETTINGS
# ============================================================

SERIAL_PORT = "COM12"
BAUD_RATE = 115200
SERIAL_TIMEOUT = 1

# Audio folder is relative to this Python file.
# This avoids the original machine-specific path:
# C:\Users\ASUS\Desktop\piano
SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
BASE_PATH = os.path.join(SCRIPT_DIR, "audio")


# ============================================================
# NOTE FILES
# ============================================================

NOTE_NAMES = [
    "C3", "D3", "E3", "F3", "G3",
    "C4", "D4", "E4", "F4", "G4",
    "C5", "D5", "E5", "F5", "G5"
]

note_files = {
    note: os.path.join(BASE_PATH, f"{note}.wav")
    for note in NOTE_NAMES
}


# ============================================================
# CHECK AUDIO FILES
# ============================================================

missing = [
    note
    for note, path in note_files.items()
    if not os.path.isfile(path)
]

if missing:
    print("ERROR: Missing WAV files:")
    for note in missing:
        print(f"  - {note}.wav")

    print()
    print("Expected audio folder:")
    print(BASE_PATH)

    raise SystemExit(1)


# ============================================================
# SOUND FUNCTIONS
# ============================================================

def stop_sound():
    """Stop the currently playing WAV file."""
    winsound.PlaySound(None, 0)


def play_note(note_name):
    """Play a WAV file asynchronously."""
    if note_name not in note_files:
        print(f"Unknown note: {note_name}")
        return

    winsound.PlaySound(
        note_files[note_name],
        winsound.SND_FILENAME | winsound.SND_ASYNC
    )


# ============================================================
# SERIAL CONNECTION
# ============================================================

try:
    ser = serial.Serial(
        SERIAL_PORT,
        BAUD_RATE,
        timeout=SERIAL_TIMEOUT
    )

    time.sleep(2)
    ser.reset_input_buffer()

    print("=" * 55)
    print("VIBEPIANO GLOVE - PC SOUND ENGINE")
    print("=" * 55)
    print(f"Connected to: {SERIAL_PORT}")
    print(f"Baud rate:    {BAUD_RATE}")
    print(f"Audio folder: {BASE_PATH}")
    print("=" * 55)

except serial.SerialException as e:
    print(f"Serial error: {e}")
    print()
    print("Check that:")
    print("1. The ESP32 is connected.")
    print("2. The correct COM port is selected.")
    print("3. Arduino Serial Monitor is closed.")
    print("4. No other program is using the COM port.")
    raise SystemExit(1)


# ============================================================
# SYSTEM STATE
# ============================================================

current_note = None
current_octave = 4


# ============================================================
# MAIN SERIAL PROCESSING LOOP
# ============================================================

try:

    while True:

        raw_line = ser.readline()

        if not raw_line:
            continue

        # Decode ESP32 serial data safely.
        line = raw_line.decode(
            "utf-8",
            errors="ignore"
        ).strip()

        if not line:
            continue

        print(line)

        # ----------------------------------------------------
        # NOTE_ON / NOTE_OFF
        # ----------------------------------------------------

        if line.startswith("NOTE_ON,") or line.startswith("NOTE_OFF,"):

            try:

                fields = next(
                    csv.reader(
                        [line],
                        skipinitialspace=True
                    )
                )

                # Expected format:
                # NOTE_ON,C4,FINGER=THUMB,OCTAVE=4,BEND=80
                #
                # or:
                # NOTE_OFF,C4,FINGER=THUMB,OCTAVE=4

                if len(fields) < 2:
                    print("Invalid note message:", line)
                    continue

                note_name = fields[1]

                finger = "UNKNOWN"
                message_octave = current_octave

                # Read optional key=value fields.
                for field in fields[2:]:

                    if "=" not in field:
                        continue

                    key, value = field.split("=", 1)

                    if key == "FINGER":
                        finger = value

                    elif key == "OCTAVE":
                        try:
                            message_octave = int(value)
                        except ValueError:
                            pass

                current_octave = message_octave

                # Check whether WAV file exists.
                if note_name not in note_files:
                    print("Unknown note:", note_name)
                    continue

                # ------------------------------------------------
                # NOTE ON
                # ------------------------------------------------

                if line.startswith("NOTE_ON,"):

                    # Only start a new sound when the note changes.
                    if current_note != note_name:

                        stop_sound()
                        play_note(note_name)

                        current_note = note_name

                    print(
                        f"PLAYING {note_name} | "
                        f"Finger={finger} | "
                        f"Octave={current_octave}"
                    )

                # ------------------------------------------------
                # NOTE OFF
                # ------------------------------------------------

                elif line.startswith("NOTE_OFF,"):

                    if current_note == note_name:

                        stop_sound()

                        print(
                            f"STOPPED {note_name} | "
                            f"Finger={finger} | "
                            f"Octave={current_octave}"
                        )

                        current_note = None

            except (ValueError, IndexError, csv.Error) as e:
                print("Parse error:", e)
                print("Received:", line)

        # ----------------------------------------------------
        # SYSTEM STATUS / OCTAVE
        # ----------------------------------------------------

        elif line.startswith("SYSTEM_STATUS"):

            try:

                if "OCTAVE=" in line:

                    octave_text = (
                        line.split("OCTAVE=", 1)[1]
                        .split(",", 1)[0]
                    )

                    current_octave = int(octave_text)

                    print(
                        "CURRENT OCTAVE =",
                        current_octave
                    )

            except (ValueError, IndexError):
                pass


# ============================================================
# EXIT / CLEANUP
# ============================================================

except KeyboardInterrupt:

    print()
    print("Stopped by user.")

finally:

    stop_sound()

    if ser.is_open:
        ser.close()

    print("Serial connection closed.")
