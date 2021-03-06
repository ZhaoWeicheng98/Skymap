//
// Created by 70700 on 2019/3/3.
//

#ifndef SKYMAP_MYFUNCTIONS_H
#define SKYMAP_MYFUNCTIONS_H
#include <math.h>
#include <iostream>
#include <algorithm>
#include <stdlib.h>
#include <stdio.h>
#include <unordered_map>
#include <algorithm>
#include <QDebug>
#include <algorithm>

double cal_dis(const double& x1,const double& y1,const double& x2,const double& y2);
bool between(const double& target, const double& left, const double& right);
double getSpotAD(const double& x1, const double& y1, const double& x2, const double& y2, const double& f);
double random_double(const double& l, const double& r);
int random_int(const int& l,const int& r);
size_t random_size_t(const size_t& l,const size_t& r);
double getSphereAD(double, double, double, double);
double getSphereAngle(double, double, double, double, double, double);
double getSpotAngle(double, double, double, double, double, double,double);
#endif //SKYMAP_MYFUNCTIONS_H
