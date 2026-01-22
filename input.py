import tkinter as tk
from tkinter import ttk
import subprocess
import shlex
from tkinter import messagebox
import sys
import os
import threading
import time

UP = 0
UPPER_RIGHT = 1
RIGHT = 2
LOWER_RIGHT = 3
DOWN = 4
LOWER_LEFT = 5
LEFT = 6
UPPER_LEFT = 7

TILE = [
    [0, 0, 0, 0, 0, 0, 0, 0],
    [0, 0, 0, 1, 0, 0, 0, 1],
    [0, 0, 1, 0, 0, 0, 1, 0],
    [0, 0, 1, 0, 0, 1, 1, 1],
    [0, 0, 1, 0, 1, 1, 0, 1],
    [0, 0, 1, 1, 1, 0, 0, 1],
    [0, 1, 0, 0, 0, 1, 0, 0],
    [0, 1, 0, 0, 1, 0, 1, 1],
    [0, 1, 0, 0, 1, 1, 1, 0],
    [0, 1, 0, 1, 0, 1, 0, 1],
    [0, 1, 0, 1, 1, 0, 1, 0],
    [0, 1, 0, 1, 1, 1, 1, 1],
    [0, 1, 1, 0, 1, 0, 0, 1],
    [0, 1, 1, 1, 0, 0, 1, 0],
    [0, 1, 1, 1, 0, 1, 1, 1],
    [0, 1, 1, 1, 1, 1, 0, 1],
    [1, 0, 0, 0, 1, 0, 0, 0],
    [1, 0, 0, 1, 0, 0, 1, 1],
    [1, 0, 0, 1, 0, 1, 1, 0],
    [1, 0, 0, 1, 1, 1, 0, 0],
    [1, 0, 1, 0, 0, 1, 0, 1],
    [1, 0, 1, 0, 1, 0, 1, 0],
    [1, 0, 1, 0, 1, 1, 1, 1],
    [1, 0, 1, 1, 0, 1, 0, 0],
    [1, 0, 1, 1, 1, 0, 1, 1],
    [1, 0, 1, 1, 1, 1, 1, 0],
    [1, 1, 0, 0, 1, 0, 0, 1],
    [1, 1, 0, 1, 0, 0, 1, 0],
    [1, 1, 0, 1, 0, 1, 1, 1],
    [1, 1, 0, 1, 1, 1, 0, 1],
    [1, 1, 1, 0, 0, 1, 0, 0],
    [1, 1, 1, 0, 1, 0, 1, 1],
    [1, 1, 1, 0, 1, 1, 1, 0],
    [1, 1, 1, 1, 0, 1, 0, 1],
    [1, 1, 1, 1, 1, 0, 1, 0],
    [1, 1, 1, 1, 1, 1, 1, 1],
]


class Edge:
    def __init__(self, x1, y1, x2, y2, boundary, clickability):
        self.NOT_USE = 0
        self.USE = 1
        self.UNKNOWN = -1

        self.x1 = x1
        self.y1 = y1
        self.x2 = x2
        self.y2 = y2
        self.boundary = boundary
        self.clickability = clickability

        self.reset_use()

    def change_use(self):
        self.use = (self.use + 1) % 2

    def clicked_func(self):
        if self.clickability == True:
            self.change_use()

    def reset_use(self):
        if self.boundary == True:
            self.use = self.USE
        elif self.clickability == True:
            self.use = self.NOT_USE
        else:
            self.use = self.UNKNOWN

    def copy(self):
        copied = Edge(
            self.x1, self.y1, self.x2, self.y2, self.boundary, self.clickability
        )
        copied.use = self.use
        return copied

    def print(self):
        print(self.x1, self.y1, self.x2, self.y2, self.use)


class CP:
    def __init__(self, width, height):
        self.width = width
        self.height = height
        self.edges = []

        def is_boundary_point(x, y):
            return x == 0 or x == width or y == 0 or y == height

        def get_clickability(x1, y1, x2, y2):
            s = is_boundary_point(x1, y1)
            e = is_boundary_point(x2, y2)
            if s + e == 0:
                return False
            if is_boundary_edge(x1, y1, x2, y2):
                return False
            return True

        def is_boundary_edge(x1, y1, x2, y2):
            s = is_boundary_point(x1, y1)
            e = is_boundary_point(x2, y2)
            return s + e == 2 and (x1 == x2 or y1 == y2)

        def is_inside(x, y):
            return 0 <= x and x <= width and 0 <= y and y <= height

        # 縦、横、右下、左下
        dxs = [0, 1, 1, -1]
        dys = [1, 0, 1, 1]
        for x in range(width + 1):
            for y in range(height + 1):
                for d in range(4):
                    dx, dy = dxs[d], dys[d]
                    x2, y2 = x + dx, y + dy
                    if not is_inside(x2, y2):
                        continue
                    clickability = get_clickability(x, y, x2, y2)
                    boundary = is_boundary_edge(x, y, x2, y2)
                    self.edges.append(Edge(x, y, x2, y2, boundary, clickability))

    # すべての内部頂点の8方向の辺のリストを1次元配列で返す
    def get_direction_list(self):
        w = self.width - 1
        h = self.height - 1
        vsize = w * h
        dlist = [-1] * (vsize * 8)

        for edge in self.edges:
            if edge.clickability == False:
                continue
            x1 = edge.x1
            x2 = edge.x2
            y1 = edge.y1
            y2 = edge.y2
            v1 = self.get_internal_vertex_index(x1, y1)
            v2 = self.get_internal_vertex_index(x2, y2)
            d1 = self.get_direction(x1, y1, x2, y2)
            d2 = self.get_direction(x2, y2, x1, y1)
            if (not v1 == -1) and (not d1 == -1):
                dlist[v1 * 8 + d1] = edge.use
            if (not v2 == -1) and (not d2 == -1):
                dlist[v2 * 8 + d2] = edge.use
        return dlist

    def get_direction(self, x1, y1, x2, y2):
        dxs = [0, 1, 1, 1, 0, -1, -1, -1]
        dys = [-1, -1, 0, 1, 1, 1, 0, -1]
        dx = x2 - x1
        dy = y2 - y1
        for i in range(8):
            if dx == dxs[i] and dy == dys[i]:
                return i
        return -1

    def get_internal_vertex_index(self, x, y):
        def is_inside(x, y):
            return 0 < x and x < self.width and 0 < y and y < self.height

        if not is_inside(x, y):
            return -1

        x = x - 1
        y = y - 1
        w = self.width - 1
        return x + y * w

    def copy(self):
        copy_cp = CP(self.width, self.height)
        edges = []
        for e in self.edges:
            edges.append(e.copy())
        copy_cp.edges = edges
        return copy_cp

    def set_edge_use(self, x1, y1, x2, y2, use):
        edge = next(
            (
                item
                for item in self.edges
                if (item.x1 == x1 and item.y1 == y1 and item.x2 == x2 and item.y2 == y2)
                or (item.x1 == x2 and item.y1 == y2 and item.x2 == x1 and item.y2 == y1)
            ),
            None,
        )
        edge.use = use

    def to_list(self):
        output = [self.width, self.height]
        for edge in self.edges:
            output += [edge.x1, edge.y1, edge.x2, edge.y2, edge.use]
        return output

    def get_corner_edge(self):
        def is_boundary_point(x, y):
            return x == 0 or x == self.width or y == 0 or y == self.height

        corner_edges = []
        for edge in self.edges:
            if edge.clickability == False:
                continue
            x1 = edge.x1
            x2 = edge.x2
            y1 = edge.y1
            y2 = edge.y2
            if is_boundary_point(x1, y1) and is_boundary_point(x2, y2):
                corner_edges.append(edge)

        return corner_edges

    def get_edge(self, x1, y1, x2, y2):
        for e in self.edges:
            if e.x1 == x1 and e.x2 == x2 and e.y1 == y1 and e.y2 == y2:
                return e
        return None


class Connector:
    def __init__(self):
        pass

    def connect(self, width, height, edges, tiles):
        dxs = [0, 1, 1, 1, 0, -1, -1, -1]
        dys = [-1, -1, 0, 1, 1, 1, 0, -1]

        for i, tile in enumerate(tiles):
            x = i % (width - 1) + 1
            y = i // (width - 1) + 1

            for d in range(8):
                dx = dxs[d]
                dy = dys[d]
                x2 = x + dx
                y2 = y + dy
                edge = self.find_edge(x, y, x2, y2, edges)
                edge.use = TILE[tile][d]

        # edgesを1次元のリストに
        list = []
        for edge in edges:
            list += [edge.x1, edge.y1, edge.x2, edge.y2, edge.use]

        return [width, height] + list

    def find_edge(self, x1, y1, x2, y2, edges):
        edge = next(
            (
                item
                for item in edges
                if (item.x1 == x1 and item.y1 == y1 and item.x2 == x2 and item.y2 == y2)
                or (item.x1 == x2 and item.y1 == y2 and item.x2 == x1 and item.y2 == y1)
            ),
            None,
        )
        return edge


def read_full_file(filepath):
    try:
        # 'r' は読み込みモード (read) を示します
        # encoding='utf-8' は日本語などを扱うために重要です
        with open(filepath, "r", encoding="utf-8") as f:
            content = f.read()
            print("--- ファイル全体 ---")
            print(content)
            return content
    except FileNotFoundError:
        print(f"エラー: ファイル '{filepath}' が見つかりません。")
    except UnicodeDecodeError:
        print(f"エラー: エンコーディング (UTF-8) がファイルと一致しません。")


class App:
    def __init__(self, root, size):
        self.root = root
        self.size = size
        self.cp = CP(size, size)
        self.id_edge_list = []

        self.canvas = tk.Canvas(root, width=540, height=540, bg="white")
        self.canvas.bind("<Button-1>", self.canvas_clicked_func)

        self.reset_button = tk.Button(root, text="リセット")
        self.reset_button.bind("<Button-1>", self.reset_button_clicked_func)

        self.execute_button = tk.Button(root, text="実行")
        self.execute_button.bind("<Button-1>", self.execute_button_clicked_func)

        self.layout()
        self.create_lines(64, 10)
        self.set_edge_color()

    def layout(self):
        self.canvas.pack()
        self.reset_button.pack()
        self.execute_button.pack()

    def create_lines(self, scale, offset):
        edges = self.cp.edges
        for edge in edges:
            x1 = edge.x1 * scale + offset
            x2 = edge.x2 * scale + offset
            y1 = edge.y1 * scale + offset
            y2 = edge.y2 * scale + offset
            id = self.canvas.create_line(x1, y1, x2, y2)
            self.id_edge_list.append((id, edge))

    def set_edge_color(self):
        colors = ["light gray", "black", "white"]
        for item in self.id_edge_list:
            id = item[0]
            edge = item[1]
            self.canvas.itemconfig(id, fill=colors[edge.use], width=4)

    def canvas_clicked_func(self, event):
        closest_items = self.canvas.find_closest(event.x, event.y)
        item_id = closest_items[0]
        edge = next(filter(lambda x: x[0] == item_id, self.id_edge_list))[1]
        edge.clicked_func()
        self.set_edge_color()

    def reset_button_clicked_func(self, event):
        for item in self.id_edge_list:
            edge = item[1]
            edge.reset_use()
        self.set_edge_color()

    def execute_process(self):
        # すべての内部頂点の8方向の辺の状態を得る
        l = self.cp.get_direction_list()
        l = list(map(str, l))

        # 平坦折り可能なタイル配置を探すスクリプトを実行する
        script_dir = os.path.dirname(os.path.abspath(__file__))

        # target_script = os.path.join(script_dir, "put_tileO3.exe")
        target_script = os.path.join(script_dir, "put_tile.exe")

        head = [target_script, str(self.size - 1)]

        start_time = time.time()  # 処理開始時刻

        result_str = subprocess.check_output(head + l, text=True, encoding="utf-8")

        end_time = time.time()  # 処理終了時刻

        # 処理時間の計算
        elapsed_time = end_time - start_time
        print(f"処理時間 (time.time()): {elapsed_time:.4f}秒")

        print("to put_tile.exe : ", head + l)
        print(result_str)

        output_lines = result_str.splitlines()

        if output_lines:  # リストが空でない（少なくとも1行の出力があった）ことを確認
            first_line = output_lines[0]
            print(first_line)
        else:
            print("コマンドの出力が空でした。")

        result_str = first_line

        # result_file_path = os.path.join(script_dir, "result.txt")
        # result_str = read_full_file(result_file_path)

        # 解が見つからない場合はアラートを出す
        if result_str == "No CP":
            self.show_alert()
            return

        # 解が見つかった場合は描画する
        tile_list = self.to_tile_list(result_str)
        copied_edges = []
        for edge in self.cp.edges:
            copied_edges.append(edge.copy())

        # output_edgesの形式は
        # width heigth
        # x1 y1 x2 y2 use
        # x1 y1 x2 y2 use
        # ...

        c = Connector()
        output_edges = c.connect(self.cp.width, self.cp.height, copied_edges, tile_list)
        output_edges = list(map(str, output_edges))
        arg = [sys.executable, os.path.join(script_dir, "drawer2.py")] + output_edges
        print(arg)
        subprocess.run(arg)

    def execute_button_clicked_func(self, event):
        threading.Thread(target=self.execute_process, daemon=True).start()

    def to_tile_list(self, input_str):
        # 文字列を2文字ずつに区切る
        tile_list = []
        for i in range(0, len(input_str), 2):
            two_chars = input_str[i : i + 2]

            # 文字列を整数に変換する
            integer = int(two_chars)
            tile_list.append(integer)

        return tile_list

    def show_alert(self):
        messagebox.showwarning("結果", "平坦に折り畳める展開図が見つかりませんでした")


if __name__ == "__main__":
    # メインウィンドウ（ルート）の作成
    root = tk.Tk()

    # アプリケーションの実行
    app = App(root, 8)

    # イベントループの開始
    root.mainloop()
