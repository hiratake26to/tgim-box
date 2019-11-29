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

