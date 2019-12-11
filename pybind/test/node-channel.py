from tgimbox import *

def Channel(t):
  ret = Box("BasicChannel","Box")
  ret.CreateChannel("c",t)
  ret.CreatePort("c", "p0")
  ret.CreatePort("c", "p1")
  return ret

Node = Box("BasicNode","Box")
Node.CreateNode("n","Node")
Node.CreateChannel("c","ANY")
Node.TriConnect([], "n", "c", "p")

ch = Channel("Csma").Fork("ch")
n0 = Node.Fork("n0")
n1 = Node.Fork("n1")

# ok
n0.ConnectPort("p", ch, "p0")
n1.ConnectPort("p", ch, "p1")
# bad
#ch.ConnectPort("p0", n0, "p")
#ch.ConnectPort("p1", n1, "p")

Builder.AddBox([ch, n0, n1])
print(Builder.Build())
