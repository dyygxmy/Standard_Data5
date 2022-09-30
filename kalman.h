#ifndef KALMAN_H
#define KALMAN_H
// 一维滤波器信息结构体
typedef  struct{
    float filterValue;  //k-1时刻的滤波值，即是k-1时刻的值
    float kalmanGain;   //   Kalamn增益
    float A;   // x(n)=A*x(n-1)+u(n),u(n)~N(0,Q)
    float H;   // z(n)=H*x(n)+w(n),w(n)~N(0,R)
    float Q;   //预测过程噪声偏差的方差
    float R;   //测量噪声偏差，(系统搭建好以后，通过测量统计实验获得)
    float P;   //估计误差协方差
}  KalmanInfo;
void Init_KalmanInfo(KalmanInfo* info, float Q, float R);
float KalmanFilter(KalmanInfo* kalmanInfo, float lastMeasurement);
/*
 * Kalman 过程详解：
(1)预测：做出先验估计x[n|n-1]=A*x[n-1|n-1];
- 对于一维的情况，A可以看成一个常数使用，经常取1，同时对于B经常取零(—可能有人会有疑问：取0没事吗，可以放心的告诉你，问题不大。反过来想想，
这只是一个估计，可以在估计噪声方差得到修正)
(2)向前推算协方差：做出预测后的新的概率分布的方差（预测上次的最优估计为当前时刻的先验估计这个过程可以当成一个符合预测过程噪声分布的和另一个
(上一次的最优估计可以看做高斯分布的)也符合高斯分布的相加。预测结果也是符合高斯噪声分布的，方差是两个相互独立的方差之和）。
- 对于一维的情况,P[n|n-1]=P[n-1|n-1]+Q。 Q为预测方差，代表对预测的不信任程度，工程上根据实际调节以改善滤波器的性能:动态效果和去噪效果
(3)计算卡尔曼增益
    对于一维的情况，K[n]=H*P[n|n-1]\/{H^2*P[n|n-1]+R}。其中H是对观测的响应倍数，通常取1，R为测量的方差，工程上一般都可以直接获得
(4)更估计值： 做出后验估计，修正后的估计值，更接近真实值。
- 对于一维的情况，最优估计由下式给出：
x[n|n]=x[n|n-1]+K[n]*{z[n]-x[n|n-1]}。其中z[n]为观测值

(5)更新误差协方差：得到最优估计的概率分布的方差。
- 对于一维的情况，新的误差协方差由下式给出：
    P[n|n]=(1-K[n]*H)*P[n|n-1]
*/
#endif // KALMAN_H
