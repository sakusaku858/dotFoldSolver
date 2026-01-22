"""
・cp_finder
    ・外周部の32頂点の折り番号を入力として、平坦折り可能な展開図があれば表示する

    （例）
    python cp_finder.py 8 4 1 1 0 0 4 0 8 1 1 4 1 0 0 3 8 4 4 0 0 0 0 6 8 0 4 1 3 4 4 3
"""

import tkinter as tk
from tkinter import ttk
import subprocess
import shlex
from tkinter import messagebox
import sys
import os
import threading

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
        print("get_direction_list")
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

    def print(self):
        for edge in self.edges:
            edge.print()


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


class App:
    def __init__(self, size, nums):
        print("init App")
        self.size = size
        self.cp = self.nums_to_cp(nums)
        self.execute_process()

    # 折り番号を入力とし、対応する辺の値のリストを返す
    def num_to_edges(self, num, x1, y1):
        print("num_to_edges")
        if num == 8:
            return []

        # 着目している点の位置を取得
        TOP = 0
        RIGHT = 1
        BOTTOM = 2
        LEFT = 3
        position = None
        if y1 == 0:
            position = TOP
        if y1 == 8:
            position = BOTTOM
        if x1 == 0:
            position = LEFT
        if x1 == 8:
            position = RIGHT

        # 3本の辺の端点の相対座標
        relative_points = ((1, 1), (0, 1), (-1, 1))
        if position == RIGHT:
            relative_points = ((-1, 1), (-1, 0), (-1, -1))
        if position == BOTTOM:
            relative_points = ((-1, -1), (0, -1), (1, -1))
        if position == LEFT:
            relative_points = ((1, -1), (1, 0), (1, 1))

        # 3本の辺の座標を得る
        edges = []
        for i in range(3):
            x2 = x1 + relative_points[i][0]
            y2 = y1 + relative_points[i][1]
            edges.append([x1, y1, x2, y2, 0])

        # numから採用する辺番号を得る
        tmp = num
        for i in range(3):
            edges[i][4] = tmp % 2
            tmp = tmp // 2

        return edges

    # 頂点の折り番号の列から外周部の折りを決定したCPを得る
    def nums_to_cp(self, nums):
        print("num_to_cp")
        cp = CP(8, 8)

        # i番目の頂点のx,y座標を決定する
        points = []
        for i in range(32):
            if i < 8:
                points.append((i, 0))
            elif i < 16:
                points.append((8, i - 8))
            elif i < 24:
                points.append((24 - i, 8))
            elif i < 32:
                points.append((0, 32 - i))

        for i in range(32):
            x = points[i][0]
            y = points[i][1]
            edges = self.num_to_edges(nums[i], x, y)
            for edge in edges:
                x2 = edge[2]
                y2 = edge[3]
                use = edge[4]
                cp.set_edge_use(x, y, x2, y2, use)

        return cp

    def execute_process(self):
        print("execute_process")
        # すべての内部頂点の8方向の辺の状態を得る
        l = self.cp.get_direction_list()
        l = list(map(str, l))

        # 平坦折り可能なタイル配置を探すスクリプトを実行する
        script_dir = os.path.dirname(os.path.abspath(__file__))
        target_script = os.path.join(script_dir, "ftcp.exe")
        head = [target_script, str(self.size - 1)]
        result_str = subprocess.check_output(head + l, text=True, encoding="utf-8")

        # 解が見つからない場合はそのまま終了
        if result_str == "No CP":
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
        subprocess.run(arg)

    def execute_button_clicked_func(self, event):
        threading.Thread(target=self.execute_process, daemon=True).start()

    def to_tile_list(self, input_str):
        # 文字列を2文字ずつに区切る
        input_str = input_str.splitlines()[0]  # 最初の1行目だけを取得
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

    # コマンドライン引数をint型のリストに変換
    input_str = sys.argv[1:]
    int_list = []
    for arg in input_str:
        int_list.append(int(arg))

    app = App(8, int_list)
