#pragma once

#include "data_comm.h"

#ifndef MOPROBO_API
#define MOPROBO_API extern
#endif

namespace MoproboGui {

class PointsBuffer {
public:
private:
    ScrollingBuffer m_buffer;
};

};  // namespace MoproboGui