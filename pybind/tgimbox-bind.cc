#include <iostream>
#include <sstream>
#include <fstream>
#include <string>
#include <list>
#include <vector>
#include <functional>
#include <algorithm>
#include <utility>
#include <map>
#include <typeinfo>
#include <boost/core/demangle.hpp>
#include <tgimbox/tgimbox.hpp>

#include <any>
#include <optional> 
#include <variant>

using std::cout;
using std::endl;
using std::optional;
using std::variant;
using std::string;
using std::vector; // should not use reference to a element of vector.
using std::list; // allow to use reference of a element.
using std::pair;
using std::map;
using std::type_info;
using std::any;

#include <nlohmann/json.hpp>
using json = nlohmann::json;

// pybind11
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11_json/pybind11_json.hpp>
namespace py = pybind11;

using namespace tgim;

PYBIND11_MODULE(tgimboxcore, m) {
  ///
  // Documentaion
  m.doc() = "TGIM Box for python bindings";

  ///
  // Point Class
  py::class_<Point>(m, "Point")
    .def(py::init<int,int>())
    .def(py::init<int,int,int>())
    .def_readonly("x", &Point::x)
    .def_readonly("y", &Point::y)
    .def_readonly("z", &Point::z)
    ;

  ///
  // Sig
  
  py::class_<Sig>(m, "Sig")
    .def("__str__", [](Schedule& a) { return a.ToString(); })
    .def(py::init<string>())
    ;

  ///
  // Schedule
  py::class_<Schedule>(m, "Schedule")
    .def("__str__", [](Schedule& a) { return a.ToString(); })
    ;

  ///
  // ScheduleControllBlock Class
  py::class_<ScheduleControllBlock>(m, "SCB")
    .def("At", [](ScheduleControllBlock& a, Sig sig) {
        return a.At(sig);
        })
    .def("At", [](ScheduleControllBlock& a, int time) {
        return a.At(time);
        })
    .def("Aft", &ScheduleControllBlock::Aft)
    .def("EndAft", &ScheduleControllBlock::EndAft)
    .def("Do", [](ScheduleControllBlock& a, string act_type, py::object config) {
        return a.Do(act_type, config);
        })
    .def("Do", [](ScheduleControllBlock& a, string act_type) {
        return a.Do(act_type, {});
        })
    .def("Do", [](ScheduleControllBlock& a, Sig sig) {
        return a.Do(sig);
        })
    //.def("Do", &ScheduleControllBlock::)
    //.def("Do", &ScheduleControllBlock::)
    //.def("Sdl", &ScheduleControllBlock::)
    //.def("Sdl", &ScheduleControllBlock::)
    ;
  
  ///
  // Node
  py::class_<Node>(m, "Node")
    .def("__str__", [](Node& a) { return a.ToString(); })
    .def("ToString", &Node::ToString, py::arg("level") = 0)
    ;

  ///
  // Channel
  py::class_<Channel>(m, "Channel")
    .def("__str__", [](Channel& a) { return a.ToString(); })
    .def("ToString", &Channel::ToString, py::arg("level") = 0)
    ;

  ///
  // Box Class
  py::class_<Box>(m, "Box")
    .def(py::init<string,string>())
    .def("__str__", [](Box& a) { return a.ToString(); })
    .def("ToString", &Box::ToString, py::arg("level") = 0)
    .def("CreateNode",    &Box::CreateNode)
    .def("CreateChannel", &Box::CreateChannel)
    .def("ConnectPort", &Box::ConnectPort)
    .def("TriConnect", (void(Box::*)(vector<string>,string,string,string))&Box::TriConnect,
        py::arg("roles"), py::arg("node"), py::arg("channel"), py::arg("port"))
    .def("SetName",  &Box::SetName)
    .def("SetType",  &Box::SetType)
    .def("SetPoint", &Box::SetPoint)
    .def("GetName",  &Box::GetName)
    .def("GetType",  &Box::GetType)
    .def("GetPoint", &Box::GetPoint)
    .def("GetSchedule", &Box::GetSchedule)
    .def("Fork", (Box(Box::*)(string,string)const)&Box::Fork,
        py::arg("name"), py::arg("type"))
    .def("Fork", (Box(Box::*)(string)const)&Box::Fork,
        py::arg("name"))
    .def("Sdl", &Box::Sdl2)
    ;

  ///
  // NsomBuilder
  py::class_<NsomBuilder>(m, "NsomBuilder")
    .def(py::init<string>())
    .def("AddBox", (void(NsomBuilder::*)(const vector<Box>&))&NsomBuilder::AddBox)
    .def("AddBox", (void(NsomBuilder::*)(const Box&))&NsomBuilder::AddBox)
    .def("SetGlobalSdl", &NsomBuilder::SetGlobalSdl)
    .def("Build", &NsomBuilder::Build)
    ;

}
