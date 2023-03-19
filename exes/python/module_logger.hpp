#pragma once

#include <pybind11/embed.h>

class MyClass {
public:
  MyClass() {}

  pybind11::module& example() {
    return m_example;
  }

private:
  pybind11::module m_example = pybind11::module("example", "pybind11 example plugin");
};

PYBIND11_MODULE(mymodule, m) {
  pybind11::class_<MyClass>(m, "MyClass")
      .def(pybind11::init<>())
      .def_property_readonly("example", &MyClass::example);

  pybind11::module example_module = m.def_submodule("example");
  example_module.def("add", [](int i, int j) { return i + j; }, "A function which adds two numbers");
}
