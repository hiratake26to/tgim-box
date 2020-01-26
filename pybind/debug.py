from tgimbox import *

## create box
term0 = BasicTerminal.Fork("term0")
term1 = BasicTerminal.Fork("term1")
sw = GenSwitch(2).Fork("sw")

## connect
term0.ConnectPort("port", sw, "port0")
term1.ConnectPort("port", sw, "port1")

## schedule
(term0.Sdl()
    .At(Sig("SimReady"))
    .Do("Ping", {"dhost": term1.AsHost(), "dport": 8080})
    )

for i in [term0, term1]: (i.Sdl()
    .At(Sig("SimReady"))
    .Do("Ping", {"dhost": term1.AsHost(), "dport": 8080})
    )

fromSimReady = term0.Sdl().At(Sig("SimReady")).Aft()
for i in range(10,30,10):
    fromSimReady.At(i).Do("Ping")

(term1.Sdl()
    .At(Sig("SimReady"))
    .Do("Sink", {"port": 8080})
    )

#TODO get box signature
# - name
# - type

BasicTerminal

## build
#BuildBox([term0, term1, sw]).Build()
