import os
import subprocess
import sys
from tkinter import messagebox


# 行列Aの全要素を足し合わせる
# Aは二次元配列
def sum_elements_of_matrix(A):
    sum = 0
    for i in range(8):
        for j in range(8):
            sum += A[i][j]
    return sum


# ドット絵の境界線の数を数える
# ドット絵は二次元配列
def count_border(dots):
    black = sum_elements_of_matrix(dots)

    inner = 0
    for i in range(8):
        for j in range(7):
            left = dots[i][j]
            right = dots[i][j + 1]
            if left == 1 and right == 1:
                inner += 1

    for i in range(7):
        for j in range(8):
            upper = dots[i][j]
            lower = dots[i + 1][j]
            if upper == 1 and lower == 1:
                inner += 1

    return 4 * black - inner * 2


class Model:
    def __init__(self):
        # ドット絵の２次元配列
        self.dots = [[0 for _ in range(8)] for _ in range(8)]

        self.circuitLength = 0

    # ドット絵を文字列形式にする
    def dots_to_str(self):
        dotstr = ""
        for i in range(64):
            x = i % 8
            y = i // 8
            dotstr += str(self.dots[y][x])
        print(dotstr)
        return dotstr

    def execute(self, speed):
        # ドット絵を文字列にしてc++プログラムに渡す
        dotstr = self.dots_to_str()

        # 平坦折り可能なタイル配置を探すスクリプトを実行する
        script_dir = os.path.dirname(os.path.abspath(__file__))
        target_script = os.path.join(script_dir, "dotToGraph.exe")
        head = [target_script, "-mode=findCP", dotstr, str(speed)]

        print("speed: " + str(speed))

        # スピードを設定する場合
        # head = [target_script, "-mode=findCP", dotstr, str(speed)]

        result_str = subprocess.check_output(head, text=True, encoding="utf-8")
        print("in model.py execute")
        print(result_str)

        cpstr = ""
        corners = []
        for line in result_str.splitlines():
            if line.startswith("CPSTR:"):
                cpstr = line.split(":")[1].strip()
            if line.startswith("CORNERS:"):
                corners = line.split(":")[1].strip().split()

        if cpstr != "":
            # TileDrawer.pyを実行して描画する
            # 引数: cpstr c1 c2 c3 c4
            cmd = [
                sys.executable,
                os.path.join(script_dir, "TileDrawer.py"),
                cpstr,
            ] + corners
            subprocess.Popen(cmd)

        if cpstr == "":
            messagebox.showwarning("Alert", "No CP was found.")

    def execute_non_connect(self):
        print("execute_non_connect")

        # ドット絵を文字列にしてc++プログラムに渡す
        dotstr = self.dots_to_str()

        # 平坦折り可能なタイル配置を探すスクリプトを実行する
        script_dir = os.path.dirname(os.path.abspath(__file__))
        target_script = os.path.join(script_dir, "solve_non_connect.exe")

        print("target_script: " + target_script)

        # スピードを設定する場合
        head = [target_script, dotstr]

        result_str = subprocess.check_output(head, text=True, encoding="utf-8")
        print(result_str)

        cpstr = ""
        corners = []
        for line in result_str.splitlines():
            if line.startswith("CPSTR:"):
                cpstr = line.split(":")[1].strip()
            if line.startswith("CORNERS:"):
                corners = line.split(":")[1].strip().split()

        if cpstr != "":
            # TileDrawer.pyを実行して描画する
            # 引数: cpstr c1 c2 c3 c4
            cmd = [
                sys.executable,
                os.path.join(script_dir, "TileDrawer.py"),
                cpstr,
            ] + corners
            subprocess.Popen(cmd)

        if cpstr == "":
            messagebox.showwarning("Alert", "No CP was found.")

    def switch_dot(self, row, col):
        value = (self.dots[row][col] + 1) % 2
        self.dots[row][col] = value
        return value

    def count_border(self):
        return count_border(self.dots)

    def clear(self):
        for x in range(8):
            for y in range(8):
                self.dots[x][y] = 0

    def calcLoopLength(self):
        # ドット絵を文字列にしてc++プログラムに渡す
        dotstr = self.dots_to_str()

        # ループ長を計算するスクリプトを実行する
        script_dir = os.path.dirname(os.path.abspath(__file__))
        target_script = os.path.join(script_dir, "dotToGraph.exe")
        head = [target_script, "-mode=calcLength", dotstr]

        result_str = subprocess.check_output(head, text=True, encoding="utf-8")
        print("in model.py calcLoopLength")
        print(result_str)

        for line in result_str.splitlines():
            if line.startswith("LOOP_LENGTH:"):
                self.circuitLength = int(line.split(":")[1].strip())
                break

        return self.circuitLength

    def is_connect(self):
        circuitLength = self.calcLoopLength()
        borderLength = self.count_border()
        return circuitLength == borderLength
