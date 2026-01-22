import tkinter as tk
from tkinter import ttk


class Square(tk.Canvas):
    def __init__(self, master, row, col, view):
        super().__init__(
            master,
            width=50,
            height=50,
            bg="white",
            highlightthickness=1,
            highlightbackground="gray",
        )

        self.row = row
        self.col = col
        self.value = 0
        self.view = view

        self.bind("<Button-1>", self._handle_click)

    def _handle_click(self, event):
        self.value += 1
        self.value = self.value % 2

        self.view.update_dot(self.value, self.row, self.col)


class View:
    def __init__(self, master):
        self.master = master
        master.title("Dot Ori Solver")

        self.square_frame = tk.Frame(master)
        self.square_frame.pack(padx=10, pady=10)
        self.squares = []
        for i in range(8):
            for j in range(8):
                square = Square(self.square_frame, i, j, self)
                square.grid(row=i, column=j)
                self.squares.append(square)

        # 速度調整バー
        self.speed_scale = tk.Scale(
            master, from_=1, to=500, orient=tk.HORIZONTAL, label="Speed"
        )
        self.speed_scale.set(500)
        self.speed_scale.pack(pady=5, padx=50)

        # 境界線の本数
        self.border_text = tk.StringVar(value="border 0")
        self.label = ttk.Label(
            master, textvariable=self.border_text, font=("Arial", 16)
        )
        self.label.pack(pady=5, padx=50)

        # 回路の長さ
        self.circuit_text = tk.StringVar(value="circuit 0")
        self.label = ttk.Label(
            master,
            textvariable=self.circuit_text,
            font=("Arial", 16),
        )
        self.label.pack(pady=5, padx=50)

        # 実行ボタン
        self.exe_button = ttk.Button(master, text="execute")
        self.exe_button.pack(pady=10)

        # クリアボタン
        self.clear_button = ttk.Button(master, text="clear")
        self.clear_button.pack(pady=10)

    def set_controller(self, controller):

        # 実行ボタンの動作の設定
        self.exe_button["command"] = controller.execute

        # クリアボタンの動作の設定
        self.clear_button["command"] = controller.clear

        # マス目の動作の設定
        for i in range(64):
            x = i % 8
            y = i // 8
            id = y * 8 + x
            self.squares[id].bind(
                "<Button-1>",
                lambda event, row=y, col=x: controller.square_click_action(row, col),
            )

    def update_border(self, new_value):
        self.border_text.set("border " + str(new_value))

    def update_circuit(self, new_value):
        self.circuit_text.set("circuit " + str(new_value))

    def update_dot(self, value, row, col):
        colors = ["white", "black"]
        self.squares[row * 8 + col].config(bg=colors[value])

    def clear(self):
        for s in self.squares:
            s.config(bg="white")
