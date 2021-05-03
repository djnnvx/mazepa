#!/usr/bin/env python3

from asciimatics.effects import Julia
from asciimatics.widgets import Frame, Layout, Divider, Button, Text, ListBox, Widget
from asciimatics.scene import Scene
from asciimatics.screen import Screen
from asciimatics.exceptions import ResizeScreenError, NextScene, StopApplication
import sys
import pandas as pd


class TabButtons(Layout):
    def __init__(self, frame, active_tab_idx):
        cols = [1, 1, 1, 1, 1]
        super().__init__(cols)
        self._frame = frame
        for i,_ in enumerate(cols):
            self.add_widget(Divider(), i)
        btns = [Button("Btn1", self._on_click_1),
                Button("Btn2", self._on_click_2),
                Button("Btn3", self._on_click_3),
                Button("Btn4", self._on_click_4),
                Button("Quit", self._on_click_Q)]
        for i, btn in enumerate(btns):
            self.add_widget(btn, i)
        btns[active_tab_idx].disabled = False

    def _on_click_1(self):
        raise NextScene("Tab1")

    def _on_click_2(self):
        raise NextScene("Tab2")

    def _on_click_3(self):
        raise NextScene("Tab3")

    def _on_click_4(self):
        raise NextScene("Tab4")

    def _on_click_Q(self):
        raise StopApplication("Quit")


class KeyLogScene(Frame):
    def __init__(self, screen, name, df):
        super().__init__(screen,
                         screen.height,
                         screen.width,
                         can_scroll=False,
                         title=name)
        layout1 = Layout([1], fill_frame=True)
        self.add_layout(layout1)
        # add your widgets here
        layout1.add_widget(ListBox(Widget.FILL_FRAME, name="something", options=[("IP:" + str(name), 1), ("Logs:" + str(df), 2)]))

        layout2 = TabButtons(self, 0)
        self.add_layout(layout2)
        self.fix()

class KeyLogFrame(Frame):
    def __init__(self, screen, x, y, i, name, df):
        super().__init__(screen,
                         screen.height // find_height_divider(i, 3),
                         screen.width // find_width_divider(i, 3),
                         can_scroll=True,
                         title=name,
                         x=x * screen.width // find_width_divider(i, 3),
                         y=(y - 1) * screen.height // find_height_divider(i, 3))
        layout = Layout([1], fill_frame=True)
        self.add_layout(layout)
        layout.add_widget(ListBox(Widget.FILL_FRAME, name="something", options=[("IP:" + str(name), 1), ("Logs:" + str(df), 2)]))
        layout2 = TabButtons(self, 0)
        self.add_layout(layout2)
        self.fix()

def find_height_divider(num, max_on_line):
    if (num // max_on_line < 1):
        return 1
    else:
        return num // max_on_line

def find_width_divider(num, max_on_line):
    if num < max_on_line:
        return num
    else:
        return max_on_line

def demo(screen, scene):
    try:
        df = pd.read_csv(".keylogs.csv")
    except FileNotFoundError:
        df = None
    dc = df.to_dict()

    dc_len = len(dc)
    scenes = []
    effects = []
    y = 0
    x = 0
    for index, key in enumerate(dc):
        x += 1
        if (index % 3 == 0):
            y += 1
            x = 0
        effects.append(KeyLogFrame(screen, x, y, dc_len, key, dc[key]))
    scenes.append(Scene(effects, -1))
    screen.play(scenes, stop_on_resize=True, start_scene=scene, allow_int=True)

last_scene = None
while True:
    try:
        Screen.wrapper(demo, catch_interrupt=False, arguments=[last_scene])
        sys.exit(0)
    except ResizeScreenError as e:
        last_scene = e.scene

