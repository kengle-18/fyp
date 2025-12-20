from pathlib import Path
import os
from datetime import datetime
import shutil
from threading import Lock
from watchdog.observers import Observer
from watchdog.events import FileSystemEventHandler
import time
import logging

logging.basicConfig(level=logging.INFO, format='%(asctime)s - %(message)s')

analyzer_dir = Path(os.getenv("ANALYZER_DIR", "/app/src/analyzer"))

lock = Lock()
class FileProcessingHandler(FileSystemEventHandler):
    def process_file(self):
        with lock:
            files = os.listdir(analyzer_dir)  # lists all files and folders
            files_only = [f for f in files if os.path.isfile(os.path.join(analyzer_dir, f))]

            for f in files_only:
                source_path = os.path.join(analyzer_dir, f)
                logging.info(f"[DEBUG] Source path: {source_path}")

                # Split filename to get timestamp part
                timestamp = f.split('_', 3)[0] + '_' + f.split('_', 3)[1] + '_' + f.split('_', 3)[2]  
                #  logging.info(f"{timestamp}")
                # Create folder for this timestamp if it doesn't exist
                timestamp_folder = os.path.join(analyzer_dir, timestamp)
                os.makedirs(timestamp_folder, exist_ok=True)

                destination_path = os.path.join(timestamp_folder, f)
                #  logging.info(f"[DEBUG] Destination path: {destination_path}")

                # # Move file into the timestamp folder
                shutil.move(source_path, destination_path)

            logging.info("Files organized by timestamp successfully!")
            

    def on_any_event(self, event):
        self.process_file()


observer = Observer()
observer.schedule(FileProcessingHandler(), path=str(analyzer_dir), recursive=False)
observer.start()
logging.info(f"Watching {analyzer_dir}...")

# Check at startup in case the file already exists
FileProcessingHandler().process_file()

try:
    while True:
        pass  # keep running
except KeyboardInterrupt:
    observer.stop()
observer.join()