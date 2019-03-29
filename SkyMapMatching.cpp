//
// Created by 70700 on 2019/3/3.
//

#include "SkyMapMatching.h"
#include <fstream>

void SkyMapMatching::LoadSky(string &f_name) {
    CSVAdapter csv_sky(f_name);
    Star s;
    //int count = 0;
    while(csv_sky.hasRecord()){
        s = csv_sky.getNextRecord();
        StarPoint sp(s.getID(),s.getX(),s.getY(),s.getMag());
        this->sky_.stars_.push_back(sp);
        this->sky_.count_++;
    }
    this->sky_.range_ = {360,180};
    this->sky_.centre_ = StarPoint(-1,180.0,0,0);
}

void SkyMapMatching::LoadImage(string &f_name) {
    //Not completed now.
    CSVAdapter csv_sky(f_name);
    Star s;
    //int count = 0;
    while(csv_sky.hasRecord()){
        s = csv_sky.getNextRecord();
        StarPoint sp(s.getID(),s.getX(),s.getY(),s.getMag());
        this->image_.stars_.push_back(sp);
        this->image_.count_++;
    }
    this->image_.RangeStandardization();
}

void SkyMapMatching::SelectTargetStar() {
    float min_dis = INT32_MAX;
    int target = 0;
    for(int i=0;i<this->image_.stars_.size();i++){
        StarPoint s = this->image_.stars_[i];
        float dis = pow(s.x,2)+pow(s.y,2);
        if(dis<min_dis){
            min_dis = dis;
            target = i;
        }
    }
    this->__target_star = this->image_.stars_[target];
}


int SkyMapMatching::TriangleModel() {
    TriangleMatching TM(sky_.stars_.size(), 12.0, 0.02);
    TM.LoadData(sky_.stars_);

    vector<StarPoint> triangle;
    triangle.push_back(this->__target_star);

    float dis12,dis13,dis23;
    while(true){
        TM.ChooseAdjacentStars(image_.stars_,triangle);
        dis12 = cal_dis(triangle[0].x,triangle[0].y,triangle[1].x,triangle[1].y);
        dis13 = cal_dis(triangle[0].x,triangle[0].y,triangle[2].x,triangle[2].y);
        dis23 = cal_dis(triangle[1].x,triangle[1].y,triangle[2].x,triangle[2].y);
        if(dis12 < TM.GetThreshold() && dis13 < TM.GetThreshold() && dis23 < TM.GetThreshold()) break;
        triangle.pop_back();
        triangle.pop_back();
    }

    int result_number = TM.MatchAlgorithm(dis12,dis13,dis23,triangle[0].magnitude,triangle[1].magnitude,triangle[2].magnitude);

    cout<<"Match Ended!"<<endl;
    return result_number-1; //编号和index差1
}

void SkyMapMatching::Match() {
    int skymap_index = TriangleModel();
    this->__matching_star = this->sky_.stars_[skymap_index];
}

bool star_point_cmp(const StarPoint &s1, const StarPoint &s2){
    float dis1 = sqrt(pow(s1.x,2)+pow(s1.y,2));
    float dis2 = sqrt(pow(s2.x,2)+pow(s2.y,2));
    return dis1<dis2;
}

bool similar_position(StarPoint &s1, StarPoint &s2){
    if(abs(s1.Distance(s2)) <0.001) return true;
    else return false;
}

bool similar_vector(vector<StarPoint> &vec1, vector<StarPoint> &vec2){
    /*TODO:
     * compare two vector,
     * without consideration of the absolute position, but the relative position.*/
    return true;
}

bool SkyMapMatching::Check() {
    vector<StarPoint> check_set = sky_.Subset(this->__matching_star,this->image_.range_.first,this->image_.range_.second);
    for(int i=0;i<check_set.size();i++){
        check_set[i].change_coordinate(this->__matching_star);
//        check_set[i].x -= this->__matching_star.x;
//        check_set[i].y -= this->__matching_star.y;
    }

    sort(check_set.begin(),check_set.end(),star_point_cmp);

    vector<StarPoint> candidate_set(this->image_.stars_);
    StarPoint new_centre = this->__target_star;
    for(int i=0;i<candidate_set.size();i++){
        candidate_set[i].change_coordinate(new_centre);
//        s.x -= new_centre.x;
//        s.y -= new_centre.y;
    }
    sort(candidate_set.begin(),candidate_set.end(),star_point_cmp);

    int len = min(check_set.size(),candidate_set.size());
    float loss = 0;
    int i=0,j=0;
    while(i<check_set.size() && j<candidate_set.size()){
        if(similar_position(check_set[i],candidate_set[j])){
            loss += abs(check_set[i].Distance(candidate_set[j]));
            i++;j++;
        }else if(check_set[i].Module() < candidate_set[j].Module()){
            loss += 1;
            i++;
        } else{
            loss += 1;
            j++;
        }
        cout<<loss<<endl;
    }
    cout<<loss<<endl;
    if(loss/len > 0.5) return false;
    return true;

//    if(this->__matching_star.index == this->__target_star.index) return true;
//    else return false;
}

vector<StarPoint> SkyMap::Subset(StarPoint centre, float length, float width) {
    //Some Check here.
    vector<StarPoint> stars;
    for(StarPoint point : stars_){
        if(point.InRange(centre,length,width)){
            stars.push_back(point);
        }
    }
    cout<<"Subset(length,width)=("<<length<<","<<width<<"):"<<endl;
    cout<<"Number of stars:"<<stars.size()<<endl;
    return stars;

}

void SkyMapMatching::GenerateSimImage(StarPoint centre, float length, float width) {
    //Some Check here.
    if(length<=0 || width<=0 ){
        if(!this->image_.stars_.empty()) this->image_.stars_.clear();
        this->image_.count_ = 0;
        this->image_.range_ = {length<0?0:length , width<0?0:width};
        return;
    }
    vector<StarPoint> stars = sky_.Subset(centre,length,width);
    for(int i=0;i<stars.size();i++){
        stars[i].change_coordinate(centre);
//        stars[i].x -= centre.x;
//        stars[i].y -= centre.y;
    }
    if(!this->image_.stars_.empty()) this->image_.stars_.clear();

    this->image_.range_ = {length , width};
    this->image_.stars_.insert(this->image_.stars_.end(),stars.begin(),stars.end());
    this->image_.count_ = this->image_.stars_.size();
    this->image_.centre_ = centre;

    cout<<"The Generated image is:"<<endl;
    for(int i=0;i<stars.size();i++){
        cout<<stars[i].index<<": ("<<stars[i].x<<" , "<<stars[i].y<<")"<<endl;
    }
    cout<<"The number of stars is:"<<stars.size()<<endl;
    //this->image_.RangeStandardization();
}

vector<StarPoint> SkyMap::Subset(StarPoint centre, float image_ratio, int num=0) {
    float upper_bound = 20;
    float lower_bound = 1;
    float length = (upper_bound+lower_bound)/2;
    vector<StarPoint> stars;
    float width;
    while( abs(lower_bound-upper_bound)>0.01 ){
        width = length/image_ratio;
        stars = this->Subset(centre,length,width);
        if(stars.size() == num) break;
        else if(stars.size() < num){
            stars.clear();
            lower_bound = length;
            length = (upper_bound+lower_bound)/2;
        } else{
            stars.clear();
            upper_bound = length;
            length = (upper_bound+lower_bound)/2;
        }
    }
    if(stars.size() == num) {
        cout<<"The number of star of this Subset:"<<stars.size()<<endl;
    } else{
        cout<<"Invalid subset! You can try another ratio or center."<<endl;
        stars.clear();
    }
    return stars;
}

void SkyMapMatching::GenerateSimImage(StarPoint centre, float image_ratio, int num) {
    vector<StarPoint> stars = sky_.Subset(centre,image_ratio,num);
    if(!this->image_.stars_.empty()) this->image_.stars_.clear();
    if(stars.size() == num) {
        for(int i=0;i<stars.size();i++){
            stars[i].change_coordinate(centre);
//            stars[i].x -= centre.x;
//            stars[i].y -= centre.y;
        }
        this->image_.stars_.insert(this->image_.stars_.end(),stars.begin(),stars.end());
        this->image_.count_ = this->image_.stars_.size();
        this->image_.centre_ = centre;

        cout<<"The Generated image is:"<<endl;
        for(int i=0;i<stars.size();i++){
            cout<<stars[i].index<<": ("<<stars[i].x<<" , "<<stars[i].y<<")"<<endl;
        }
        cout<<"The number of stars is:"<<stars.size()<<endl;

    } else{
        this->image_.count_ = 0;
        this->image_.centre_ = centre;
        cout<<"Invalid Subset! You can try another ratio or center."<<endl;
    }
    this->image_.RangeStandardization();
}

void Observation::RangeStandardization(){
    if(this->count_==0){
        this->range_ = {0,0};
        return;
    }else{
        float lx = 10000.0,rx = -10000.0,ly = 10000.0,ry = -10000.0;
        for(int i=0;i<stars_.size();i++){
            lx = min(stars_[i].x,lx);
            rx = max(stars_[i].x,rx);
            ly = min(stars_[i].x,ly);
            ry = max(stars_[i].x,ry);
        }
        this->range_ = {rx-lx,ry-ly};
    }
}