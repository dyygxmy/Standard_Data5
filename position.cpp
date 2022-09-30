#include "position.h"
#include <QtCore/qmath.h>
#include <vector>

const double speed = 299792458 / (128 * 499.2 * 1e6) * 1000;

Position::Position(QObject *parent) : QObject(parent)
{

}

MatrixXf Position::mat_ppow(MatrixXf A, double pow, double delta)
{
    Eigen::JacobiSVD<Eigen::MatrixXf> svd(A, Eigen::ComputeFullV | Eigen::ComputeFullU); // ComputeThinU | ComputeThinV
    Eigen::MatrixXf singular_values = svd.singularValues();
    Eigen::MatrixXf left_singular_vectors = svd.matrixU();
    Eigen::MatrixXf right_singular_vectors = svd.matrixV();

    //    cout<<"singular_values:"<<endl<<singular_values<<endl;
    //    cout<<"left_singular_vectors:"<<endl<<left_singular_vectors<<endl;
    //    cout<<"right_singular_vectors:"<<endl<<right_singular_vectors<<endl;

    double lambda11 = singular_values(0,0);
    if(singular_values(0,0) != 0)
    {
        for(int i=0;i<singular_values.rows();i++)
        {
            if(singular_values(i,0)/lambda11>delta)
                singular_values(i,0) = qPow(singular_values(i,0) , pow);
            else
                singular_values(i,0) = 0;
        }
    }
    //    cout<<"singular_values:"<<endl<<singular_values<<endl;
    MatrixXf T = MatrixXf::Zero(A.rows(),A.cols());
    //    cout<<"T:"<<endl<<T<<endl;
    for(int i =0; i <min(A.rows(),A.cols()); i++)
    {
        T(i,i) = singular_values(i,0);
    }
    //    cout<<"T:"<<endl<<T<<endl;

    //    MatrixXd Eigen_Vals_Diag(singular_values.asDiagonal());
    MatrixXf P;
    if(pow >0)
        P = left_singular_vectors * T * (right_singular_vectors.transpose());
    else
        P = right_singular_vectors * (T.transpose()) * (left_singular_vectors.transpose());
    //    cout<<"P:"<<endl<<P<<endl;
    return P;
}

//add lw 2017/10/30
MatrixXf Position::Solve_value(MatrixXf bs_pos, int BS_COUNT, MatrixXf Rm1_orgi,MatrixXf Tayor_pos,MatrixXf init_loc)
{

    VectorXf dp = VectorXf::Zero(BS_COUNT - 1 )  ,
             dp2= VectorXf::Zero(BS_COUNT - 1);
    MatrixXf bs = bs_pos.transpose();
    double dp0=0.0 , dp20=0.0 ;
    for (int i = 0;i < BS_COUNT ; i ++)
    {
        VectorXf v = bs.col(i) - init_loc;
        VectorXf v2 = bs.col(i) - Tayor_pos;
        if ( i == 0 )
        {
            dp0 = sqrt(v.transpose()*v );
            dp20 = sqrt( v2.transpose()*v2 );
        }
        else
        {
            dp(i - 1 ) = sqrt(v.transpose()*v )- dp0;
            dp2(i -1 )= sqrt( v2.transpose()*v2 ) - dp20;
        }
    }
    dp = dp - Rm1_orgi ;
    dp2 = dp2 - Rm1_orgi ;

    double diff_p = dp.transpose()*dp ;
    double diff_p2 = dp2.transpose()*dp2 ;

    double M = qPow ((3*(BS_COUNT-1)) , (diff_p2/diff_p));
    double w = diff_p / diff_p2 / M ;
    init_loc = (init_loc + Tayor_pos*w) /(1+w);

    return init_loc;
}
//end
//add lw 2017/10/30
MatrixXf Position::Taylor_Solve(MatrixXf bs_pos,  MatrixXf Rm1_orgi,MatrixXf init_loc)//泰勒优化
{
    MatrixXf T = init_loc ;
    MatrixXf Rn1 = Rm1_orgi;
    MatrixXf BS = bs_pos.transpose();
    double delta_last = 0 ;
    double delta_curr = 0 ;
    //const int dim = 3 ;//3D location
    for ( int iter_cnt = 0 ; iter_cnt < 50 ; iter_cnt ++)
    {
        MatrixXf CoorDiff = MatrixXf::Zero(BS.rows(),BS.cols());
        for(int i = 0 ; i< BS.cols() ; i ++)
        {
            CoorDiff.col(i) = BS.col(i) - T ;
        }
        VectorXf Ri = MatrixXf::Zero(BS.cols(),1);
        for (int i = 0 ; i< BS.cols() ; i ++)
        {
            VectorXf ri = CoorDiff.col(i);
            Ri(i) = sqrt(ri.transpose() * ri);
        }
        VectorXf h = MatrixXf::Zero(Ri.rows() -1 , 1 );
        for (int i = 0 ; i< h.rows()  ; i ++)
        {
            h(i) = Rn1(i) - (Ri(i+1)-Ri(0));
        }
        MatrixXf tmpG = MatrixXf::Zero(BS.cols(),BS.rows());
        for ( int i = 0 ; i < Ri.rows() ;i++)
        {
            if(Ri(i)<1e-4)
            {
                return BS.col(i);
            }
            //tmpG.row(i) = ( (BS.col(i) - T)/Ri(i) ).transpose();
            MatrixXf a = (BS.col(i) - T) ;
            MatrixXf b = a / Ri(i) ;
            MatrixXf c = b.transpose();
            tmpG.row(i) = c ;
        }
        MatrixXf _G = MatrixXf::Zero(BS.rows()  ,BS.cols()-1);
        for ( int i = 0 ; i < _G.rows() ;i++)
        {
            MatrixXf a = tmpG.row(0) - tmpG.row(i+1);
            _G.col(i) = a.transpose() ;
        }
        MatrixXf G = _G.transpose();
        VectorXf delta= mat_ppow(G.transpose()*G,-1,0.1)*G.transpose()*h;
        T = T+delta ;

        if ( ( delta_curr = sqrt(delta.transpose()*delta) )  < 300
             || ( delta_curr < delta_last && iter_cnt >10 ) )
        {
            break ;
        }
        delta_last = delta_curr ;
    }
    return T ;

}
//end

MatrixXf Position::GetPosition(MatrixXf bs_pos, int COUNT, MatrixXf diffTime)
{
//    MatrixXf bs_pos = MatrixXf::Zero(5,3);
//    bs_pos  <<   0,    0,  350,
//            0, 3070 , 350,
//            2750 ,3070 , 350,
//            2750, 1000 , 350,
//            1520, 2340 , 350;
//    COUNT = 5;
////    MatrixXf diffTime(4,1);
//    diffTime << -307,
//            199,
//            304,
//            54;
    MatrixXf p = MatrixXf::Zero(3,1);
//    MatrixXf p = MatrixXf::Zero(3,1);
    //    MatrixXf p(3,1);
    //    Pointxyz *p = new Pointxyz();
    //    qDebug()<<p->X<<p->Y<<p->Z;
    MatrixXf pos = MatrixXf(COUNT,3);
    pos = bs_pos;
    MatrixXf ka = MatrixXf(COUNT,1);
    for(int i= 0;i<COUNT;i++)
    {
        ka(i,0)=qPow(pos(i,0),2)+qPow(pos(i,1),2)+qPow(pos(i,2),2);
    }
    MatrixXf h = MatrixXf(COUNT-1,1);
    for(int i= 0;i<COUNT-1;i++)
    {
        h(i,0)=ka(i+1,0)-ka(0,0)-qPow(diffTime(i,0)*speed,2);
    }
    MatrixXf Ga_tmp = MatrixXf(COUNT-1,4);
    for(int i= 0;i<COUNT-1;i++)
    {
        Ga_tmp(i,0) = pos(i+1,0)-pos(0,0);
        Ga_tmp(i,1) = pos(i+1,1)-pos(0,1);
        Ga_tmp(i,2) = pos(i+1,2)-pos(0,2);
        Ga_tmp(i,3) = diffTime(i,0)*speed;
    }

    MatrixXf Ga = 2 * Ga_tmp;
    MatrixXf Q = MatrixXf::Identity(COUNT-1, COUNT-1);
    Ga = mat_ppow(Q,-0.5,0) * Ga;
    h = mat_ppow(Q,0.5,0) * h;
    MatrixXf Ga_pinv = MatrixXf(COUNT-1,4);
    Ga_pinv = mat_ppow(Ga,-1,1.0f/100);
    MatrixXf Za0 = MatrixXf(COUNT-1,1);
    Za0 = Ga_pinv * h;

    if (Za0(3,0) < 0)
    {
        p(0,0) = Za0(0,0);
        p(1,0) = Za0(1,0);
        p(2,0) = Za0(2,0);
        return p;
    }

    VectorXf Va(COUNT-1);
    for(int i =0 ;i<COUNT-1;i++)
    {
        Va[i] = qSqrt( qPow( (pos(i+1,0)-Za0(0,0)),2) + qPow( (pos(i+1,1)-Za0(1,0)),2) + qPow( (pos(i+1,2)-Za0(2,0)),2) );
    }

    MatrixXf B(Va.asDiagonal());
    MatrixXf FI = MatrixXf(COUNT-1,COUNT-1);
    FI = B * Q * B;
    MatrixXf Ga1 = mat_ppow(FI,-0.5,0)*Ga;
    MatrixXf h1 = mat_ppow(FI,-0.5,0)*h;
    MatrixXf Ga1_pinv = mat_ppow(Ga1,-1,1.0f/100);
    MatrixXf Za1 = Ga1_pinv * h1;
    if (Za1(3,0) < 0)
    {
        p(0,0) = Za1(0,0);
        p(1,0) = Za1(1,0);
        p(2,0) = Za1(2,0);
        return p;
    }

    MatrixXf CovZa = ((Ga.transpose())*(FI.inverse())*Ga).inverse();
    VectorXf Va2(4);
    Va2<<Za1(0,0)-pos(0,0),Za1(1,0)-pos(1,0),Za1(2,0)-pos(2,0),Za1(3,0);
    MatrixXf SB(Va2.asDiagonal());
    MatrixXf SFI = 4*SB*CovZa*SB;
    MatrixXf SGa(4,3);
    SGa << 1,0,0,
            0,1,0,
            0,0,1,
            1,1,1;
    MatrixXf Sh(4,1);
    Sh << qPow(Za1(0,0)-pos(0,0),2),qPow(Za1(1,0)-pos(1,0),2),qPow(Za1(2,0)-pos(2,0),2),qPow(Za1(3,0),2);
    MatrixXf SGa1 = mat_ppow(SFI,-0.5,0)*SGa;
    MatrixXf Sh1 = mat_ppow(SFI,-0.5,0)*Sh;
    MatrixXf SGa1_pinv = mat_ppow(SGa1,-1,0.1f/100);
    MatrixXf Za2 = SGa1_pinv * Sh1;

    if ( Za2(0,0) < 0 || Za2(0,0) < 0 || Za2(0,0) < 0 )
    {
        p(0,0) = Za1(0,0);
        p(1,0) = Za1(1,0);
        p(2,0) = Za1(2,0);
        return p;
    }

    VectorXf Va3(3);
    Va3<<qSqrt(Za2(0,0)),qSqrt(Za2(1,0)),qSqrt(Za2(2,0));
    MatrixXf TMP(8,3);
    TMP << 1,1,1,
            1,1,-1,
            1,-1,1,
            1,-1,-1,
            -1,1,1,
            -1,1,-1,
            -1,-1,1,
            -1,-1,-1;

    MatrixXf M(Va3.asDiagonal());
    MatrixXf out = TMP * M;
    for(int i=0;i<8;i++)
    {
        out(i,0)=out(i,0)+pos(0,0);
        out(i,1)=out(i,1)+pos(0,1);
        out(i,2)=out(i,2)+pos(0,2);

    }

    double m =0,n =0, j=0;
    for(int i=0; i<8; i++)
    {
        m = qSqrt(qPow(out(i,0)-Za1(0,0),2)+qPow(out(i,1)-Za1(1,0),2)+qPow(out(i,2)-Za1(2,0),2));
        if(i == 0)
            n = m;
        if(m < n)
        {
            n = m;
            j = i;
        }
    }
    if (n < 600)
    {
        p(0,0) = out(j,0);
        p(1,0) = out(j,1);
        p(2,0) = out(j,2);
    }
    else
    {
        p(0,0) = Za1(0,0);
        p(1,0) = Za1(1,0);
        p(2,0) = Za1(2,0);
    }
    return p;
}

bool Position::pInQuadrangle(QPointF a, QPointF b, QPointF c, QPointF d,
                             QPointF p)
{
 //   qDebug()<<"triangleArea ="<<QString::number(triangleArea(a, b, p), 10, 2)<<QString::number(triangleArea(b, c, p), 10, 2)<<QString::number(triangleArea(c, d, p), 10, 2)<<QString::number(triangleArea(d, a, p), 10, 2);
    double dTriangle = triangleArea(a, b, p) + triangleArea(b, c, p)
            + triangleArea(c, d, p) + triangleArea(d, a, p);
    double dQuadrangle = triangleArea(a, b, c) + triangleArea(c, d, a);
//    qDebug()<<"dTriangle ="<<QString::number(dTriangle,10,2)<<QString::number(dQuadrangle,10,2);
    return dTriangle <= dQuadrangle+40;//add lw 2017/8/21 +40 去除漂移误差
}
// 返回三个点组成三角形的面积
double Position::triangleArea(QPointF a, QPointF b, QPointF c) {
     double result  = qAbs(a.x()*(b.y()-c.y())+b.x()*(c.y()-a.y())+c.x()*(a.y()-b.y()))/2.0;//add lw 2017/8/21
    return result;
}
