#ifndef POSITION_H
#define POSITION_H

#include <QObject>
#include <Eigen/Geometry>
#include <Eigen/Dense>
#include <iostream>
#include <QDebug>
#include <QPoint>
//#include "pointxyz.h"
//class Pointxyz

using std::cout;
using std::endl;
using std::cin;
using namespace Eigen;
using namespace std;

class Position : public QObject
{
    Q_OBJECT
public:
    explicit Position(QObject *parent = 0);
    MatrixXf mat_ppow(MatrixXf, double, double);
    MatrixXf GetPosition(MatrixXf, int, MatrixXf);
    bool pInQuadrangle(QPointF a, QPointF b, QPointF c, QPointF d, QPointF p);
    MatrixXf Taylor_Solve(MatrixXf bs_pos, MatrixXf Rm1_orgi, MatrixXf init_loc);
    MatrixXf Solve_value(MatrixXf bs_pos, int BS_COUNT, MatrixXf Rm1_orgi,MatrixXf Tayor_pos,MatrixXf init_loc);
signals:

public slots:

private:
    //    const double speed = 299792458 / (128 * 499.2 * 1e6) * 1000;
    double triangleArea(QPointF a, QPointF b, QPointF c);//add lw 2017/8/21 float->double
};

#endif // POSITION_H
