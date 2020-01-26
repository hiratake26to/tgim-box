from tgimboxcore import Box, Point, Sig, NsomBuilder
import itertools
import json

"""
The standard box definition in this module.
"""

## Standard The Box
# Empty
BasicEmpty = Box("Empty", "Empty")

# Basic
BasicBox = Box("Basic", "Basic")
BasicBox.CreateNode("n0", "Main")

# Hub
BasicHub = Box("Hub", "Hub")
BasicHub.CreateChannel("c0", "Csma")
BasicHub.CreatePort("c0", "port0")
BasicHub.CreatePort("c0", "port1")
BasicHub.CreatePort("c0", "port2")
BasicHub.CreatePort("c0", "port3")

# Terminal
BasicTerminal = BasicBox.Fork("Terminal", "Terminal")
BasicTerminal.CreateChannel("c0", "Csma")
BasicTerminal.TriConnect([], "n0", "c0", "port")

# Router
#BasicRouter = BasicBox.Fork("Router", "Router")
#BasicRouter.CreateChannel("c0", "PointToPoint")
#BasicRouter.CreateChannel("c1", "Csma")
#BasicRouter.TriConnect([], "n0", "c0", "wan")
##BasicRouter.TriConnect(["Router"], "n0", "c1", "lan")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan0")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan1")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan2")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan3")
def GenRouter(lan_num):
  if not(type(lan_num) is int):
    raise TypeError("must be int, not "+lan_num.__class__.__name__)
  ret = BasicBox.Fork("Router", "Router")
  # create gateway
  ch_wan = "c_wan"
  ret.CreateChannel(ch_wan, "PointToPoint")
  ret.TriConnect(["Router"], "n0", ch_wan, "wan")
  # create lan port
  for i in range(lan_num):
    ch_name = "c_lan_"+str(i)
    ret.CreateChannel(ch_name, "Csma")
    ret.TriConnect(["Router"], "n0", ch_name, "lan"+str(i))
  return ret
BasicRouter = GenRouter(lan_num=4).Fork("Router", "Router")

# Switch
BasicSwitch = BasicBox.Fork("Switch", "Switch")
BasicSwitch.CreateChannel("c0", "Csma").SetConfig({"Address": "192.168.1.0/24"})
BasicSwitch.CreateChannel("c1", "Csma").SetConfig({"Address": "192.168.1.0/24"})
BasicSwitch.CreateChannel("c2", "Csma").SetConfig({"Address": "192.168.1.0/24"})
BasicSwitch.CreateChannel("c3", "Csma").SetConfig({"Address": "192.168.1.0/24"})
BasicSwitch.TriConnect(["Switch"], "n0", "c0", "port0")
BasicSwitch.TriConnect(["Switch"], "n0", "c1", "port1")
BasicSwitch.TriConnect(["Switch"], "n0", "c2", "port2")
BasicSwitch.TriConnect(["Switch"], "n0", "c3", "port3")
def GenSwitch(port_num):
  if not(type(port_num) is int):
    raise TypeError("must be int, not "+port_num.__class__.__name__)
  ret = BasicBox.Fork("Switch", "Switch")
  for i in range(port_num):
    ch_name = "c"+str(i)
    ret.CreateChannel(ch_name, "Csma").SetConfig({"Address": "192.168.1.0/24"})
    ret.TriConnect(["Switch"], "n0", ch_name, "port"+str(i))
  return ret


# RouteSwitch
BasicRouteSwitch = BasicBox.Fork("RouteSwitch", "RouteSwitch")
BasicRouteSwitch.CreateChannel("c0", "PointToPoint")
BasicRouteSwitch.CreateChannel("c1", "PointToPoint")
BasicRouteSwitch.CreateChannel("c2", "PointToPoint")
BasicRouteSwitch.TriConnect([], "n0", "c0", "wan")
BasicRouteSwitch.TriConnect([], "n0", "c1", "port0")
BasicRouteSwitch.TriConnect([], "n0", "c2", "port1")
(BasicRouteSwitch.Sdl()
  .At(Sig("SwitchPort0"))
  .Aft()
    #.At(0).Do("NicCtl", {"idx": 1, "enable": False})
    #.At(1).Do("NicCtl", {"idx": 0, "enable": True})
    .At(0).Do("ExpNicCtl", {"idx": 3, "enable": False})
    .At(0.1).Do("ExpNicCtl", {"idx": 2, "enable": True})
  .EndAft()
  .At(Sig("SwitchPort1"))
  .Aft()
    #.At(0).Do("NicCtl", {"idx": 0, "enable": False})
    #.At(1).Do("NicCtl", {"idx": 1, "enable": True})
    .At(0).Do("ExpNicCtl", {"idx": 2, "enable": False})
    .At(0.1).Do("ExpNicCtl", {"idx": 3, "enable": True})
  .EndAft()
  )

# Wifi Access Point
BasicWifiAccessPoint = BasicBox.Fork("WifiAccessPoint", "WifiAccessPoint")
BasicWifiAccessPoint.CreateChannel("c0", "PointToPoint")
BasicWifiAccessPoint.CreateChannel("c1", "WifiApSta")
BasicWifiAccessPoint.TriConnect([], "n0", "c0", "wan")
BasicWifiAccessPoint.TriConnect(["Ap"], "n0", "c1", "wlan0")
BasicWifiAccessPoint.CreatePort("c1", "wlan1")
BasicWifiAccessPoint.CreatePort("c1", "wlan2")
BasicWifiAccessPoint.CreatePort("c1", "wlan3")

# Wifi Station
BasicWifiStation = BasicBox.Fork("WifiStation", "WifiStation")
BasicWifiStation.CreateChannel("c0", "Wifi")
BasicWifiStation.TriConnect(["Sta"], "n0", "c0", "wport")

## GlobalSchedule
Global = Box("__global_schedule__", "BuiltInSchedule")
GlobalSchedule = Global.Sdl()
(GlobalSchedule
    .At(5).Do(Sig("SimReady"))
    )

## Standard Builder
class BuildBox:
  """
  BuildBox is the Box to convert and build into NSOM format file.
  """
  def __init__(self, boxs, net_name="network"):
    self.builder = NsomBuilder(net_name) # "network" is the network name
    self.builder.SetGlobalSdl(Global)
    if boxs.__class__.__name__ == "Box":
      self.builder.AddBox(boxs)
    elif hasattr(boxs, '__iter__'):
      self.builder.AddBox(self.__flatten(boxs))
    else:
      raise TypeError("must be Box or list of Box, not "+boxs.__class__.__name__)

  def Build(self):
    with open('tgim-main.json', 'w') as f:
      f.write(self.builder.Build())
    return self

  def Merge(self, node_channel_file):
    with open('tgim-main.json', 'r') as f:
      a = json.load(f)
    with open(node_channel_file, 'r') as f:
      b = json.load(f)
    c = self.__mergeAll(a,b)
    with open('tgim-main.json', 'w') as f:
      f.write(json.dumps(c,indent=2))

  def __getItems(self, a, key):
    if 'items' in dir(a[key]):
      return a[key].items()
    else:
      return []

  def __merge(self, key, a, b):
    a_items = self.__getItems(a, key)
    b_items = self.__getItems(b, key)
    print(a_items)
    print(b_items)
    return {key:value for (key,value) in (list(a_items)+list(b_items))}

  def __scale(self, ratio, obj):
    for (key,value) in obj.items():
      if not('point' in value): continue
      point = dict()
      if ('x' in value['point']):
        point['x'] = value['point']['x'] * ratio
      if ('y' in value['point']):
        point['y'] = value['point']['y'] * ratio
      if ('z' in value['point']):
        point['z'] = value['point']['z'] * ratio
      value['point'] = point

  def __mergeAll(self, a, b):
    self.__scale(0.1, b['node'])
    self.__scale(0.1, b['channel'])
    self.__scale(0.1, b['subnet'])
    ret = dict()
    ret['name']    = a['name']
    ret['node']    = self.__merge('node', a, b)
    ret['channel'] = self.__merge('channel', a, b)
    ret['subnet']  = self.__merge('subnet', a, b)
    ret['apps']    = self.__merge('apps', a, b)
    return ret

  def __flatten(self, box_list):
    if isinstance(box_list, str):
      raise TypeError("must be Box or list of Box, not "
          + box_list.__class__.__name__ + ": " + str(box_list))
    if ((not hasattr(box_list, '__iter__'))
        and (box_list.__class__.__name__ == "Box")):
      return [box_list]
    elif hasattr(box_list, '__iter__'):
      return [j for i in box_list for j in self.__flatten(i)]
    else:
      raise TypeError("must be Box or list of Box, not "
          + box_list.__class__.__name__ + ": " + str(box_list))
