#ifndef CURRENTSTATUS_H
#define CURRENTSTATUS_H
#include <stdint.h>

typedef struct CurrentStatus
{
    bool isInside;
    uint8_t powerStatus;
    QPointF A;
    QPointF B;
    QPointF C;
    QPointF D;
    QPointF P;
    bool Label_Time_out;
}Currentstatus;
#endif // CURRENTSTATUS_H
