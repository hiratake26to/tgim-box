from tgimbox import *

## create box
t0 = BasicTerminal.Fork("t0")
t1 = BasicTerminal.Fork("t1")
rs = BasicRouteSwitch.Fork("rs")

## connect
t0.ConnectPort("port", rs, "port0")
t1.ConnectPort("port", rs, "port1")

## schedule
(t0.Sdl()
    .At(Sig("SimReady"))
    .Do("Ping", {"dhost": "${_Bt1_Nn0}", "dport": 8080})
    )

fromSimReady = t0.Sdl().At(Sig("SimReady")).Aft()
#for i in range(10,30,1):
#  fromSimReady.At(i).Do("Ping")
fromSimReady.Do("FlowPattern", {"Type": "CBR"})

(t1.Sdl()
    .At(Sig("SimReady"))
    .Do("Sink", {"port": 8080})
    )

(t1.)

## build
Builder.AddBox([t0, t1, rs])
result = Builder.Build()

