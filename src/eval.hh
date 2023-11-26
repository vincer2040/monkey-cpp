#pragma once

#include "object.hh"
#include "ast.hh"

Object eval(Program& program, std::shared_ptr<Environment> env);
