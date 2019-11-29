void merge_channel_test() {
  // create box
  Box b0("b0", "Box");
  b0.CreateNode("n", "Node");
  b0.CreateChannel("c", "Channel");
  // internally connect
  b0.ConnectNodeToChannel("n", "c");

  // create port
  b0.CreatePort("c", "p");
  // TODO to be studied
  //b0.CreatePort("c", "c");
  //b0.PortForward("c");
  //b0.Portalize("c"); // otherwise
  //b0.CreatePort("c"); // create and portalize to channel "c"
  
  // fork
  Box b1 = b0.Fork("b1");

  b0.ConnectPort("p", b1, "p");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void multi_inner_node_test() {
  /* multi inner node connection
   *
   * b0
   * +----------------+
   * | n0 -> c0 -> [p0|--+
   * | n1 -> c1 -> [p1|--|--+
   * +----------------+  |  |
   *                     |  |
   * b1                  |  |
   * +----------------+  |  |
   * | n0 -> c0 -> [p0|--+  |
   * | n1 -> c1 -> [p1|-----+
   * +----------------+
   */
  // create box
  Box b0("b0", "Box");
  b0.CreateNode("n0", "Node");
  b0.CreateNode("n1", "Node");
  b0.CreateChannel("c0", "Channel");
  b0.CreateChannel("c1", "Channel");
  // internally connect
  b0.ConnectNodeToChannel("n0", "c0");
  b0.ConnectNodeToChannel("n1", "c1");

  // create port
  b0.CreatePort("c0", "p0");
  b0.CreatePort("c1", "p1");
  //b0.SetPortMaxConnection("p", 2);
  // TODO to be studied
  //b0.CreatePort("c", "c");
  //b0.PortForward("c");
  //b0.Portalize("c"); // otherwise
  //b0.CreatePort("c"); // create and portalize to channel "c"
  
  // fork
  Box b1 = b0.Fork("b1");

  b0.ConnectPort("p0", b1, "p0");
  b0.ConnectPort("p1", b1, "p1");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void box_self_port_connect_test() {
  /* Multiple inner node connection
   *
   * b0
   * +----------------+
   * | n0 -> c0 -> [p0>--+
   * | n1 -> c1 -> [p1>--|--+
   * +----------------+  |  |
   *                     |..|..merged channel decide by b0.c0
   * b1                  |  |....merged channel decide by b0.c1
   * +----------------+  |  |
   * | n0 -> c0 -> [p0<--+  |
   * | n1 -> c1 -> [p1<-----+
   * |  |           ︙|
   * |  +--> c2 -> [p2|  what is connection of p1 and p2
   * +----------------+
   *
   * NOTE
   * It would be necessary, first, that solve box internal connection?
   * That would be equivalent following?
   * ...
   * | n1 -> c1 -> [p1|       | n1 ->|c1|-> [p1|
   * |  |           ︙|       |      |  |    ︙|
   * |  +--> c2 -> [p2|   ,   |      |c2|-> [p2|
   * ...
   * even included in the same box, both c1 and c2 should be merged.
   */
  // create box
  Box b0("b0", "Box");
  b0.CreateNode("n0", "Node");
  b0.CreateNode("n1", "Node");
  b0.CreateChannel("c0", "Channel");
  b0.CreateChannel("c1", "Channel");
  // internally connect
  b0.ConnectNodeToChannel("n0", "c0");
  b0.ConnectNodeToChannel("n1", "c1");

  // create port
  b0.CreatePort("c0", "p0");
  b0.CreatePort("c1", "p1");
  //b0.SetPortMaxConnection("p", 2);
  
  // fork
  Box b1 = b0.Fork("b1");
  b1.CreateChannel("c2", "Channel"); // would be merged with c1
  b1.ConnectNodeToChannel("n1", "c2"); // n1 connect to c2
  b1.CreatePort("c2", "p2");
  b1.SetPortMaxConnection("p2", 2);
  b1.ConnectPort("p1", b1, "p2"); // internal connection of "p1-to-p2" in "b1", TODO to be check
  b0.ConnectPort("p0", b1, "p0");
  b0.ConnectPort("p1", b1, "p2");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

void box_multiconnection() {
  // create geteway
  Box b0("b0", "Box");
  b0.CreateNode("n0", "Node");
  b0.CreateChannel("c0", "Channel");
  b0.ConnectNodeToChannel("n0", "c0");
  b0.CreatePort("c0", "p0")
    .SetMaxConnection(10);
  
  vector<Box> boxs;
  for (int i = 0; i<10; ++i) {
    string name = string{} + "b" + std::to_string(i);
    Box b(name, "Box");
    b.CreateNode("n0", "Node");
    b.CreateChannel("c0", "Channel");
    b.ConnectNodeToChannel("n0", "c0");
    b.CreatePort("c0", "p0");
    // FIXME multi-connection
    b.ConnectPort("p0", b0, "p0");
    boxs.push_back(b);
  }

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b0);
  for (const auto& b : boxs) {
    builder.AddBox(b);
  }
  cout << builder.Build() << endl;
}

void bridge_test() {
  /* create switch-box */
  Box b0("bridge", "SwitchBox");
  b0.CreateNode("n0", "Node");
  // channel creation and configuration for bridge
  b0.CreateChannel("c0", "Csma")
    .SetConfig({ {"Address", { {"Base", "192.168.10.1/24"}, {"Type", "NetworkUnique"} }}});
  b0.CopyChannel("c0", "c1");
  b0.CopyChannel("c0", "c2");
  b0.TriConnect({"Switch"}, "n0", "c0", "p0");
  b0.TriConnect({"Switch"}, "n0", "c1", "p1");
  b0.TriConnect({"Switch"}, "n0", "c2", "p2");
  
  /* create terminal-box */
  Box b1("b1", "TerminalBox");
  b1.CreateNode("n0", "Node");
  b1.CreateChannel("c0", "Channel");
  b1.TriConnect("n0", "c0", "p0");
  Box b2 = b1.Fork("b2");
  Box b3 = b1.Fork("b3");

  // set points
  b0.SetPoint({10,10});
  b1.SetPoint({10,0});
  b2.SetPoint({0,20});
  b3.SetPoint({20,20});

  // connect box
  b1.ConnectPort("p0", b0, "p0");
  b2.ConnectPort("p0", b0, "p1");
  b3.ConnectPort("p0", b0, "p2");

  // NSOM build
  NsomBuilder builder("TestNet");
  builder.AddBox(b0);
  builder.AddBox(b1);
  builder.AddBox(b2);
  builder.AddBox(b3);
  cout << builder.Build() << endl;
}
void box_order_test() {

  init();
  
  {
    cout << "- Incomparable BoxType, Equality ParentChild" << endl;
    cout << "  (-,Parent), (-,Parent)" << endl;
    BoxPriority a{"", ParentChild::Parent};
    BoxPriority b{"", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- Equality BoxType, Equality ParentChild" << endl;
    cout << "  (Terminal,Parent), (Terminal,Parent)" << endl;
    BoxPriority a{"Terminal", ParentChild::Parent};
    BoxPriority b{"Terminal", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- Comparable BoxType, Equality ParentChild" << endl;
    cout << "  (Terminal,Parent), (Switch,Parent)" << endl;
    BoxPriority a{"Terminal", ParentChild::Parent};
    BoxPriority b{"Switch", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- Incomparable BoxType, Comparable ParentChild" << endl;
    cout << "  Parent, Child" << endl;
    BoxPriority a{"", ParentChild::Parent};
    BoxPriority b{"", ParentChild::Child};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- BoxType incomparable on one and not on the other, ParentChild is comparable" << endl;
    cout << "  (Terminal,Child), (-,Parent)" << endl;
    BoxPriority a{"Terminal", ParentChild::Child};
    BoxPriority b{"", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
  {
    cout << "- BoxType incomparable on one and not on the other, ParentChild is comparable" << endl;
    cout << "  (-,Child), (Terminal,Parent)" << endl;
    BoxPriority a{"", ParentChild::Child};
    BoxPriority b{"Terminal", ParentChild::Parent};
    if (a > b) { cout << ">"; } else if(a < b) { cout << "<"; }
    if (a == b) { cout << "=" << endl; } else { cout << "!=" << endl; }
  }
}
