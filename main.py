import tkinter as tk
from tkinter import ttk

import View
import Model
import Controller

if __name__ == "__main__":
    root = tk.Tk()

    model = Model.Model()
    view = View.View(root)

    controller = Controller.CounterController(model, view)

    root.mainloop()
