#include "Hypothesis.h"

Hypothesis::Hypothesis(vector<vector<vector<cv::Point>>>c,double m,vector<Mat> i){
	listcontours=c;
	histmax=m;
	images=i;
	nolabels=0;
	maxnolabels=0;
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
	h.zc=images[imageno].at<float>(int(h.yc),int(h.xc));
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
		vector<vector<HPoint>>framepoints;
		labels.push_back(cv::Mat::ones(images[0].rows,images[0].cols,CV_32FC1)*-1);
		for(unsigned int i=0;i<contours.size();i++){
		//Find the contour bounds
			cv::Rect r=cv::boundingRect(contours[i]);
		//Find the points which are in the contour
			vector<HPoint> points;
			bool** valid1=new bool*[images[0].cols];
			for(int k=0;k<images[0].cols;k++){
				valid1[k]=new bool[images[0].rows];
				for(int k2=0;k2<images[0].rows;k2++)
					valid1[k][k2]=false;
			}

			for(int x=r.x;x<r.x+r.width;x++){
				for(int y=r.y;y<r.y+r.height;y++){
					if(pointPolygonTest(contours[i],cv::Point2f(x,y),false)>=0){
						HPoint hp;
						hp.p=cv::Point(x,y);
						hp.z=images[j].at<float>(x,y);
						points.push_back(hp);
						if(!_isnan(double(hp.z)))
							valid1[x][y]=true;
					}
				}//y
			}//x
			valid.push_back(valid1);
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
				nolabels++;
			}
			framepoints.push_back(points);
			nodelist.push_back(pair<int,int>(j,i));
		}//i
		all_hypo.push_back(framehypo);
		reduce_age();
		framecondition.push_back(conditions);
		check_blob_count(contours.size(),framehypo);
		//for(int i=0;i<framehypo.size();i++)
			//update_hypo(matches[i],framehypo[i],i);
		//cout<<j<<endl;
		all_points.push_back(framepoints);
		update_all(matches,framehypo);
		all_color.push_back(colorh);
		maxnolabels=maxnolabels<nolabels?nolabels:maxnolabels;
		framelinks.push_back(links);
		links.clear();
		listcontours[j].clear();
		show_analysis_time(float(j)/float(listcontours.size()));
	}//j
	show_analysis_time(1.0);
	//make_adjlist();
	make_colors();
	cout<<endl<<"Number of particles detected: "<<maxnolabels<<endl;
	cout<<endl;
}


void Hypothesis::check_localpos(){

}
void Hypothesis::make_colors(){
	framecontcolor.resize(images.size());
	for(int i=0;i<all_hypo.size();i++)
		framecontcolor[i].resize(all_hypo[i].size());
	int framemax;
	maxnolabels=0;
	for(int i=0;i<all_hypo.size();i++){
		if(all_hypo[i].size()>maxnolabels){
			framemax=i;
			maxnolabels=all_hypo[i].size();
		}
		//maxnolabels=maxnolabels<all_hypo[i].size()?all_hypo[i].size():maxnolabels;
	}
	for(int i=0;i<framelinks.size();i++){
		for(int j=0;j<framelinks[i].size();j++){
			framecontcolor[i][framelinks[i][j].first].push_back(framelinks[i][j].second);
			//maxnolabels=maxnolabels<framelinks[i][j].second?framelinks[i][j].second:maxnolabels;
		}
	}
	//maxnolabels++;
	//check for breaks
	for(int i=framemax;i<framemax+1;i++){
		listh.clear();
		colorh.clear();
		bool broke=false;
		for(int j=0;j<framecondition[i].size();j++){
			if(framecondition[i][j]==BREAK){
				int start=framelinks[i+1][j].second;
				for(int k=i;k>=0;k--){
					if(k==i)
						framecontcolor[k][start].clear();
					framecontcolor[k][start].push_back(framelinks[i+1][j].second);
					start=framelinks[k][start].second;
					broke=true;
				}
			}
			listh.push_back(all_hypo[i][j]);
			listh.back().v=cv::Point(0,0);
			colorh.push_back(all_color[i][j]);
		}
		if(broke){
			for(int k1=i;k1>=0;k1--){
				for(unsigned int k2=0; k2<all_points[k1].size();k2++){
					for(unsigned int k3=0;k3<all_points[k1][k2].size();k3++){
						all_points[k1][k2][k3].label.clear();
						check_hypo(all_points[k1][k2][k3],true);
					}
				}
				for(int j=0;j<listh.size();j++){
					cv::Point np(0,0);
					int npc=0;
					//draw_with_one(k1,listh[j]);
					//cv::waitKey();
					for(int k=0;k<all_points[k1].size();k++){
						for(int c=0;c<all_points[k1][k].size();c++){
							for(int kk=0;kk<all_points[k1][k][c].label.size();kk++){
								if(j==all_points[k1][k][c].label[kk]){
									np.x+=all_points[k1][k][c].p.x;
									np.y+=all_points[k1][k][c].p.y;
									npc++;
								}
							}//kk
						}//c
					}//k
					np.x=(double)np.x/(double)npc;
					np.y=(double)np.y/(double)npc;
					double d=DBL_MAX;
					int closest=-1;
					for(int k=0;k<listh.size();k++){
						if(listh[k].age<0)
							continue;
						double d1=(listh[k].xc-np.x)*(listh[k].xc-np.x)+(listh[k].yc-np.y)*(listh[k].yc-np.y);
						if(d1<d){
							d=d1;
							closest=k;
						}
					}
					cout<<k1<<endl;
					cv::Point v;
					v=cv::Point(np.x-listh[closest].xc,np.y-listh[closest].yc);
					listh[closest].xc=np.x;
					listh[closest].yc=np.y;
					listh[closest].v=v;
					listh[closest].age=AGE_MAX;
				}
			}
		}
	}
	//for(int cont=0;cont<all_hypo[framemax].size();cont++){
	//	bool done=false;
	//	int n,orign;
	//	find_in_nodelist(framemax,cont,n);
	//	int origstart,start=find_starting_point(n);
	//	orign=n;origstart=start;
	//	//before the max
	//	while(!done){
	//		int f=nodelist[n].first;
	//		int c=nodelist[n].second;
	//		framecontcolor[f][c].push_back(cont);
	//		//conjecture there cannot be a repeat which is not accessed
	//		check_for_repeat(start,n,true);
	//		done=!find_prev(start,n);
	//		if(done && repeat.size()>0){
	//			start=repeat[0].first;
	//			n=repeat[0].second;
	//			repeat.erase(repeat.begin()+0);
	//			done=false;
	//		}
	//	}
	//	//after the max
	//	n=adjlist[origstart].second;
	//	start=origstart;
	//	done=false;
	//	while(!done){
	//		int f=nodelist[n].first;
	//		int c=nodelist[n].second;
	//		framecontcolor[f][c].push_back(cont);
	//		check_for_repeat(start,n,false);
	//		done=!find_next(start,n);
	//		if(done && repeat.size()>0){
	//			start=repeat[0].first;
	//			n=repeat[0].second;
	//			repeat.erase(repeat.begin()+0);
	//			done=false;
	//		}
	//	}
	//}
	//remove any repetitions
	for(int i=0;i<framecontcolor.size();i++){
		for(int j=0;j<framecontcolor[i].size();j++){
			sort(framecontcolor[i][j].begin(),framecontcolor[i][j].end());
			framecontcolor[i][j].erase( unique( framecontcolor[i][j].begin(), framecontcolor[i][j].end() ), framecontcolor[i][j].end());
		}
	}
	for(int f=0;f<all_points.size();f++){
		map<int,cv::Point3d>ccentres;
		map<int,int>ccount;
		for(int i=0;i<all_points[f].size();i++){
			for(int j=0;j<all_points[f][i].size();j++){
				if(all_points[f][i][j].label.size()>0){
					if(all_points[f][i][j].label[0]>=maxnolabels)
						all_points[f][i][j].label[0]=maxnolabels-1;
					labels[f].at<float>(all_points[f][i][j].p.y,all_points[f][i][j].p.x)=all_points[f][i][j].label[0];
					if(ccentres.find(all_points[f][i][j].label[0])==ccentres.end()){
						if(_isnan(all_points[f][i][j].z))
							ccentres[all_points[f][i][j].label[0]]=cv::Point3d(all_points[f][i][j].p.x,all_points[f][i][j].p.y,0.0);
						else
							ccentres[all_points[f][i][j].label[0]]=cv::Point3d(all_points[f][i][j].p.x,all_points[f][i][j].p.y,all_points[f][i][j].z);
						ccount[all_points[f][i][j].label[0]]=1;
					}
					else{
						if(!_isnan(all_points[f][i][j].z)){
							//ccentres[all_points[f][i][j].label[0]]+=cv::Point3d(all_points[f][i][j].p.x,all_points[f][i][j].p.y,0.0);
						//else
							ccentres[all_points[f][i][j].label[0]]+=cv::Point3d(all_points[f][i][j].p.x,all_points[f][i][j].p.y,all_points[f][i][j].z);
							ccount[all_points[f][i][j].label[0]]++;
						}
					}
				}
			}
		}
		for(map<int,int>::iterator it=ccount.begin();it!=ccount.end();it++){
			ccentres[(*it).first].x/=(*it).second;
			ccentres[(*it).first].y/=(*it).second;
			ccentres[(*it).first].z/=(*it).second;
		}
		centres.push_back(ccentres);
	}
	//check_localpos();
}

void Hypothesis::check_for_repeat(int start,int n,bool prev){
	if(start==0 || start==adjlist.size()-1)
		return;
	if(prev){
		if(adjlist[start-1].first==n)
			repeat.push_back(pair<int,int>(start-1,n));
	}
	else{
		if(adjlist[start+1].first==n)
			repeat.push_back(pair<int,int>(start+1,n));
	}
}
bool Hypothesis::find_prev(int& start,int& n){
	for(int i=start-1;i>=0;i--){
		if(adjlist[i].second==n){
			start=i;
			n=adjlist[i].first;
			return true;
		}
	}
	return false;
}
bool Hypothesis::find_next(int& start,int& n){
	for(int i=start+1;i<adjlist.size();i++){
		if(adjlist[i].first==n){
			start=i;
			n=adjlist[i].second;
			return true;
		}
	}
	return false;
}

int Hypothesis::find_starting_point(int n){
	for(int i=0;i<adjlist.size();i++){
		if(adjlist[i].first==n)
			return i;
	}
	return -1;
}
bool Hypothesis::find_in_nodelist(int frameno,int withinframe, int&n){
	for(int i=0;i<nodelist.size();i++){
		pair<int,int>mypair=pair<int,int>(frameno,withinframe);
		if(nodelist[i]==mypair){
			n=i;
			return true;
		}
	}
	return false;
}

void Hypothesis::make_adjlist(){
	int jc=0;
	for(int frameno=1;frameno<framelinks.size();frameno++){
		//cout<<frameno<<endl;
		for(int curr=0;curr<framelinks[frameno].size();curr++){
			int prev=framelinks[frameno][curr].second;
			/*for(prev=0;prev<framelinks[frameno-1].size();prev++){
				if(framelinks[frameno][curr].second==framelinks[frameno-1][prev].second)
					break;
			}*/
			
			int n1=0,n2=0;
			if(curr < framecondition[frameno].size() && framecondition[frameno][curr]==JOIN){
				for(int i=0;i<joinvec[jc].size();i++){
					if(find_in_nodelist(frameno,curr,n1) && find_in_nodelist(frameno-1,framelinks[frameno-1][joinvec[jc][i]].first,n2))
						adjlist.push_back(pair<int,int>(n2,n1));
				}
				jc++;
				continue;
			}
			if(find_in_nodelist(frameno,curr,n1) && find_in_nodelist(frameno-1,framelinks[frameno-1][prev].first,n2))
				adjlist.push_back(pair<int,int>(n2,n1));
		}//curr
	}//frameno
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
	vector<int>gone;
	for(int i=0;i<listh.size();i++){
		/*if(listh[i].age < AGE_MAX && near_the_edge(listh[i])){
			for(int j=0;j<links.size();j++){
				if(links[j].second > i)
					links[j].second--;
			}
			listh.erase(listh.begin()+i);
			gone.push_back(i);
			i--;
			continue;
		}*/
		listh[i].age--;
	}
	gonelist.push_back(gone);
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
				point.label.push_back(colorh[i]);
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
		point.label.push_back(colorh[ltemp]);
	//cout<<d<<" "; 
	
	return retval;
}

void Hypothesis::update_hypo(bool match,hypo h,int blobno){
	if(!match){
		listh.push_back(h);
		pair<int,int> link(blobno,nolabels);
		////pair<int,int> blobcon(-1,-1);
		////bloblinks.push_back(blobcon);
		links.push_back(link);
		framecondition[framecondition.size()-1][blobno]=NEW;
		//nolabels++;
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
		/*int frameno=framecondition.size()-1;
		int curr=blobno;
		int n1=0,n2=0;
		int jc=joinvec.size()-1;
		if(curr < framecondition[frameno].size() && framecondition[frameno][curr]==JOIN){
			for(int i=0;i<joinvec[jc].size();i++){
				if(find_in_nodelist(frameno,curr,n1) && find_in_nodelist(frameno-1,framelinks[frameno-1][joinvec[jc][i]].first,n2))
					adjlist.push_back(pair<int,int>(n2,n1));
			}
			return;
		}
		if(find_in_nodelist(frameno,curr,n1) && find_in_nodelist(frameno-1,framelinks[frameno-1][prev].first,n2))
			adjlist.push_back(pair<int,int>(n2,n1));*/
	}
}
void Hypothesis::update_all(vector<bool>matches,vector<hypo>framehypo){
	for(int i=0;i<matches.size();i++){
		hypo h=framehypo[i];
		if(!matches[i]){
			int n=1;
			for(int j=i+1;j<matches.size();j++)
				matches[j]?n++:n;
			listh.push_back(h);
			colorh.push_back(nolabels-n);
			pair<int,int> link(i,nolabels-n);
			links.push_back(link);
			framecondition[framecondition.size()-1][i]=NEW;
		}
		else{
			if(framecondition[framecondition.size()-1][i]==JOIN){
				vector<int>framejoin=joinvec.back();
				vector<vector<HPoint>>framepoints = all_points[all_points.size()-1];
				for(int j=0;j<framejoin.size();j++){
					cv::Point np(0,0);
					int npc=0;
					for(int k=0;k<framepoints.size();k++){
						for(int c=0;c<framepoints[k].size();c++){
							for(int kk=0;kk<framepoints[k][c].label.size();kk++){
								if(framejoin[j]==framepoints[k][c].label[kk]){
									np.x+=framepoints[k][c].p.x;
									np.y+=framepoints[k][c].p.y;
									npc++;
								}
							}//kk
						}//c
					}//k
					np.x=(double)np.x/(double)npc;
					np.y=(double)np.y/(double)npc;
					double d=DBL_MAX;
					int closest=-1;
					for(int k=0;k<listh.size();k++){
						if(listh[k].age<0)
							continue;
						double d1=(listh[k].xc-np.x)*(listh[k].xc-np.x)+(listh[k].yc-np.y)*(listh[k].yc-np.y);
						if(d1<d){
							d=d1;
							closest=k;
						}
					}
					cv::Point v;
					v=cv::Point(np.x-listh[closest].xc,np.y-listh[closest].yc);
					listh[closest].xc=np.x;
					listh[closest].yc=np.y;
					listh[closest].v=v;
					listh[closest].age=AGE_MAX;
				}//j
				//joinvec.pop_front();
			}//if JOIN
			else{
				double d=DBL_MAX;
				int closest=-1;
				for(int i=0;i<listh.size();i++){
					if(listh[i].age<0)
							continue;
					double d1=compare_ellipses(listh[i],h);
					if(d>d1){
						d=d1;
						closest=i;
					}
				}
				h.v=cv::Point(h.xc-listh[closest].xc,h.yc-listh[closest].yc);
				listh[closest]=h;
			}//second else
		}//first else
	}//i
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
	modify_points(remove,place,BREAK);
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
	modify_points(remove,place,JOIN);
}

void Hypothesis::cleanupandhousekeeping(vector<hypo>& framehypo,vector<int>remove,vector<int>place,condition c){
	sort(remove.begin(),remove.end());
	remove.erase( unique( remove.begin(), remove.end() ), remove.end());
	sort(place.begin(),place.end());
	place.erase( unique( place.begin(), place.end() ), place.end());
	if(c==JOIN){
		/*vector<int>myvec;
		for(int i=0;i<remove.size();i++){
			for(int j=0;j<framelinks[framelinks.size()-1].size();j++){
				if(framelinks[framelinks.size()-1][j].second==remove[i]){
					myvec.push_back(framelinks[framelinks.size()-1][j].first);
				}
			}
		}
		joinvec.push_back(myvec);*/
		joinvec.push_back(remove);
	}
	if(c==BREAK){
		/*vector<hypo> listhtemp;
		vector<int> colortemp;
		for(int i=0;i<listh.size();i++){
			bool there=false;
			for(int j=0;j<remove.size();j++){
				if(i==remove[j])
					there=true;
			}
			if(!there){
				listhtemp.push_back(listh[i]);
				colortemp.push_back(colorh[i]);
			}
		}
		listh.clear();
		colorh.clear();
		listh.assign(listhtemp.begin(),listhtemp.end());
		colorh.assign(colortemp.begin(),colortemp.end());
		listhtemp.clear();
		colortemp.clear();
		for(int i=0;i<place.size();i++)
			listh.push_back(framehypo[place[i]]);
		int range=colorh.size()+place.size();
		for(int i=0;i<range;i++){
			bool there=false;
			for(int j=0;j<colorh.size();j++){
				if(i==colorh[j])
					there=true;
			}
			if(!there)
				colorh.push_back(i);
		}*/
		listh.clear();
		colorh.clear();
		for(int i=0;i<framehypo.size();i++)
			colorh.push_back(i);
		listh.assign(framehypo.begin(),framehypo.end());
	}
	vector<hypo> tempframehypo;
	for(int i=0;i<framehypo.size();i++){
		bool there=false;
		for(int j=0;j<place.size();j++){
			if(i==place[j]){
				there=true;
				framecondition[framecondition.size()-1][i]=c;
			}
		}
		//if(c==BREAK && !there)
		//	tempframehypo.push_back(framehypo[i]);
	}
	/*if(c==BREAK){
		framehypo.clear();
		framehypo.assign(tempframehypo.begin(),tempframehypo.end());
	}*/
	tempframehypo.clear();
	nolabels+=(place.size()-remove.size());
}

void Hypothesis::modify_points(vector<int>remove,vector<int>place,condition c){
	if(c==BREAK){
		modify_points_break(remove,place);
	}
}
void Hypothesis::modify_points_break(vector<int>remove,vector<int>place){
	
}