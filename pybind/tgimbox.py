from tgimboxcore import Box, Point, Sig, NsomBuilder

"""
The standard box definition in this module.
"""

## Standard The Box
# Basic
BasicBox = Box("basic", "Basic")
BasicBox.CreateNode("n0", "Main")

# Terminal
BasicTerminal = BasicBox.Fork("terminal", "Terminal")
BasicTerminal.CreateChannel("c0", "Csma")
BasicTerminal.TriConnect([], "n0", "c0", "port")

# Router
BasicRouter = BasicBox.Fork("router", "Router")
BasicRouter.CreateChannel("c0", "PointToPoint")
BasicRouter.CreateChannel("c1", "Csma")
BasicRouter.TriConnect([], "n0", "c0", "wan")
BasicRouter.TriConnect(["Router"], "n0", "c1", "lan")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan0")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan1")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan2")
#BasicTerminal.TriConnect(["Router"], "n0", "c1", "lan3")

# Switch
BasicSwitch = BasicBox.Fork("switch", "Switch")
BasicSwitch.CreateChannel("c0", "Csma")
BasicSwitch.CreateChannel("c1", "Csma")
BasicSwitch.CreateChannel("c2", "Csma")
BasicSwitch.CreateChannel("c3", "Csma")
BasicSwitch.TriConnect(["Switch"], "n0", "c0", "port0")
BasicSwitch.TriConnect(["Switch"], "n0", "c1", "port1")
BasicSwitch.TriConnect(["Switch"], "n0", "c2", "port2")
BasicSwitch.TriConnect(["Switch"], "n0", "c3", "port3")
def GenSwitch(port_num):
  if not(type(port_num) is int):
    raise TypeError("must be int, not "+port_num.__class__.__name__)
  BasicSwitch = BasicBox.Fork("switch", "Switch")
  for i in range(port_num):
    ch = "c"+str(i)
    BasicSwitch.CreateChannel(ch, "Csma")
    BasicSwitch.TriConnect(["Switch"], "n0", ch, "port"+str(i))
  return BasicSwitch


# RouteSwitch
BasicRouteSwitch = BasicBox.Fork("route_switch", "RouteSwitch")
BasicRouteSwitch.CreateChannel("c0", "ANY")
BasicRouteSwitch.CreateChannel("c1", "ANY")
BasicRouteSwitch.TriConnect([], "n0", "c0", "port0")
BasicRouteSwitch.TriConnect([], "n0", "c1", "port1")
(BasicRouteSwitch.Sdl()
  .At(Sig("SwitchPort0"))
  .Aft()
    .At(0).Do("NicCtl", {"idx": 1, "enable": False})
    .At(1).Do("NicCtl", {"idx": 0, "enable": True})
  .EndAft()
  .At(Sig("SwitchPort1"))
  .Aft()
    .At(0).Do("NicCtl", {"idx": 0, "enable": False})
    .At(1).Do("NicCtl", {"idx": 1, "enable": True})
  .EndAft()
  )

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
    self.builder.AddBox(boxs)

  def Build(self):
    with open('tgim-main.json', 'w') as f:
      f.write(self.builder.Build())

