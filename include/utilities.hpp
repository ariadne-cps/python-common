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

template<class T> struct PythonLiteral {
    const T* pointer;
    explicit PythonLiteral(const T& t) : pointer(&t) { }
    const T& reference() const { return *pointer; }
};

template<class T> PythonLiteral<T> python_literal(const T& t) {
    return PythonLiteral<T>(t);
}

template<class A>
pybind11::class_<A>& define_logical(pybind11::module&, pybind11::class_<A>& pyclass) {
    pyclass.def("__and__", &__and__<A,A>);
    pyclass.def("__or__", &__or__<A,A>);
    pyclass.def("__invert__", &__not__<A>);
    return pyclass;
}

} // namespace Ariadne

#if defined(__GNUC__) || defined(__clang__)
namespace PyBind11 __attribute__((visibility("hidden"))) {
#else
namespace PyBind11 {
#endif

template<template<class...>class T> class Template { };

template<class V, class K>
void as_instantiate_template(pybind11::module const& module, pybind11::dict& instantiations) {
    instantiations[module.attr(Ariadne::python_class_name<K>().c_str())]
        = module.attr(Ariadne::python_class_name<V>().c_str());
}

template<template<class...>class T, class K>
void instantiate(pybind11::module const& module, pybind11::dict& instantiations) {
    instantiations[module.attr(Ariadne::python_class_name<K>().c_str())]
        = module.attr(Ariadne::python_class_name<T<K>>().c_str());
}

template<template<class...>class T, class K1, class K2>
void instantiate(pybind11::module const& module, pybind11::dict& instantiations) {
    pybind11::tuple tuple_name=pybind11::make_tuple(
        module.attr(Ariadne::python_class_name<K1>().c_str()),
        module.attr(Ariadne::python_class_name<K2>().c_str())
    );
    instantiations[tuple_name]=module.attr(Ariadne::python_class_name<T<K1,K2>>().c_str());
}

template<template<class...>class T, class... KS>
void instantiate_template(pybind11::module const& module, pybind11::dict& instantiations) {
    instantiate<T,KS...>(module,instantiations);
}

template<template<class...>class T>
class template_ : public pybind11::class_<Template<T>> {
    pybind11::module _module;
    pybind11::dict _instantiations;
  public:
    static pybind11::class_<Template<T>> _get_class(pybind11::module m, const char* n) {
        if (pybind11::hasattr(m,n)) {
            return m.attr(n);
        } else {
            return pybind11::class_<Template<T>>(m,n);
        }
    }

    static pybind11::dict _get_instantiations(pybind11::class_<Template<T>> cls) {
        if (pybind11::hasattr(cls,"_instantiations")) {
            return cls.attr("_instantiations");
        } else {
            return pybind11::dict();
        }
    }

    template_(pybind11::module m, const char* name=Ariadne::python_template_name<T>().c_str())
        : pybind11::class_<Template<T>>(_get_class(m,name)),
          _module(m),
          _instantiations(_get_instantiations(*this)) { }

    ~template_() {
        this->attr("_instantiations")=this->_instantiations;
        this->_def_class_getitem(this->_instantiations);
    }

    template<class V, class... K> void as_instantiate() {
        as_instantiate_template<V,K...>(this->_module,this->_instantiations);
    }

    template<class... K> void instantiate() {
        instantiate_template<T,K...>(this->_module,this->_instantiations);
    }

    void instantiate(const char* k, const char* v) {
        pybind11::object key=this->_module.attr(k);
        pybind11::object value=this->_module.attr(v);
        _instantiations[key]=value;
    }

    void instantiate(std::string k, std::string v) {
        this->instantiate(k.c_str(),v.c_str());
    }

    template<class F> void def_new(F&& f) {
        this->_def_new(std::forward<F>(f), (pybind11::detail::function_signature_t<F>*) nullptr);
    }

  private:
    template<class F, class R, class... ARGS>
    void _def_new(F&& f, R(*)(ARGS...)) {
        this->def("__new__", [&f](pybind11::object, ARGS... args){return f(args...);});
    }

    void _def_class_getitem(pybind11::dict instantiations) {
        this->def_static("__class_getitem__", [instantiations](pybind11::object key) {
            return instantiations[key];
        });
    }
};

} // namespace PyBind11

#endif /* ARIADNE_PYTHON_UTILITIES_HPP */
