/***************************************************************************
 *            utilities.hpp
 *
 *  Copyright  2005-26  Alberto Casagrande, Pieter Collins
 *
 ****************************************************************************/

#ifndef ARIADNE_PYTHON_UTILITIES_HPP
#define ARIADNE_PYTHON_UTILITIES_HPP

#include "pybind11.hpp"

#include <sstream>
#include <string>
#include <utility>

namespace Ariadne {

template<class T> struct PythonClassName;
template<class T> inline std::string python_class_name() { return PythonClassName<T>().get(); }
template<class T> inline std::string python_template_class_name(std::string str) { return python_class_name<T>()+str; }

template<template<class...>class> struct PythonTemplateName;
template<template<class...>class T> inline std::string python_template_name() { return PythonTemplateName<T>::get(); }

template<class A> bool __bool__(const A& a) { return static_cast<bool>(a); }

template<class A1, class A2>
auto __and__(const A1& a1, const A2& a2) -> decltype(a1 && a2) { return a1 && a2; }

template<class A1, class A2>
auto __or__(const A1& a1, const A2& a2) -> decltype(a1 || a2) { return a1 || a2; }

template<class A>
auto __not__(const A& a) -> decltype(!a) { return !a; }

template<class T> std::string __cstr__(const T& t) {
    std::stringstream ss;
    ss << t;
    return ss.str();
}

template<class T> struct PythonRepresentation {
    const T* pointer;
    explicit PythonRepresentation(const T& t) : pointer(&t) { }
    const T& reference() const { return *pointer; }
};

template<class T> PythonRepresentation<T> python_representation(const T& t) {
    return PythonRepresentation<T>(t);
}

template<class T> std::string __repr__(const T& t) {
    std::stringstream ss;
    ss << python_representation(t);
    return ss.str();
}

template<class A>
pybind11::class_<A>& define_logical(pybind11::module&, pybind11::class_<A>& pyclass) {
    pyclass.def("__and__", &__and__<A,A>);
    pyclass.def("__or__", &__or__<A,A>);
    pyclass.def("__invert__", &__not__<A>);
    return pyclass;
}

} // namespace Ariadne

#endif /* ARIADNE_PYTHON_UTILITIES_HPP */
