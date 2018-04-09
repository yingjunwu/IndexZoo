import matplotlib.pyplot as plt
import numpy as np
import re
import pickle
import os
import pylab
import matplotlib
import csv
import sys
import common
from matplotlib.font_manager import FontProperties
from matplotlib.ticker import LinearLocator


OPT_FONT_NAME = 'Helvetica'
TICK_FONT_SIZE = 10
LABEL_FONT_SIZE = 20
LEGEND_FONT_SIZE = 10
LABEL_FP = FontProperties(style='normal', size=LABEL_FONT_SIZE)
LEGEND_FP = FontProperties(style='normal', size=LEGEND_FONT_SIZE)
TICK_FP = FontProperties(style='normal', size=TICK_FONT_SIZE)

MARKERS = (['o', 's', 'v', "^", "h", "v", ">", "x", "d", "<", "|", "", "|", "_"])
COLOR_MAP = ( '#F58A87', '#80CA86', '#9EC9E9', '#FED113', '#D89761', '#F15854', '#5DA5DA', '#60BD68',  '#B276B2', '#DECF3F', '#F17CB0', '#B2912F', '#FAA43A', '#AFAFAF')
PATTERNS = ([ "//", "\\\\", "////", "o", "o", "\\\\" , "\\\\" , "//////", "//////", "." , "\\\\\\" , "\\\\\\" ])
LABEL_WEIGHT = 'bold'
LINE_COLORS = COLOR_MAP
LINE_WIDTH = 4.0
MARKER_SIZE = 0.0
MARKER_FREQUENCY = 100000

matplotlib.rcParams['ps.useafm'] = True
matplotlib.rcParams['pdf.use14corefonts'] = True
matplotlib.rcParams['xtick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['ytick.labelsize'] = TICK_FONT_SIZE
matplotlib.rcParams['font.family'] = OPT_FONT_NAME


def ConvertEpsToPdf(dir_filename):
  os.system("epstopdf --outfile " + dir_filename + ".pdf " + dir_filename + ".eps")
  os.system("rm -rf " + dir_filename + ".eps")


def draw_figure(key_list, pos_list, filename):

  key_bound = key_list[len(key_list) - 1]

  fig = plt.figure(figsize=(8,6))
  figure = fig.add_subplot(111)
  figure.plot(key_list, pos_list, color=LINE_COLORS[0], linewidth=LINE_WIDTH, marker=MARKERS[0], markersize = MARKER_SIZE, markevery=MARKER_FREQUENCY)

  plt.xticks([0, key_bound])
  plt.yticks([])

  plt.xlim(0, key_bound)
  plt.ylim(0, )
  
  figure.get_xaxis().set_tick_params(direction='in', pad=10)
  figure.get_yaxis().set_tick_params(direction='in', pad=10)

  plt.xlabel('Key', fontproperties=LABEL_FP)
  plt.ylabel('Position', fontproperties=LABEL_FP)

  plt.show()


if __name__ == "__main__":

  filename = "data.txt"
  if len(sys.argv) != 2:
    print("usage: " + sys.argv[0] + " filename")
    sys.exit()
  else:
    filename = sys.argv[1]

  print("filename: " + filename)
  
  pos_list, key_list = common.read_file(filename)

  draw_figure(pos_list, key_list, "data.pdf")