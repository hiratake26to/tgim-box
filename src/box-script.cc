#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>
#include <map>
#include <typeinfo>
#include <any>
#include <variant>
#include <boost/core/demangle.hpp>
#include "ExceptionDecolater.hpp"
#include "schedule/schedule.hpp"
#include "box/box.hpp"

using std::cout;
using std::endl;
using std::optional;
using std::string;
using std::vector; // should not use reference to a element of vector.
using std::list; // allow to use reference of a element.
using std::pair;
using std::map;
using std::type_info;
using std::any;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

void signal_test() {
  Box Base("sender_base", "Base");
  Base.CreateNode("n0", "Node");
  Base.CreateChannel("c0", "PointToPoint");
  Base.TriConnect("n0", "c0", "p0");

  Box sender = Base.Fork("sender", "Sender");
  Box sinker = Base.Fork("sinker", "Sinker");
  sender.ConnectPort("p0", sinker, "p0");

  cout << "[TEST(1)]" << endl;
  sender
    .At(Time{1}).Do(Sig{"Sig1"})
    .At(Time{1}).Do(Sig{"Sig2"}) // simultaneous do signal
    .At(Time{2}).Do(Sig{"Sig1"})
    .At(Sig{"Sig1"}).Do(Sig{"Sig3"})
    .At(Sig{"Sig2"}).Do("Pre1", {})
    .At(Sig{"Sig2"}).Do(Sig{"Sig4"})
    .At(Sig{"Sig3"}).Do("Pre2", {})
    .At(Sig{"Sig3"}).Do(Sig{"Sig2"})
    ;
  for (const auto& i : sender.FlattenSchedule()) {
    cout << "[test] " << i.ToString() << endl;
  }
  sender.ClearSchedule();

  cout << "[TEST(2)]" << endl;
  sender
    .At(Time{1}).Do(Sig{"Sig1"})
    .At(Sig{"Sig1"}).Do("Pre1",{})
    .At(Sig{"Sig1"}).Do("Pre2",{})
    ;
  for (const auto& i : sender.FlattenSchedule()) {
    cout << "[test] " << i.ToString() << endl;
  }
  sender.ClearSchedule();

  cout << "[TEST(3)]" << endl;
  sender
    .At(0).Do("Prepare",{})
    .At(5).Aft()
      .Do(Sig{"Ready"})

    .At(Sig{"Ready"}).Aft()
      .Do("Send",{})
      .Do("Ping",{})
      .Do("FlowPattern",{})
      .Do("Sink",{})
      .Do("NicCtl",{})
      .Do("Move",{})
    ;
  for (const auto& i : sender.FlattenSchedule()) {
    cout << "[test] " << i.ToString() << endl;
  }
  sender.ClearSchedule();

  cout << "[TEST(4)]" << endl;
  sender
    //.At(Time{0}).Sdl([](auto&&a){a
    //.At(Sig{"world"}).Do("Hoge",{})
    .At(10).Sdl([](auto&&a){a
      .At(1).Do(Sig{"hello"}) // 11
      .At(2).Do(Sig{"world"}) // 12
      ;
    })
    .At(Sig{"hello"}).Do("Honma",{}) // 11*
    .At(Sig{"world"}).Do("Himawari",{}) // 12*
      .At(Sig{"world"}).Aft() // 12
      .At(1).Do("🌻",{}) // 13*
      .At(0).Do("🌻",{}) // 12*
        .At(2).Aft().Do("📕",{}) // 14*
          .At(1).Do("(・ヮ・🌻)",{}) // 15*
          .At(3).Aft().Do("📕",{}) // 17*
    ;
  //cout << sender.DumpSchedule() << endl;
  for (const auto& i : sender.FlattenSchedule()) {
    cout << "[test] " << i.ToString() << endl;
  }
  sender.ClearSchedule();

  cout << "[TEST(5)]" << endl;
  sender
    .At(0).Do("0",{})
    .At(1).Do("1",{})
    .At(5).Aft()
      .At(0).Do("5",{})
      .At(2).Do("7",{})
      .At(7).Aft() // 5+7=12
        .At(1).Aft() // 12+1=13
          .Do("13",{})
          .Do("13",{})
    ;

  //cout << sender.DumpSchedule() << endl;
  for (const auto& i : sender.FlattenSchedule()) {
    cout << "[test] " << i.ToString() << endl;
  }
  sender.ClearSchedule();

  cout << "[TEST(6)]" << endl;
  sender
    .At(2).Do(Sig{"SimReady"})

    .At(Sig{"SimReady"}).Do("recv_sim_ready",{})

    .At(Sig{"SimReady"}).Aft()
      .At(4).Do(Sig{"AppStart"})
      .At(6).Do("end",{})
    .EndAft()

    .At(Sig{"AppStart"}).Do("recv_app_start",{})
    ;

  cout << sender.DumpSchedule() << endl;
  for (const auto& i : sender.FlattenSchedule()) {
    cout << "[test] " << i.ToString() << endl;
  }
  sender.ClearSchedule();

  /// TEST
  sender
    .At(Sig{"Sig1"})
    .Do("Pre1", {})
    .At(Sig{"Sig2"})
    .Do("Pre1", {})
    ;
  sender
    .At(Time{1}).Do(Sig{"Sig1"})
    .At(Time{2}).Do(Sig{"Sig2"})
    ;
  NsomBuilder builder("TestNet");
  builder.AddBox(sender);
  builder.AddBox(sinker);
  //cout << builder.Build() << endl;
}

void nic_switch_test() {
  // create base box
  Box RouteSwitch("route_switch_base", "RouteSwitch");
  Box Router("router_base", "Router");
  Box Sinker("sinker_base", "Sinker");

  // TODO implement ANY-channel
  // create node and channel over the box.
  RouteSwitch.CreateNode("n0", "Node");
  RouteSwitch.CreateChannel("c0", "ANY");
  RouteSwitch.CreateChannel("c1", "ANY");
  RouteSwitch.TriConnect("n0", "c0", "p0");
  RouteSwitch.TriConnect("n0", "c1", "p1");

  Router.CreateNode("n0", "Node");
  Router.CreateChannel("c0", "Csma");
  Router.CreateChannel("c1", "Csma");
  Router.TriConnect("n0", "c0", "p0");
  Router.TriConnect("n0", "c1", "p1");

  Sinker.CreateNode("n0", "Node");
  Sinker.CreateChannel("c0", "ANY");
  Sinker.CreateChannel("c1", "ANY");
  Sinker.TriConnect("n0", "c0", "p0");
  Sinker.TriConnect("n0", "c1", "p1");
  Sinker
    .At(Sig{"Start"})
    .Do("Sink", {{"port",8080},{"time",30}})
    ;

  // preparate application(schedule)
  RouteSwitch
    .At(Sig{"SwitchPort0"})
    .Sdl([](auto&&a){a
      .At(Time{0}).Do("NicCtl", {{"idx", "1"},{"enable", "0"}})
      .At(Time{0}).Do("NicCtl", {{"idx", "0"},{"enable", "1"}})
      //.At(Time{0}).Do("PRE_0", {})
      ;
    })
    //.At(Sig{"SwitchPort1"}).Do("PRE_1", {})
    .At(Sig{"SwitchPort1"})
    .Do("NicCtl", {{"idx", "0"},{"enable", "0"}})
    .At(Sig{"SwitchPort1"})
    .Do("NicCtl", {{"idx", "1"},{"enable", "1"}})
    ;

  // topology
  //
  //  RS              Router      Sinker
  // ┌─────┐         ┌─────┐     ┌─────┐
  // │   p0│─────────│p0 p1│─────│p0   │
  // │   p1│────┐    └─────┘    ┌│p1   │
  // └─────┘    │    ┌─────┐    │└─────┘
  //            └────│p0 p1│────┘
  //                 └─────┘
  //
  
  Box rs = RouteSwitch.Fork("rs");
  Box r0 = Router.Fork("r0");
  Box r1 = Router.Fork("r1");
  Box s0 = Sinker.Fork("s0");

  rs.ConnectPort("p0", r0, "p0");
  rs.ConnectPort("p1", r1, "p0");

  s0.ConnectPort("p0", r0, "p1");
  s0.ConnectPort("p1", r1, "p1");

  // schedule
  // global signal: SimStart
  Box global("_global", "GlobalSig"); // exclusive box for signal
  global
    .At(Time{5}).Do(Sig{"SimStart"})
    ;
  // sinker
  s0
    .Sdl(global.GetSchedule())
    .At(Sig{"SimStart"}).Do(Sig{"Start"})
    ;
  // route switch
  rs
    .Sdl(global.GetSchedule())
    .At(Sig{"SwitchPort0"}).Do("ADD_PRE_0", {})
    .At(Sig{"SimStart"})
    .Aft()
      .At(Sig{"SwitchPort1"}).Do("OVERRIDE_PRE_1", {})
      .At(Range<Time>{10,111,100})
      .Aft()
        .At(Time{0}).Do(Sig{"SwitchPort0"})
        .At(Time{5}).Do(Sig{"SwitchPort1"})
      .EndAft()
      .At(Sig{"Timeout"})
      .Do(Sig{"Stop"})
    .EndAft();
    ;

  // DEBUG
  //cout << "[DEBUG Schedule]" << endl;
  //cout << rs.DumpSchedule() << endl;

  // build
  NsomBuilder builder("TestNet");
  builder.AddBox(rs);
  builder.AddBox(r0);
  builder.AddBox(r1);
  builder.AddBox(s0);
  cout << builder.Build() << endl;
}

void test() {
  // logic_error: library responsibility
  // runtime_error: user responsibility
  try{
    //signal_test();
    nic_switch_test();
  }catch(const std::exception& e) {
    std::cerr
      //<< e.what()
      << edc::HonhimaDecolate(e).what()
      //<< edc::HondaDecolate(e).what()
      << std::flush;
  }
  cout << "バグなし！生きてるだけで勝ち🌻" << endl;
}

int main() {
  test();
  return 0;
}
