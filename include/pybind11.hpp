/***************************************************************************
 *            pybind11.hpp
 *
 *  Copyright  2014-26  Pieter Collins
 *
 ****************************************************************************/

#ifndef ARIADNE_PYBIND11_HPP
#define ARIADNE_PYBIND11_HPP

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

#include <string>
#include <vector>
#include <set>
#include <map>
#include <sstream>
#include <iostream>

#if defined(_WIN32)
#  define ARIADNE_PYTHON_VISIBLE
#else
#  define ARIADNE_PYTHON_VISIBLE __attribute__ ((visibility ("default")))
#endif

namespace pybind11 {

template<class... BS> class bases { };

template<class T, class... BS> class class_<T,bases<BS...>> : public class_<T,BS...> {
  public:
    using class_<T,BS...>::class_;
};

class ARIADNE_PYTHON_VISIBLE result_object {
  public:
    explicit result_object() : _obj() { }
    explicit result_object(pybind11::object obj) : _obj(obj) { }
  public:
    template <class T> operator T() { return this->convert_to<T>(); }
    template <class T> operator T&() const { return const_cast<result_object*>(this)->convert_to<T&>(); }
  private:
    template<class T> T convert_to() {
        if (pybind11::detail::cast_is_temporary_value_reference<T>::value) {
            static pybind11::detail::override_caster_t<T> caster;
            return pybind11::detail::cast_ref<T>(std::move(this->_obj), caster);
        } else {
            return pybind11::detail::cast_safe<T>(std::move(this->_obj));
        }
    }
    pybind11::object _obj;
};

struct ARIADNE_PYTHON_VISIBLE override_function {
    override_function(pybind11::function func) : _func(func) { }
    template<class... ARGS> result_object operator() (ARGS& ... args) { return result_object(_func(args...)); }
  private:
    pybind11::function _func;
};

template<class I> class wrapper : public I {
    const char* _pyclass_name;
  protected:
    wrapper(const char* pyclass_name) : _pyclass_name(pyclass_name) { }
    override_function get_override(const char* pymethod) const {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overrider = pybind11::get_override(static_cast<const I*>(this), pymethod);
        if (overrider) {
            return override_function(overrider);
        } else {
            std::string error_msg=std::string("Tried to call pure virtual function \"")+this->_pyclass_name+"::"+pymethod+"\"";
            pybind11_fail(error_msg.c_str());
        }
    }
};

} // namespace pybind11

#undef ARIADNE_PYTHON_VISIBLE

#endif /* ARIADNE_PYBIND11_HPP */
