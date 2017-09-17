//
// Created by kurono267 on 18.09.17.
//

#include "mtlpp.hpp"

using namespace ns;

Object::Object(){}

Object::Object(const Handle& handle){}

Object::Object(const Object& rhs){}

#if MTLPP_CONFIG_RVALUE_REFERENCES
Object::Object(Object&& rhs){}
#endif

Object::~Object(){}

Object& Object::operator=(const Object& rhs){}

#if MTLPP_CONFIG_RVALUE_REFERENCES
Object& Object::operator=(Object&& rhs){}
#endif