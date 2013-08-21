#include "Hypothesis.h"

Hypothesis::Hypothesis(vector<vector<vector<cv::Point>>>c,double m,vector<Mat> i){
	listcontours=c;
	histmax=m;
	images=i;
	nolabels=0;
}

Hypothesis::~Hypothesis(){

}


hypo Hypothesis::create_hypo(vector<cv::Point>contour,Mat histmask,int imageno){
	hypo h;
	if(contour.size()==0)
			return h;
	cv::Scalar meanval=cv::mean(Mat(contour));
	//Calculate the covariance matrix
	double covMat[2][2];
	for(int ii = 0; ii < 2; ii++){
		for (int jj = 0; jj < 2; jj++){
			covMat[ii][jj] = 0.0;
			for (int k = 0; k < contour.size(); k++){
				covMat[ii][jj]+=(meanval.val[ii] - double((ii==0)?contour[k].x:contour[k].y)) * 
								(meanval.val[jj] - double((jj==0)?contour[k].x:contour[k].y));
			}//k
			covMat[ii][jj] /= contour.size();
		}//jj
	}//ii
	double sigmaxx=covMat[0][0];
	double sigmaxy=covMat[1][0];
	double sigmayy=covMat[1][1];
	//cout<<(sigmaxx-sigmayy)*(sigmaxx-sigmayy)<<" "<<4*sigmaxy*sigmaxy<<endl;
	//if((sigmaxx-sigmayy)*(sigmaxx-sigmayy) < 4*sigmaxy*sigmaxy)
	//	continue;
	double ultaV=sqrt(abs((sigmaxx-sigmayy)*(sigmaxx-sigmayy) - 4*sigmaxy*sigmaxy));
	double lambda1=(sigmaxx+sigmayy+ultaV)/2;
	double lambda2=(sigmaxx+sigmayy-ultaV)/2;
	h.alpha = sqrt(lambda1);
	h.beta = sqrt(lambda2);
	h.theta = atan(-sigmaxy/(lambda1-sigmayy));
	h.xc=meanval.val[0];
	h.yc=meanval.val[1];
	h.age=AGE_MAX;
	h.area=cv::contourArea(contour);

	int histsize=20;
	float range[] = { 0, 9 } ;
	const float* histRange = { range };
	calcHist(&images[imageno],1,0,histmask,h.hist,1,&histsize,&histRange,true,false);
	cv::normalize(h.hist,h.hist);
	//imshow("shoe",images[imageno]);
	//waitKey();
	//imshow("shoe1",histmask);
	//waitKey();
	//for(int i=0;i<histsize;i++)
		//cout<<h.hist.at<float>(i,0)<<endl;
	h.v=cv::Point(0,0);
	return h;
}
void Hypothesis::draw_with_one(int frameno, hypo h){
	Mat mm(images[frameno].size(),CV_8UC3);
	cv::cvtColor(images[frameno],mm,CV_GRAY2RGB);
	for(int pp=0;pp<listh.size();pp++){
		if(listh[pp].age<AGE_MAX-1)
			continue;
		//Draw the ellipse
		cv::ellipse(mm,cv::Point(int(listh[pp].xc),int(listh[pp].yc)),
				cv::Size(int(2*listh[pp].alpha),int(2*listh[pp].beta)),listh[pp].theta*180/PI,0,360,CV_RGB(0,0,255));
	}
	cv::ellipse(mm,cv::Point(int(h.xc),int(h.yc)),
				cv::Size(int(2*h.alpha),int(2*h.beta)),h.theta*180/PI,0,360,CV_RGB(255,0,0));
	cv::imshow("showme",mm);
	cv::waitKey(20);
}

void Hypothesis::draw_all(int frameno,vector<hypo> hv){
	for(int i=0;i<hv.size();i++){
		draw_with_one(frameno,hv[i]);
	}
}

void Hypothesis::track(){
	cout<<"Tracking and analyzing the data"<<endl;
	for(unsigned int j=0;j<listcontours.size();j++){
		//Each frame
		vector<vector<cv::Point>> contours = listcontours[j];
		vector<hypo> framehypo;
		vector<bool>matches;
		vector<int> conditions;
		for(unsigned int i=0;i<contours.size();i++){
		//Find the contour bounds
			cv::Rect r=cv::minAreaRect(contours[i]).boundingRect();
		//Find the points which are in the contour
			vector<HPoint> points;
			for(int x=r.x;x<r.x+r.width;x++){
				for(int y=r.y;y<r.y+r.height;y++){
					if(pointPolygonTest(contours[i],cv::Point2f(x,y),false)>=0){
						HPoint hp;
						hp.p=cv::Point(x,y);
						points.push_back(hp);
					}
				}//y
			}//x
		//Check each point against all hypos
			Mat hmask=cv::Mat::zeros(images[0].size(),CV_8UC1);
			vector<vector<cv::Point>> tempc;
			tempc.push_back(contours[i]);
			cv::fillPoly(hmask,tempc,cv::Scalar(255));
			hypo chypo=create_hypo(contours[i],hmask,j);
			framehypo.push_back(chypo);
			//draw_with_one(j,chypo);
			bool mymatch=check_hypos(points,i);
			matches.push_back(mymatch);
			conditions.push_back(NOTHING);
			if(!mymatch){
				for(int k=0;k<points.size();k++)
					points[k].label.size()>0?points[k].label[0]=nolabels+1:points[k].label.push_back(nolabels+1);
			}
		}//i
		reduce_age();
		framecondition.push_back(conditions);
		check_blob_count(contours.size(),framehypo);
		for(int i=0;i<framehypo.size();i++)
			update_hypo(matches[i],framehypo[i],i);
		framelinks.push_back(links);
		links.clear();
		listcontours[j].clear();
		show_analysis_time(float(j)/float(listcontours.size()));
	}//j
	show_analysis_time(1.0);
	cout<<endl;
}

bool Hypothesis::near_the_edge(hypo h){
	bool nearedge=false;
	int height=images[0].rows;
	int width=images[0].cols;
	double d1=abs(h.xc-0);
	double d2=abs(h.yc-height);
	double d3=abs(h.xc-width);
	double d4=abs(h.yc-0);
	if(d1 < D_MAX || d2 < D_MAX || d3 < D_MAX || d4 < D_MAX)
		nearedge=true;
	return nearedge;
}
void Hypothesis::reduce_age(){
	for(int i=0;i<listh.size();i++){
		if(listh[i].age < AGE_MAX && near_the_edge(listh[i])){
			for(int j=0;j<links.size();j++){
				if(links[j].second > i)
					links[j].second--;
			}
			listh.erase(listh.begin()+i);
			i--;
			continue;
		}
		listh[i].age--;
	}
}
void Hypothesis::show_analysis_time(float t){
	static char bar[]= "<.................................................>";
	int ptage=t*100;
	if(ptage%2!=0)
		ptage++;
	printf("%s %d%%\r",&bar[50-ptage/2],ptage);
	fflush(stdout);
}
bool Hypothesis::check_hypos(vector<HPoint>& points,int blobno){
	bool match=false;
	vector<int> linkt;
	//first round to see if blob overlaps any hypo
	for(unsigned int i=0; i<points.size();i++){
		!match?match=check_hypo(points[i],false):check_hypo(points[i],false);
	}
	if(!match)
		return match;
	//second round to label outliers in case inliers overlap/exist
	map<int,int> maplist;
	for(unsigned int i=0; i<points.size();i++){
		check_hypo(points[i],true);

		for(int k=0;k<points[i].label.size();k++){
			/*bool there=false;
			for(int j=0;j<linkt.size();j++){
				if(points[i].label[k]==linkt[j])
					there=true;
			}
			if(!there)
				linkt.push_back(points[i].label[k]);*/
			if(maplist[points[i].label[k]]==NULL)
				maplist[points[i].label[k]]=1;
			else
				maplist[points[i].label[k]]++;
		}
	}
	int maxval=0,maxno=0;
	for(map<int,int>::iterator it=maplist.begin();it!=maplist.end();it++){
		if(maxval<it->second){
			maxval=it->second;
			maxno=it->first;
		}
	}
	for(map<int,int>::iterator it=maplist.begin();it!=maplist.end();it++){
		if(it->first==maxno)
			continue;
		if((double)it->second/(double)maxval > 0.5){
			pair<int,int> link(blobno,it->first);
			links.push_back(link);
		}
	}
	//for(int i=0;i<linkt.size();i++){
		pair<int,int> link(blobno,maxno);
		links.push_back(link);
	//}
	return match;
}
double Hypothesis::getD(HPoint point,int i){
	double a=(point.p.x-listh[i].xc-listh[i].v.x)/listh[i].alpha;
	double b=(point.p.y-listh[i].yc-listh[i].v.y)/listh[i].beta;
	double p1=a*cos(listh[i].theta)-b*sin(listh[i].theta);
	double p2=a*sin(listh[i].theta)+b*cos(listh[i].theta);
	double d1=sqrt(p1*p1+p2*p2);
	return d1;
}
bool Hypothesis::check_hypo(HPoint& point,bool recheck){
	bool retval=false;
	double d=DBL_MAX;
	int ltemp=-1;
	if(point.label.size()>0)
		return retval;
	for(unsigned int i=0;i<listh.size();i++){
		if(listh[i].age<=0)
			continue;
		double d1=getD(point,i);
		if(!recheck){
			if(d1<=1.0){
				point.label.push_back(i);
				retval=true; 
			}
		}
		else{
			if(d>d1){
				ltemp=i;
				d=d1;
			}
		}
	}
	if(recheck)
		point.label.push_back(ltemp);
	//cout<<d<<" "; 
	
	return retval;
}

void Hypothesis::update_hypo(bool match,hypo h,int blobno){
	if(!match){
		listh.push_back(h);
		pair<int,int> link(blobno,nolabels);
		links.push_back(link);
		framecondition[framecondition.size()-1][blobno]=NEW;
		nolabels++;
	}
	else{
		double d=DBL_MAX;
		int closest=-1;
		for(int i=0;i<listh.size();i++){
			double d1=compare_ellipses(listh[i],h);
			if(d>d1){
				d=d1;
				closest=i;
			}
		}
		//cout<<"closest: "<<closest<<endl;
		//if(h.area/listh[closest].area < AREA_THRESH){
			h.v=cv::Point(h.xc-listh[closest].xc,h.yc-listh[closest].yc);
			listh[closest]=h;
		//}
	}
}

double Hypothesis::compare_ellipses(hypo h1,hypo h2){
	//double d;
	//double histc=cv::compareHist(h1.hist,h2.hist, CV_COMP_BHATTACHARYYA);
	Mat m1(h1.hist.rows+5,1,CV_32FC1); Mat m2(h2.hist.rows+5,1,CV_32FC1);
	int i;
	for(i=0;i<h1.hist.rows;i++){
		m1.at<float>(i,0)=h1.hist.at<float>(i,0);
		m2.at<float>(i,0)=h2.hist.at<float>(i,0);
	}
	m1.at<float>(i,0)=h1.alpha; m2.at<float>(i,0)=h2.alpha;
	m1.at<float>(i+1,0)=h1.beta;  m2.at<float>(i+1,0)=h2.beta;
	m1.at<float>(i+2,0)=h1.theta; m2.at<float>(i+2,0)=h2.theta;
	m1.at<float>(i+3,0)=h1.xc;    m2.at<float>(i+3,0)=h2.xc;
	m1.at<float>(i+4,0)=h1.yc;    m2.at<float>(i+4,0)=h2.yc;

	double retval=0;
	for(int i=0;i<h1.hist.rows+5;i++){
		retval+=(m1.at<float>(i,0)-m2.at<float>(i,0))*(m1.at<float>(i,0)-m2.at<float>(i,0));
	}
	//double ellc=1/cv::norm(m1,m2,NORM_L2);

	//d=(histc+ellc)/sqrt(histc*histc+ellc*ellc);

	//double histc=cv::compareHist(m1,m2, CV_COMP_BHATTACHARYYA);
	return retval;
}
void Hypothesis::check_blob_count(int noblobs,vector<hypo>& framehypo){
	if(links.size()==0 || links.size()==1)
		return;
	hypos_broke(framehypo);
	hypos_join(framehypo);
	//assert(noblobs==listh.size());
}
void Hypothesis::hypos_broke(vector<hypo>& framehypo){
	vector<int>remove;
	vector<int>place;
	for(int i=0;i<links.size()-1;i++){
		for(int j=i+1;j<links.size();j++){
			if(links[i].second==links[j].second){
				remove.push_back(links[i].second);
				place.push_back(links[i].first);
				place.push_back(links[j].first);
			}
		}
	}
	if(remove.empty())
		return;
	cleanupandhousekeeping(framehypo,remove,place,BREAK);
}
void Hypothesis::hypos_join(vector<hypo>& framehypo){
	vector<int>remove;
	vector<int>place;
	for(int i=0;i<links.size()-1;i++){
		for(int j=i+1;j<links.size();j++){
			if(links[i].first==links[j].first){
				remove.push_back(links[i].second);
				remove.push_back(links[j].second);
				place.push_back(links[i].first);
			}
		}
	}
	if(remove.empty())
		return;
	cleanupandhousekeeping(framehypo,remove,place,JOIN);
}

void Hypothesis::cleanupandhousekeeping(vector<hypo>& framehypo,vector<int>remove,vector<int>place,condition c){
	sort(remove.begin(),remove.end());
	remove.erase( unique( remove.begin(), remove.end() ), remove.end());
	sort(place.begin(),place.end());
	place.erase( unique( place.begin(), place.end() ), place.end());
	if(c==JOIN){
		vector<int>myvec;
		for(int i=0;i<remove.size();i++){
			for(int j=0;j<framelinks[framelinks.size()-1].size();j++){
				if(framelinks[framelinks.size()-1][j].second==remove[i]){
					myvec.push_back(framelinks[framelinks.size()-1][j].first);
				}
			}
		}
		joinvec.push_back(myvec);
	}
	vector<hypo> listhtemp;
	for(int i=0;i<listh.size();i++){
		bool there=false;
		for(int j=0;j<remove.size();j++){
			if(i==remove[j])
				there=true;
		}
		if(!there)
			listhtemp.push_back(listh[i]);
	}
	listh.clear();
	listh.assign(listhtemp.begin(),listhtemp.end());
	listhtemp.clear();
	for(int i=0;i<place.size();i++)
		listh.push_back(framehypo[place[i]]);
	
	vector<hypo> tempframehypo;
	for(int i=0;i<framehypo.size();i++){
		bool there=false;
		for(int j=0;j<place.size();j++){
			if(i==place[j]){
				there=true;
				framecondition[framecondition.size()-1][i]=c;
			}
		}
		if(!there)
			tempframehypo.push_back(framehypo[i]);
	}
	framehypo.clear();
	framehypo.assign(tempframehypo.begin(),tempframehypo.end());
	tempframehypo.clear();
	nolabels+=(place.size()-remove.size());
}