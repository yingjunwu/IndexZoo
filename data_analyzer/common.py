def read_file(filename):
  f = open(filename)
  last_line = ""

  key_list = []
  pos_list = []
  count = 0
  while 1:
    line = f.readline().rstrip("\n")
    if not line:
      break
    key_list.append(int(line))
    pos_list.append(count)
    count = count + 1
  return key_list, pos_list