import os
import sys

from PyQt6.QtWidgets import QApplication


def pytest_configure():
    global app
    if sys.platform == "win32":
        os.add_dll_directory(str(os.getenv("UIBASE_PATH")))
    else:
        env_string = os.getenv("LD_LIBRARY_PATH", "")
        if len(env_string) != 0:
            env_string = env_string + ":"
        os.putenv("LD_LIBRARY_PATH", env_string + str(os.getenv("UIBASE_PATH")))

    app = QApplication(sys.argv)
