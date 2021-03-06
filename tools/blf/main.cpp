//
//  main.cpp
//  Nesting Problem
//
//  Created by Yang Shan on 2020/4/13.
//  Copyright © 2020 Tongji SEM. All rights reserved.
//

#include <iostream>
#include <vector>
#include <stdbool.h>
#include <csv/reader.hpp>
#include "geometry.cpp"
#include "plot.cpp"
#include <time.h>
#include <algorithm>


class BLF{
protected:
    int polygons=0; // 输入形状可以考虑对应的序列
    PolysArrange polys_arrange; // 形状的排样情况
    NFPAssistant *nfp_assistant;
public:
    BLF(){
        DataAssistant::readData(polys_arrange); // 加载数据
        nfp_assistant=new NFPAssistant("/Users/sean/Documents/Projects/Data/fu_clock.csv",polys_arrange.type_num,4);
        
    };
    void run(){
        placeFirstPoly();
        int _max=int(polys_arrange.polys.size());
        for(int j=1;j<_max;j++){
            placeNextPoly(j);
        }
    };
    void placeFirstPoly(){
        VectorPoints ifr;
        PackingAssistant::getIFR(polys_arrange.polys[0], polys_arrange.width, 99999999, ifr);
        PackingAssistant::slideToPosition(polys_arrange.polys[0], ifr[3]);
        PrintAssistant::print2DVector(polys_arrange.polys[0], true);
    };
    void placeNextPoly(int j){
        // 获得IFR
        VectorPoints ifr;
        PackingAssistant::getIFR(polys_arrange.polys[j], polys_arrange.width, 99999999, ifr);
        // IFR转Polygon并计算差集
        Polygon IFR;
        GeometryProcess::convertPoly(ifr,IFR);
        list<Polygon> feasible_region={IFR};
//        cout<<endl<<"IFR:"<<dsv(IFR)<<endl;
        // 逐一计算
        for(int i=0;i<j;i++){
            // 初步处理NFP多边形
            VectorPoints nfp;
            int type_i=polys_arrange.polys_type[i],type_j=polys_arrange.polys_type[j];
            int oi=polys_arrange.polys_orientation[i],oj=polys_arrange.polys_orientation[j];
            nfp_assistant->getNFP(type_i,type_j,oi,oj,polys_arrange.polys[i],nfp);
            // 转化为多边形并求交集
            Polygon nfp_poly;
            GeometryProcess::convertPoly(nfp,nfp_poly);
            PolygonsOperator::polysDifference(feasible_region,nfp_poly);
            
        };
        // 遍历获得所有的点
        VectorPoints all_points;
        GeometryProcess::getAllPoints(feasible_region,all_points);
        // 选择最左侧的点
        vector<double> bl_point;
        PackingAssistant::getBottomLeft(all_points,bl_point);
        PackingAssistant::slideToPosition(polys_arrange.polys[j],bl_point);
        PrintAssistant::print2DVector(polys_arrange.polys[j], true);
    }
};

class LPSearch{
protected:
    VectorPoints cur_ifr; // 获得当前的IFR
    double poly_num;
    vector<VectorPoints> all_nfps; // 获得当前全部NFP
    vector<Polygon> all_nfps_poly; // 获得当前全部NFP的Polygon模型
    vector<vector<vector<double>>> edges_target_funs,points_target_funs; // 分别获得直线和点的目标函数
    
    PolysArrange best_solution,cur_solution; // 最佳解和当前解，不需要用指针
    vector<VectorPoints> best_polys,polys; // 暂存当前的全部形状
    vector<int> cur_orientation; // 当前采用的方向
    vector<int> polys_type; // 所有形状情况
    double best_length,cur_length,width; // 长度等性质

    vector<vector <double>> poly_overlap; // 重叠情况
    vector<vector <double>> nfp_overlap_pair; // NFP的重叠情况
    vector<vector <double>> nfp_overlap_list; // 每个NFP的重叠对象（计算相交）
    vector<vector <double>> miu; // 重叠情况
    vector<vector <VectorPoints>> all_polygons; // 全部形状的加载
    
    int choosed_index; // 当前选择的index
        
    NFPAssistant *nfp_assistant; // NFP辅助操作
    
    vector<list<Polygon>> nfp_sub_ifr; // 除去IFR后的NFP，全部存为Vector<VectorPoints>（计算相交情况）
    vector<vector<list<Polygon>>> nfp_sub_ifr_phase; // 各个阶段的NFP的记录
    vector<vector<vector<int>>> target_indexs; // 获得各个阶段交集的目标区域
public:
    LPSearch(){
        DataAssistant::readBLF(1,best_solution); // 读取BLF的解
        cur_solution=best_solution; // 赋值当前解
        nfp_assistant=new NFPAssistant("/Users/sean/Documents/Projects/Data/fu_clock.csv",best_solution.type_num,4); // NFP辅助

        polys=cur_solution.polys;
        cur_orientation=cur_solution.polys_orientation; // 赋值全部形状（变化）
        cur_length=cur_solution.length; // 当前长度（变化）
        best_length=best_solution.length; // 最佳长度（变化）

        width=best_solution.width; // 宽度（不变）
        poly_num=best_solution.total_num; // 形状总数（不变）
        polys_type=cur_solution.polys_type; // 形状类别（不变）
        
        PltFunc::pltShow(polys);
    };
    void run(){
        // 限制检索时间是否超过时间
        slideToContainer();
        
        // 执行最小化重叠后，判断解是否可行
        minimizeOverlap();
        
    };
    // 输入序列形状，每次处理一个序列
    void minimizeOverlap(){
        initial2DVector(1,poly_num,miu); // 初始化Miu的值
        int it=0,N=1; // 循环次数限制
        double lowest_overlap=0; // 重叠的处理
        
        // 选择一个形状
        while(it<N){
            // 生成一个序列，逐一赋值并寻找更优位置
            vector<int> search_permutation;
            randomPermutation(search_permutation);
            for(auto index:search_permutation){
                // 检索该形状各个方向是否有更优解
                choosed_index=index;
                searchOnePoly();
            }
            
            // 获得本序列结束后的情况
            double cur_overlap=getTotalOverlap();
            if(cur_overlap<BIAS){
                cout<<"本次计算结束，没有重叠"<<endl;
                break;
            }else if (cur_overlap<lowest_overlap){
                lowest_overlap=cur_overlap;
                it=0;
            }
            // 更新Miu和循环次数，开始下一次检索
            cout<<"当前重叠:"<<cur_overlap<<endl;
            updateMiu();
            it++;
        };
        if(N==it){
            cout<<"超出检索次数"<<endl;
        }
        
    };
    
    /*
     寻找某个形状的更优位置（采用全局的choose_index）
     1. 只有获得了NFPIFR，并获得了目标函数后，才可以求某点深度（通
     过遍历与目标形状有重叠的NF的边和点）
     2. 如果需要求最值点，则需要计算出NFP的拆分情况，然后分别对各个
     点求NFP，
     */
    void searchOnePoly(){
        // 判断当前选择形状是否有重叠
        if(judgePositionOverlap()==true){
            return;
        }
        // 获得初始位置，后续求解最优位置，需要比较
        vector<double> original_position;
        PackingAssistant::getReferPt(polys[choosed_index], original_position);
        
        // 获得全部NFP和IFR，外加所有多边形的目标函数
        getNFPIFR(choosed_index,cur_orientation[choosed_index]);
        getTargetFunc();
        
        // 计算最佳的位置
        double best_depth=getInitialDepth(original_position);
        int best_orientation=cur_orientation[choosed_index];
        vector<double> best_position=original_position;
        
        // 测试每个角度检索位置，获得NFP、IFR，再寻找最佳的位置
        for(int orientation:{0,1,2,3}){
            // 获得对应的NFP和IFR
            getNFPIFR(choosed_index,orientation);
            
            // 判断IFR除去NFP之后是否还有空隙，若有，则更新并返回
            list<Polygon> ifr_sub_nfp;
            getIfrSubNfp(ifr_sub_nfp);
            if(PackingAssistant::totalArea(ifr_sub_nfp)<BIAS){
                // 更新全部对象退出
                best_depth=0;
                best_orientation=orientation;
//                best_position=;
                break;
            }
            
            // 获得目标函数以及香蕉情况
            getTargetFunc();
            getInterPoints();
            
            // 检索并获得最佳位置
            vector<double> new_poistion;
            double new_depth=searchForBestPosition(new_poistion);
            
            // 比较新的结果是否需要更新最值，需要更新则更新深度、最佳位置和最佳方向
            if(new_depth<best_depth){
                best_depth=new_depth;
                best_position=new_poistion;
                best_orientation=orientation;
            }
        }
        
        // 如果最佳的位置和原始位置不一样，则移动当前解，并更新重叠
        if(best_position[0]!=original_position[0]&&best_position[1]!=original_position[1]){
            polys[choosed_index]=all_polygons[choosed_index][best_orientation]; // 更新选择形状
            PackingAssistant::slideToPosition(polys[choosed_index], best_position); // 移到最佳位置
            updatePolyOverlap(choosed_index); // 更新全部重叠
        }

    };
    
    // 判断某个形状当前位置是否有重叠
    bool judgePositionOverlap(){
        for(auto item:poly_overlap[choosed_index]){
            if(item>0){
                return true;
            }
        }
        return false;
    };
    
    // 获得某个形状的全部NFP和IFR
    void getNFPIFR(int j,int oj){
        // 获得全部的NFP
        all_nfps={};
        all_nfps_poly={};
        for(int i=0;i<poly_num;i++){
            // 如果等于则直接加空的
            if(i==j){
                all_nfps.push_back({});
                continue;
            }
            // 获得对应的NFP
            VectorPoints nfp;
            nfp_assistant->getNFP(polys_type[i], polys_type[j], cur_orientation[i], oj, polys[j], nfp);
            all_nfps.push_back(nfp);
            // 获得对应NFP的Polygon模型
            Polygon nfp_poly;
            GeometryProcess::convertPoly(nfp, nfp_poly);
            all_nfps_poly.push_back(nfp_poly);
        }
        // 获得IFR形状
        cur_ifr={};
        PackingAssistant::getIFR(cur_solution.polys[j], width, cur_length, cur_ifr);
    };
    
    // 在获得NFP和IFR之后获得目所有目标函数
    void getTargetFunc(){
        for(int i=0;i<poly_num;i++){
            // 初始化添加
            edges_target_funs.push_back({});
            points_target_funs.push_back({});
            // 如果相同则跳过
            if(i==choosed_index){
                continue;
            }
            // 首先计算全部的直线
            vector<VectorPoints> all_edges;
            getAllEdges(all_nfps[i], all_edges);
            for(auto edge:all_edges){
                vector<double> coeff;
                getEdgeCoeff(coeff,edge);
                edges_target_funs[i].push_back(coeff);
            }
            // 其次遍历所有的点（其实不用封装也OK）
            for(auto pt:all_nfps[i]){
                points_target_funs[i].push_back({pt[0],pt[1]});
            }
        }
    };
    
    // 获得某个位置与某个形状的最低高度
    double getPointPolyDepth(int poly_index,vector<double> pt){
        double min_depth=9999999;
        // 遍历全部的边对象 abs(ax+by+c)
        for(auto edge_target:edges_target_funs[poly_index]){
            double value=abs(edge_target[0]*pt[0]+edge_target[1]*pt[1]+edge_target[2]);
            if(value<min_depth){
                min_depth=value;
            }
        }
        // 遍历全部的点 abs(x-x0)+abs(y-y0)
        for(auto point_target:points_target_funs[poly_index]){
            double value=abs(pt[0]-point_target[0])+abs(pt[1]-point_target[1]);
            if(value<min_depth){
                min_depth=value;
            }
        }
        return min_depth;
    };
    
    // 获得某个位置的最低Depth，通过点的位置和对应的NFP
    double getPointDepth(vector<double> pt,vector<int> all_nfp_indexs){
        double depth=0;
        for(auto nfp_index:all_nfp_indexs){
            depth+=getPointPolyDepth(nfp_index, pt);
        }
        return depth;
    };
    
    // 在初始化之后获得该形状的初始深度
    double getInitialDepth(vector<double> pt){
        double poly_depth=0;
        for(int i=0;i<poly_num;i++){
            // 如果没有重叠直接跳过（重叠情况每次变更都会更新）
            if(poly_overlap[choosed_index][i]==0||i==choosed_index){
                continue;
            }
            poly_depth+=getPointPolyDepth(i,pt);
        }
        return 0;
    };
    
    // 判断IFR除去全部NFP后还有剩余
    void getIfrSubNfp(list<Polygon> &feasible_region){
        // 计算IFR以及可行区域
        Polygon IFR;
        GeometryProcess::convertPoly(cur_ifr, IFR);
        feasible_region={IFR};
        // 逐步遍历全部NFP求差集
        for(auto nfp:all_nfps){
            Polygon nfp_poly;
            GeometryProcess::convertPoly(nfp, nfp_poly);
            PolygonsOperator::polysDifference(feasible_region, nfp_poly);
        }
    };
    
    // 获得一阶NFP除去IFR的情况
    void getNfpSubIfr(){
        // 首先获得IFR的情况
        Polygon ifr_poly;
        GeometryProcess::convertPoly(cur_ifr, ifr_poly);
        // 辅助计算最终情况的函数
        nfp_sub_ifr_phase={{}};
        target_indexs.push_back({});
        // 获得除去IFR的后的
        for(int i=0;i<poly_num;i++){
            // 先求NFP
            Polygon nfp_poly;
            auto nfp=all_nfps[i];
            GeometryProcess::convertPoly(nfp, nfp_poly);
            // 再求划分之后的结果
            list<Polygon> final_nfp={nfp_poly};
            PolygonsOperator::polysDifference(final_nfp, ifr_poly);
            nfp_sub_ifr.push_back(final_nfp);
            // 阶段性计算存储
            nfp_sub_ifr_phase[0].push_back(final_nfp);
            target_indexs[0].push_back({});
            target_indexs[0][0].push_back(i);
        }
    };
    
    // 获得全部的交点和对应的三角形
    void getInterPoints(){
        // 获得NFP重叠和切除情况
        getNFPOverlap();
        getNfpSubIfr();
        
        // 求解下一阶段以及上一阶段切除
        int pair_num=2;
        while(true){
            nfp_sub_ifr_phase.push_back({});
            // 获得每个阶段上一个的最后一个index的匹配结果
            for(int i=0;i<nfp_sub_ifr_phase[pair_num-1].size();i++){
                auto target_father=target_indexs[pair_num-1][i][target_indexs[pair_num-1][i].size()-1];
                for(int j:nfp_overlap_list[target_father]){
                    list<Polygon> inter_region;
                    PolygonsOperator::polysIntersection(nfp_sub_ifr_phase[pair_num-1][i],all_nfps_poly[j],inter_region);
                    if(PolygonsOperator::judgeListEmpty(inter_region)==false){
                        nfp_sub_ifr_phase[pair_num-1].push_back(inter_region);
                        auto new_target_indexs = target_indexs[pair_num-1][i];
                        new_target_indexs.push_back(j);
                        target_indexs[pair_num-1].push_back(new_target_indexs);
                    }
                }
            }
            if(nfp_sub_ifr_phase[pair_num-1].size()==0){
                break;
            }
            subFrontRegion(pair_num);
            pair_num++;
        }
        
    };
    
    // 切除上一阶段的计算结果(不考虑速度暂时)
    void subFrontRegion(int pair_num){
        for(int i=0;i<target_indexs[pair_num-2].size();i++){
            auto front_indexs=target_indexs[pair_num-2][i];
            for(int j=0;j<target_indexs[pair_num-1].size();j++){
                auto cur_indexs=target_indexs[pair_num-1][j];
                if(containIndex(cur_indexs,front_indexs)==true){
                    PolygonsOperator::polyListDifference(nfp_sub_ifr_phase[pair_num-2][i],nfp_sub_ifr_phase[pair_num-1][j]);
                }
            }
        }
    };
        
    // 获得最小的Penetration Depth的位置
    double searchForBestPosition(vector<double> &position){
        double min_depth=9999999999;
        for(int i=0;i<nfp_sub_ifr_phase.size();i++){
            auto phase=nfp_sub_ifr_phase[i];
            for(int j=0;j<phase.size();j++){
                // 每个List Item都是来源相同的一个或多个多边形
                vector<vector<double>> all_points;
                GeometryProcess::getAllPoints(phase[j],all_points);
                for(auto pt:all_points){
                    double depth=getPointDepth(pt, target_indexs[i][j]);
                    if(depth<min_depth){
                        min_depth=depth;
                        position={pt[0],pt[1]};
                    }
                }
            }
        }
        return 0;
    };
    
    // 获得NFP的重叠情况
    void getNFPOverlap(){
        // 初始化两个数组
        initial2DVector(0, poly_num, nfp_overlap_pair);
        initial1DVector(poly_num,nfp_overlap_list);
        // 逐步判断和求解
        for(int i=0;i<poly_num-1;i++){
            nfp_overlap_pair.push_back({});
            for(int j=i+1;j<poly_num;j++){
                if(PackingAssistant::judgeOverlap(all_nfps[i],all_nfps[j])==true){
                    // List修改，只考虑升序
                    nfp_overlap_list[i].push_back(j);
//                    nfp_overlap_list[j].push_back(i);
                    // Pair修改
                    nfp_overlap_pair[i][j] = 1;
                    nfp_overlap_pair[j][i] = 1;
                }
            }
        }
    };
    
    // 增加获得全部重叠，用于比较情况
    double getTotalOverlap(){
        double total_overlap=0;
        for(int i=0;i<poly_num-1;i++){
            for(int j=i+1;j<poly_num;j++){
                if(i==j){
                    continue;
                }
                total_overlap+=PackingAssistant::overlapArea(polys[i],polys[j]);
            }
        }
        return total_overlap;
    };
    
    // 初始化当前的整个重叠（两两间的重叠）
    void initialPolyOverlap(){
        initial2DVector(0,poly_num,poly_overlap);
        for(int i=0;i<poly_num-1;i++){
            for(int j=i+1;j<poly_num;j++){
                double overlap=PackingAssistant::overlapArea(polys[i], polys[j]);
                poly_overlap[i][j]=overlap;
                poly_overlap[j][i]=overlap;
            }
        }
    };
    
    // 更新多边形的重叠情况（单独更新某个形状）
    void updatePolyOverlap(int i){
        for(int j=0;j<poly_num;j++){
            if(i==j){
                continue;
            }
            double overlap=PackingAssistant::overlapArea(polys[i], polys[j]);
            poly_overlap[j][i]=overlap;
            poly_overlap[i][j]=overlap;
        }
    };
    
    // 平移多边形到内部
    void slideToContainer(){
        for(auto poly:polys){
            vector<double> right_pt;
            PackingAssistant::getRightPt(poly,right_pt);
            if(right_pt[0]>cur_length){
                PackingAssistant::slidePoly(poly, cur_length-right_pt[0], 0);
            }
        }
    };
    
    /*
     以下函数可以封装到PackingAssistant中！
     */
    
    // 判断当前解是否可行（获得重叠）
    bool judgeFeasible(){
        for(int i=0;i<poly_num-1;i++){
            for(int j=i+1;j<poly_num;j++){
                if(poly_overlap[i][j]>0){
                    return false;
                }
            }
        }
        return true;
    };
    
    // 获得某个多边形所有的边
    void getAllEdges(VectorPoints poly,vector<VectorPoints> &all_edges){
        for(int i=0;i<poly_num;i++){
            if(i==poly_num-1){
                all_edges.push_back({poly[i],poly[0]});
            }else{
                all_edges.push_back({poly[i],poly[i+1]});
            }
        }
    };
    
    // 获得点到直线的距离的系数参数
    void getEdgeCoeff(vector<double> coeff, VectorPoints edge){
        double A=edge[0][1]-edge[1][1];
        double B=edge[1][0]-edge[0][0];
        double C=edge[0][0]*edge[1][1]-edge[1][0]*edge[0][1];
        double D=sqrt(A*A+B*B);
        coeff={A/D,B/D,C/D};
    };
    
    // 初始化二维数组
    void initial2DVector(double initial_value,double initial_size,vector<vector<double>> target){
        target={};
        for(int i=0;i<initial_size;i++){
            target.push_back({});
            for(int j=0;j<initial_size;j++){
                target[i].push_back(initial_value);
            }
        }
    };
    
    // 初始化一维数组
    void initial1DVector(double initial_size,vector<vector<double>> target){
        target={};
        for(int i=0;i<initial_size;i++){
            target.push_back({});
        }
    };
    
    // 获得随机序列
    void randomPermutation(vector<int> permutation){
        srand((unsigned)time(NULL));
        for(int i=0;i<poly_num;i++){
            permutation.push_back(i);
        }
        random_shuffle(permutation.begin(),permutation.end());
    };
    
    // 更新权重参数Miu
    void updateMiu(){
        // 寻找最大的重叠
        double _max=0;
        for(auto line:poly_overlap){
            for(auto item:line){
                if(item>_max){
                    _max=item;
                }
            }
        }
        // 更新全部的Miu
        for(int i=0;i<poly_num;i++){
            for(int j=0;j<poly_num;j++){
                miu[i][j]+=poly_overlap[i][j]/_max;
                miu[j][i]+=poly_overlap[j][i]/_max;
            }
        }
    };

    // 判断是否包含上一个的全部index
    bool containIndex(vector<int> container,vector<int> target){
        int wrong_num=0,i=0,j=0;
        while(true){
            if(container[i]!=target[j]){
                if(wrong_num==0){
                    wrong_num++;
                    i++;
                }else{
                    return false;
                }
                i++; j++;
            }
        }
        return true;
    }
};

int main(int argc, const char * argv[]) {
    LPSearch *lp;
    lp=new LPSearch();
//    lp->run();

//    WriterAssistant::writeCSV();
    
    
//    BLF *blf;
//    blf=new BLF();
//    blf->run();
//        clock_t start,end;
//        start=clock();
//        end=clock();
//        cout<<"运行总时间"<<(double)(end-start)/CLOCKS_PER_SEC<<endl;

    return 0;
}

