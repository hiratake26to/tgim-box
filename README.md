# Box - tgim-box

tgim-box is a box's api. for C++ and Python.

## Design Goals

The API to use ns-3 network simulator by box. API core implementation by C++. For ease of use, it binding to Python.

## Make

## C++ API

Example: `test/box-script.cc`

```sh
$ make
```

## Python API

Example:
- `pybind/tgimbox.py`: Basic box definition
- `pybind/debug.py`: Basic box use


### build
```sh
$ cd pybind
$ make
```

### REPL
```sh
$ make run
```

## Example

### To use box

```python
from tgimbox import *

## create box
t0 = BasicTerminal.Fork("t0")
t1 = BasicTerminal.Fork("t1")
s0 = GenSwitch(2).Fork("s0")

## connect
t0.ConnectPort("port", s0, "port0")
t1.ConnectPort("port", s0, "port1")

## schedule
(t0.Sdl()
    .At(Sig("SimReady"))
    .Do("Ping", {"dhost": "${_Bt1_Nn0}", "dport": 8080})
    )

fromSimReady = t0.Sdl().At(Sig("SimReady")).Aft()
for i in range(10,30,10):
  fromSimReady.At(i).Do("Ping")

(t1.Sdl()
    .At(Sig("SimReady"))
    .Do("Sink", {"port": 8080})
    )

## build
Builder.AddBox([t0, t1, s0])
result = Builder.Build()
```

### Topology

```python
# topology
#
#  RS              Router      Sinker
# ┌─────┐         ┌─────┐     ┌─────┐
# │   p0│─────────│p0 p1│─────│p0   │
# │   p1│────┐    └─────┘    ┌│p1   │
# └─────┘    │    ┌─────┐    │└─────┘
#            └────│p0 p1│────┘
#                 └─────┘
#

# fork box
rs = RouteSwitch.Fork("rs");
r0 = Router.Fork("r0");
r1 = Router.Fork("r1");
s0 = Sinker.Fork("s0");

# set point
rs.SetPoint({0,0});
r0.SetPoint({20,0});
r1.SetPoint({20,10});
s0.SetPoint({40,0});

# connect from RS to Routers
rs.ConnectPort("p0", r0, "p0");
rs.ConnectPort("p1", r1, "p0");

# connect from Sinker to Routers
s0.ConnectPort("p0", r0, "p1");
s0.ConnectPort("p1", r1, "p1");
```

### Schedule

```python
# GlobalSchedule
Global = Box("__global_schedule__", "BuiltInSchedule")
GlobalSchedule = Global.Sdl()
(GlobalSchedule
    .At(5).Do(Sig("SimReady"))
    )
```

```python
# RouteSwitch schedule
BasicRouteSwitch = BasicBox.Fork("route_switch", "RouteSwitch")
BasicRouteSwitch.CreateChannel("c0", "ANY")
BasicRouteSwitch.CreateChannel("c1", "ANY")
BasicRouteSwitch.TriConnect([], "n0", "c0", "p0");
BasicRouteSwitch.TriConnect([], "n0", "c1", "p1");
(BasicRouteSwitch.Sdl()
  .At(Sig("SwitchPort0"))
  .Aft()
    .At(0).Do("NicCtl", {"idx": 1, "enable": False})
    .At(0).Do("NicCtl", {"idx": 0, "enable": True})
  .EndAft()
  .At(Sig("SwitchPort1"))
  .Aft()
    .At(0).Do("NicCtl", {"idx": 0, "enable": False})
    .At(0).Do("NicCtl", {"idx": 1, "enable": True})
  .EndAft()
  )
```

### Box definition

```python
# BasicBox
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
```

### Build NSOM

```python
Builder = NsomBuilder("network")   # "network" is the network name
Builder.AddBox([box0, box1, box0]) # adding boxs
result = Builder.Build()           # build
```

## Dependences

- BuildSystem: `cmake`
- Compiler: `gcc-8`, `g++-8`
- PythonBinding: `python3`, `pybind11`

## LICENSE

This software licensed under the MIT license:

Copyright 2019 hiratake26to

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

