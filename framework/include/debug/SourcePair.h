// Copyright 2025 solar-mist

#ifndef BASILISK_FRAMEWORK_DEBUG_SOURCE_PAIR_H
#define BASILISK_FRAMEWORK_DEBUG_SOURCE_PAIR_H 1

#include "lexer/SourceLocation.h"

using namespace basilisk;

struct SourcePair
{
    lexer::SourceLocation start;
    lexer::SourceLocation end;
};

#endif // BASILISK_FRAMEWORK_DEBUG_SOURCE_PAIR_H