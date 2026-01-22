import tkinter as tk
from tkinter import ttk
import subprocess
import shlex
from tkinter import messagebox
import sys


def filter_edges_by_use(edges, n):
    return [edge for edge in edges if edge[4] == n]


def draw_edges(edges, canvas, scale=64, offset=10):
    # 辺の描画
    colors = ["#EEEEEE", "black"]
    for e in edges:
        x1 = e[0] * scale + offset
        y1 = e[1] * scale + offset
        x2 = e[2] * scale + offset
        y2 = e[3] * scale + offset
        canvas.create_line(x1, y1, x2, y2, fill=colors[e[4]], width=4)


if __name__ == "__main__":
    # print(len(sys.argv))
    # 引数の整理
    width = int(sys.argv[1])
    height = int(sys.argv[2])

    edges = []
    i = 3
    while i < len(sys.argv):
        x1 = int(sys.argv[i])
        y1 = int(sys.argv[i + 1])
        x2 = int(sys.argv[i + 2])
        y2 = int(sys.argv[i + 3])
        use = int(sys.argv[i + 4])
        i = i + 5
        edges.append([x1, y1, x2, y2, use])

    # メインウィンドウ（ルート）の作成
    root = tk.Tk()

    canvas = tk.Canvas(root, width=540, height=540, bg="white")
    canvas.pack()

    useEdges = filter_edges_by_use(edges, 1)
    notUseEdges = filter_edges_by_use(edges, 0)

    draw_edges(notUseEdges, canvas, 64, 10)
    draw_edges(useEdges, canvas, 64, 10)

    # イベントループの開始
    root.mainloop()
