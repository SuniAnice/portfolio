#pragma once
#include <GL/glut.h>
#include <stdio.h>
#include <fmod.hpp>
#include <iostream>
#include <fstream>
#include "jsoncpp\json\json.h"
#include "bitmap.h"


using namespace FMOD;
using namespace Json;


void re_init(bool isgameover);

void draw_result(bool isgameover);