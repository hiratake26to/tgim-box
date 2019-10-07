#if not PORT2

void box_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Basic");
  basic_box.CreateNode("N0", "NodeType");
  basic_box.CreateChannel("C0", "ChannelType");

  // create b0, b1, b2 from basic box
  Box b0 = basic_box.Fork("b0");

  Box b1 = basic_box.Fork("b1");
  b1.CreateChannel("C1", "ChannelType");

  Box b2 = basic_box.Fork("b2");

  // connect:
  // b0[C0] - [C0]b1[C1] - [C0]b2
  b0.Connect("C0", b1, "C0");
  b1.Connect("C1", b2, "C0");

  cout << b0.ToString() << endl;
  cout << b1.ToString() << endl;
  cout << b2.ToString() << endl;
}

void box_channel_merge_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Basic");
  basic_box.CreateNode("R", "Router");
  basic_box.CreateChannel("C", "Csma");

  // create router box
  Box r0 = basic_box.Fork("Router0");
    r0.CreateChannel("C1", "Csma");
  Box r1 = basic_box.Fork("Router1");
  Box r2 = basic_box.Fork("Router2");

  // connect:
  // r0[C] -> [C]r1
  //     | -> [C]r2
  r0.Connect("C", r1, "C");
  r0.Connect("C1", r2, "C");

  cout << r0.ToString() << endl;
  cout << r1.ToString() << endl;
  cout << r2.ToString() << endl;

  //
  vector<Box> boxs;
  boxs.push_back(r0);
  boxs.push_back(r1);
  boxs.push_back(r2);

  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (not p.IsParent()) {
      cout << b.GetName();
      cout << " connect to ";
      cout << p.to_box;
      cout << " via Channel type of ";
      cout << p.GetMergeChannelType() << endl;
    }
  }

}

void box_channel_merge_test2() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Basic");
  basic_box.CreateNode("R", "Router");
  basic_box.CreateChannel("C", "Csma");

  // create router box
  Box r0 = basic_box.Fork("Router0");
    r0.channels[0].type = "PPP";
    r0.CreateChannel("C1", "PPP");
    r0.CreateChannel("C2", "PPP");
  Box r1 = basic_box.Fork("Router1");
    r1.channels[0].type = "Csma";
  Box r2 = basic_box.Fork("Router2");
    r2.channels[0].type = "Csma";
  Box r3 = basic_box.Fork("Router3");
    r3.channels[0].type = "Csma";

  // connect:
  // r3[C] -> r0[C] -> [C]r1
  //              | -> [C]r2
  r0.Connect("C", r1, "C");
  r0.Connect("C1", r2, "C");
  r3.Connect("C", r0, "C2");

  cout << r0.ToString() << endl;
  cout << r1.ToString() << endl;
  cout << r2.ToString() << endl;

  //
  vector<Box> boxs;
  boxs.push_back(r0);
  boxs.push_back(r1);
  boxs.push_back(r2);
  boxs.push_back(r3);

  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (not p.IsParent()) {
      cout << b.GetName();
      cout << " connect to ";
      cout << p.to_box;
      cout << " via Channel type of ";
      cout << p.GetMergeChannelType() << endl;
    }
  }

}

void box_channel_merge_test3() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create basic box
  Box basic_box("undefined", "Pc");
  basic_box.CreateNode("P", "Pc");
  basic_box.CreateChannel("P", "PointToPoint");

  // create pc box
  Box p0 = basic_box.Fork("rb0");
  Box p1 = basic_box.Fork("rb1");
  Box p2 = basic_box.Fork("rb2");
  // create hub box
  Box h0("hb0", "Hub");
  h0.CreateNode("H", "Hub");
  h0.CreateChannel("P0", "PointToPoint");
  h0.CreateChannel("P1", "PointToPoint");
  h0.CreateChannel("P2", "PointToPoint");

  // connect:
  // r0[P] -> [P0]h0
  // r1[P] -> [P1]h0
  // r2[P] -> [P2]h0
  p0.Connect("P", h0, "P0");
  p1.Connect("P", h0, "P1");
  p2.Connect("P", h0, "P2");

  cout << p0.ToString() << endl;
  cout << p1.ToString() << endl;
  cout << p2.ToString() << endl;

  //
  vector<Box> boxs;
  boxs.push_back(p0);
  boxs.push_back(p1);
  boxs.push_back(p2);
  boxs.push_back(h0);

  // check PPP contract
  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (p.IsParent()) {
      //
    }
  }


  for (const auto& b : boxs)
  for (const auto& p : b.ports) {
    if (not p.IsParent()) {
      cout << b.GetName();
      cout << " connect to ";
      cout << p.to_box;
      cout << " via Channel type of ";
      cout << p.GetMergeChannelType() << endl;
    }
  }

}

void box_channel_merge_test4() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  Box dev0("dev0", "Device");
  dev0.CreateNode("n", "Basic");
  dev0.CreateChannel("c", "PointToPoint");
  Box dev1 = dev0.Fork("dev1");

  Box rt("rt", "Router");
  rt.CreateNode("n", "Router");
  rt.CreateChannel("c0", "Csma");
  rt.CreateChannel("c1", "Csma");

  // dev0[c] -> [c0]rt
  dev0.Connect("c", rt, "c0");
  // dev1[c] -> [c1]rt
  dev1.Connect("c", rt, "c1");

  vector<Box> boxs;
  boxs.push_back(dev0);
  boxs.push_back(dev1);
  boxs.push_back(rt);
  for (const auto& b : boxs)
    for (const auto& p : b.ports) {
      if (not p.IsParent()) {
        cout << b.GetName();
        cout << " connect to ";
        cout << p.to_box;
        cout << " via Channel type of ";
        cout << p.GetMergeChannelType() << endl;
      }
    }

}

void box_channel_merge_test5() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  Box dev0("dev0", "Device");
  dev0.CreateNode("n", "Basic");
  dev0.CreateChannel("c", "PointToPoint");
  Box dev1 = dev0.Fork("dev1");

  // dev0[c] -> [c0]rt
  dev0.Connect("c", dev1, "c");

  vector<Box> boxs;
  boxs.push_back(dev0);
  boxs.push_back(dev1);
  for (const auto& b : boxs)
    for (const auto& p : b.ports) {
      if (not p.IsParent()) {
        cout << b.GetName();
        cout << " connect to ";
        cout << p.to_box;
        cout << " via Channel type of ";
        cout << p.GetMergeChannelType() << endl;
      }
    }

}

void internally_connect_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  Box b("box", "Box");
  b.CreateNode("n0", "Node");
  b.CreateNode("n1", "Node");
  b.CreateChannel("c", "Channel");
  // A node connection to a channel is once.
  b.ConnectNodeToChannel("n0", "c");
  //b.ConnectNodeToChannel("n0", "c"); // the error occurs by that call it continue from above calling
  // but channel can accept connection from other nodes.
  b.ConnectNodeToChannel("n1", "c");

  // [TODO] investigate grouping channel.
  // otherwise, tagged channel is usable.
  //b.Tag("c0", "C");
  //b.Tag("c1", "C");
  //b.Tag("c2", "C");
  //b.MakeGroup({"c0", "c1", "c2"}, "C");
}

void all_box_test() {
  box_test();
  cout << endl;
  box_channel_merge_test();
  cout << endl;
  box_channel_merge_test2();
  cout << endl;
  box_channel_merge_test3();
  cout << endl;
  box_channel_merge_test4();
  cout << endl;
  box_channel_merge_test5();
  cout << endl;
  internally_connect_test();
  cout << "All test OK" << endl;
}

void builder_test() {
  cout << "[TEST] " << __FUNCTION__ << endl;

  // create box
  Box b("box", "Box");
  b.CreateNode("n0", "Node");
  b.CreateNode("n1", "Node");
  b.CreateChannel("c", "Channel");

  // internally connect
  b.ConnectNodeToChannel("n0", "c");
  b.ConnectNodeToChannel("n1", "c");

  Box b1 = b.Fork("box1");

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

  cout << "[TEST] " << __FUNCTION__ << endl;

  // create box
  Box b("box", "Box");
  b.CreateNode("n0", "Node");
  b.CreateNode("n1", "Node");
  b.CreateChannel("c", "Channel");

  // internally connect
  b.ConnectNodeToChannel("n0", "c");
  b.ConnectNodeToChannel("n1", "c");

  Box b1 = b.Fork("box1");

  // set point of node
  b.FindNode("n0").value().get().point = {.x=10, .y=20};
  b.FindNode("n1").value().get().point = {.x=20, .y=30};
  b1.FindNode("n0").value().get().point = {.x=110, .y=120};
  b1.FindNode("n1").value().get().point = {.x=120, .y=130};

  b.Connect("c", b1, "c").value();
  //b.Connect("c", b1, "c").value();

  // builder instanciate
  NsomBuilder builder("test-net");
  builder.AddBox(b);
  builder.AddBox(b1);
  cout << builder.Build() << endl;
}

#endif
