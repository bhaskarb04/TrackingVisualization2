#include "Visualizer.h"

#include <opencv\cv.h>
#include <opencv\highgui.h>
Visualizer::Visualizer(float **d,int num_of_frames,int w,int h){
	data=d;
	nframes=num_of_frames;
	width=w;
	height=h;
	showcentresonly=true;
	cycleparticle=false;
	cyclemodel=false;
	vc = new VisualizerCallback(num_of_frames,this);
	modelon=0;
	particleon=0;
}


Visualizer::~Visualizer(void)
{
}

void Visualizer::add_contours(vector<vector<vector<cv::Point>>> lc){
	listcontours=lc;
}
void Visualizer::add_conditions(vector<vector<int>> v){
	conditions=v;
}
void Visualizer::add_joinvec(vector<vector<int>>jv){
	joinvec=jv;
}
void Visualizer::add_links(vector<vector<pair<int,int>>> fl){
	framelinks=fl;
	make_all_vertices();
	make_color_array();
}
void Visualizer::add_fcc(vector<vector<vector<int>>> fcc){
	framecontcolor=fcc;
}
void Visualizer::add_nocolors(int c){
	totalcolors=c;
}
void Visualizer::add_allpoints(vector<vector<vector<HPoint>>> ap){
	all_points=ap;
}
void Visualizer::add_minmax(double minv,double maxv){
	minval=minv;
	maxval=maxv;
}
void Visualizer::add_validity(vector<bool**> v){
	valid=v;
}
void Visualizer::add_labels(vector<cv::Mat> l){
	labels=l;
}
void Visualizer::add_centres(vector<map<int,cv::Point3d>> c){
	centres=c;
}
osg::ref_ptr<osg::LightSource> Visualizer::get_lightsource(){
	osg::ref_ptr<osg::Light> light = new osg::Light();
	osg::ref_ptr<osg::LightSource> lightsource = new osg::LightSource();
	lightsource->setLight(light);
	osg::StateSet *stateset=root->getOrCreateStateSet();
	lightsource->setStateSetModes(*stateset,osg::StateAttribute::ON);
	light->setAmbient(osg::Vec4(1.0,1.0,1.0,1.0));
	light->setDiffuse(osg::Vec4(1.0,1.0,1.0,1.0));
	return lightsource;
}
osg::Vec4 Visualizer::make_new_color(){
	float r= (float)rand()/(float)RAND_MAX;
	float g= (float)rand()/(float)RAND_MAX;
	float b= (float)rand()/(float)RAND_MAX;
	return osg::Vec4(r,g,b,0.0);
}

void Visualizer::make_color_array(){
	/*int jc=0;
	bool getoutofjoined;
	for(int frame=0;frame<framelinks.size();frame++){
		vector<osg::Vec4> framecolor;
		vector<int> alreadyseen;
		for(int bc=0;bc<framelinks[frame].size();bc++){
			switch(conditions[frame][framelinks[frame][bc].first]){
			case(NOTHING):
				framecolor.push_back(colors[frame-1][framelinks[frame][bc].second]);
				break;
			case(BREAK):
				framecolor.push_back(colors[frame-1][framelinks[frame][bc].second]);
				break;
			case(JOIN):
				getoutofjoined=false;
				for(int i=0;i<alreadyseen.size();i++){
					if(alreadyseen[i]==framelinks[frame][bc].first)
						getoutofjoined=true;
				}
				if(getoutofjoined)
					break;
				alreadyseen.push_back(framelinks[frame][bc].first);
				for(int i=0;i<joinvec[jc].size();i++)
					framecolor.push_back(colors[frame-1][joinvec[jc][i]]);
				jc++;
				break;
			case(NEW):
				framecolor.push_back(make_new_color());
				break;
			}
		}
		colors.push_back(framecolor);
	}*/
	for(int i=0;i<totalcolors;i++){
		clrs.push_back(make_new_color());
	}
	for(int frame=0;frame<framecontcolor.size();frame++){
		vector<osg::Vec4Array*> framecolors;
		for(int cont=0;cont<framecontcolor[frame].size();cont++){
			//vector<osg::Vec4f> contcolor;
			//for(int i=0;i<framecontcolor[frame][cont].size();i++)
			//	contcolor.push_back(clrs[framecontcolor[frame][cont][i]]);
			osg::Vec4Array *carray = new osg::Vec4Array;
			//Rect r=cv::boundingRect(listcontours[frame][cont]);
			//r.x=r.x<0?0:r.x;
			//r.y=r.y<0?0:r.y;
			//int step=r.height/framecontcolor[frame][cont].size();
			for(int j=0;j<vertices_all[frame][cont]->size();j++){
				//int binval=find_label((*vertices_all[frame][cont])[j].x(),(*vertices_all[frame][cont])[j].y(),frame,cont);
				//int binval=find_bin(step,(*vertices_all[frame][cont])[j].y()-r.y-1);
				int binval=labels[frame].at<float>(int((*vertices_all[frame][cont])[j].y()),int((*vertices_all[frame][cont])[j].x()));
				if(binval<0)
					carray->push_back(osg::Vec4(0,0,0,0));
				else
					carray->push_back(clrs[binval]);
			}
			framecolors.push_back(carray);
		}
		colorarray.push_back(framecolors);
		cout<<frame<<endl;
	}
}
int Visualizer::find_label(int x, int y,int frame,int cont){
	int label=-1;
	for(int i=0;i<all_points[frame][cont].size();i++){
		if(all_points[frame][cont][i].p.x==x && all_points[frame][cont][i].p.y==y){
			label=all_points[frame][cont][i].label[0];
			if(label>=totalcolors)
				label=totalcolors-1;
			break;
		}
	}
	return label;
}
void Visualizer::view(){
	//create_vertices();
	setscene();
	osgViewer::Viewer viewer;
	viewer.setSceneData( root );
	viewer.setCameraManipulator(new osgGA::TrackballManipulator);
	viewer.setUpViewInWindow(10,10,800,600);
	//viewer.getCameraManipulator()->setHomePosition(osg::Vec3(width/2,height/2,100),osg::Vec3(width/2,height/2,100),osg::Vec3(0,1,0));
	//viewer.home();
	//viewer.getCameraManipulator()->setHomePosition(osg::Vec3(
	while(!viewer.done()){
		viewer.frame();
	}
}

void Visualizer::setscene(){
	root = new osg::Group;
	transform = new osg::MatrixTransform;
	particletransform = new osg::MatrixTransform;
	make_particle_models();
	root->addChild(bgdraw());
	//root->addChild(tractordraw());
	root->addChild(get_lightsource());
	root->addChild(drawAxes());
	root->addChild(get_modelgroup());
	root->addChild(transform);
	transform->setUpdateCallback(vc);
	transform->addChild(particletransform);
	
}
osg::ref_ptr<osg::Geode> Visualizer::bgdraw()
{
	osg::ref_ptr<osg::Box> tractor= new osg::Box(osg::Vec3d(float(width)/2,float(height)/2,0),width,height,1);
	osg::ref_ptr<osg::ShapeDrawable> tractorDrawable = new osg::ShapeDrawable(tractor);
	tractorDrawable->setColor(osg::Vec4(0,1,0,0));
	osg::ref_ptr<osg::Geode> tractorGeode = new osg::Geode();
	tractorGeode->addDrawable(tractorDrawable);
	
	return(tractorGeode.release());
}

osg::ref_ptr<osg::PositionAttitudeTransform> Visualizer::tractordraw(){
	osg::Node* tractorface = osgDB::readNodeFile("../Data/7930_hood.ive");
	osg::ref_ptr<osg::PositionAttitudeTransform> tractormat = new osg::PositionAttitudeTransform;
	tractormat->setPosition(osg::Vec3(3950,-280,0));
	tractormat->setAttitude(osg::Quat(osg::DegreesToRadians(-90.0),osg::Vec3(0,1,0)));
	tractormat->addChild(tractorface);
	return (tractormat.release());
}

void Visualizer::create_vertices(){
	for(int i=0;i<nframes;i++){
		//osg::Vec3Array *vertice(new osg::Vec3Array());
		osg::Vec3Array *vdata(new osg::Vec3Array());
		osg::DrawElementsUInt* idata=new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
		
		
		for(int y=0;y<height;y++){
			for(int x=0;x<width;x++){
				vdata->push_back(convert_deere_geom(x,y,data[i][x+y*width]));
				if(x+1 >=width || y+1 >=height)
					continue;
				float f1= data[i][x+y*width];
				float f2= data[i][x+(y+1)*width];
				float f3= data[i][(x+1)+(y+1)*width];
				float f4= data[i][(x+1)+y*width];
				
				for(int j=0;j<3;j++){
					switch(j){
					case(0):
						if(_isnan(f1) || _isnan(f2) || _isnan(f3))
							break;
						//vertice->push_back(osg::Vec3(x,y,f1));
						//vertice->push_back(osg::Vec3(x,y+1,f2));
						//vertice->push_back(osg::Vec3(x+1,y+1,f3));
						idata->push_back(x+y*width);
						idata->push_back(x+(y+1)*width);
						idata->push_back(x+1+(y+1)*width);
						break;
					case(1):
						if(_isnan(f1) || _isnan(f3) || _isnan(f4))
							break;
						//vertice->push_back(osg::Vec3(x,y,f1));
						//vertice->push_back(osg::Vec3(x+1,y+1,f3));
						//vertice->push_back(osg::Vec3(x+1,y,f4));
						idata->push_back(x+y*width);
						idata->push_back(x+1+(y+1)*width);
						idata->push_back(x+1+y*width);
						break;
					case(2):
						if(_isnan(f2) || _isnan(f3) || _isnan(f4))
							break;
						//vertice->push_back(osg::Vec3(x,y+1,f2));
						//vertice->push_back(osg::Vec3(x+1,y+1,f3));
						//vertice->push_back(osg::Vec3(x+1,y,f4));
						idata->push_back(x+(y+1)*width);
						idata->push_back(x+1+(y+1)*width);
						idata->push_back(x+1+y*width);
						break;
					}//switch
				}//j
			}//height
		}//width
		//vertices.push_back(vertice);
		verts.push_back(vdata);
		inds.push_back(idata);
	}//i
}

int Visualizer::find_bin(int n,int x){
	int i=0;
	while(n*i<x)
		i++;
	i=i-1<0?1:i;
	return i-1;
}
void Visualizer::update_vertices(int framenum){
	if(inds_all[framenum].size()==0)
		return;
	if(particletransform->getNumChildren()>0)
		particletransform->removeChildren(0,particletransform->getNumChildren());
	if(!showcentresonly){
		for(int i=0;i<vertices_all[framenum].size();i++){
			osg::ref_ptr<osg::Geode> geode = new osg::Geode; 
			osg::ref_ptr<osg::Geometry> geometry= new osg::Geometry;
			geometry->setVertexArray((osg::Vec3Array*)vertices_all[framenum][i]->clone(osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
			geometry->setColorArray((osg::Vec4Array*)colorarray[framenum][i]->clone(osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
			geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
			geometry->addPrimitiveSet((osg::DrawElementsUInt*)inds_all[framenum][i]->clone(osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
			geode->addDrawable(geometry);
			particletransform->addChild(geode);
		}
	}
	osg::ref_ptr<osg::Geode> pgeode = new osg::Geode;
	pgeode=(osg::Geode*)particlemodels[particleon]->clone(osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL));
	for(map<int,cv::Point3d>::iterator it=centres[framenum].begin();it!=centres[framenum].end();it++){
		osg::PositionAttitudeTransform* pat=new osg::PositionAttitudeTransform;
		pat->addChild(pgeode);
		osg::ref_ptr<osg::Material> material = new osg::Material;
		material->setColorMode(osg::Material::DIFFUSE);
		osg::Vec4 color=clrs[labels[framenum].at<float>(int((*it).second.y),int((*it).second.x))];
		material->setAmbient (osg::Material::FRONT_AND_BACK, color);
		pgeode->getOrCreateStateSet()->setAttributeAndModes(material.get(), osg::StateAttribute::ON);
		pat->setPosition(osg::Vec3((*it).second.x,(*it).second.y,(*it).second.z));
		particletransform->addChild(pat);
	}

}

osg::Vec3 Visualizer::convert_deere_geom(float x,float y,float z){
	float xdash=(x-243)*0.54-250;
	float ydash=(y-329)*0.54+280;
	float zdash=(z-2.1)*54.0/3.0;
	return (osg::Vec3(xdash,-ydash,zdash));
}

osg::ref_ptr<osg::Geode> Visualizer::drawAxes()
{
	osg::ref_ptr<osg::Geode> return_axes=new osg::Geode;
	osg::ref_ptr<osg::Geometry> geom = new osg::Geometry;

	osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array(6);
	(*vertices)[0].set( 0.0f, 0.0f, 0.0f); //origin
	(*vertices)[1].set( 100.0f, 0.0f, 0.0f); //X
	(*vertices)[2].set( 0.0f, 0.0f, 0.0f); //origin
	(*vertices)[3].set( 0.0f, 100.0f, 0.0f); //Y
	(*vertices)[4].set( 0.0f, 0.0f, 0.0f); //origin
	(*vertices)[5].set( 0.0f, 0.0f, 100.0f); //Z

	osg::ref_ptr<osg::DrawArrays>indices =new osg::DrawArrays(osg::PrimitiveSet::LINES,0,6);
	
	osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array(6);
	(*colors)[0].set(1.0,0.0,0.0,0.0);
	(*colors)[1].set(1.0,0.0,0.0,0.0);
	(*colors)[2].set(0.0,1.0,0.0,0.0);
	(*colors)[3].set(0.0,1.0,0.0,0.0);
	(*colors)[4].set(0.0,0.0,1.0,0.0);
	(*colors)[5].set(0.0,0.0,1.0,0.0);
	
	geom->setVertexArray( vertices.get() );

	geom->setColorArray(colors.get());
	geom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

	
	geom->addPrimitiveSet( indices.get() );
	osg::StateSet* stateset = new osg::StateSet;
	stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	geom->setStateSet(stateset);

	
	return_axes->addDrawable(geom);
	return(return_axes.release());
}

bool Visualizer::check_xy_valid(int x,int y){
	if(x+1 >=width || y+1 >=height)
		return false;
	return true;
}

bool Visualizer::check_if_valid(vector<cv::Point> contour,int x1,int y1,int x2,int y2,int x3,int y3,float f1,float f2, float f3){
	if(_isnan(f1) || _isnan(f2) || _isnan(f3))
		return false;
	if( pointPolygonTest(contour,cv::Point2f(x1,y1),false)<0)
		return false;
	if( pointPolygonTest(contour,cv::Point2f(x2,y2),false)<0)
		return false;
	if( pointPolygonTest(contour,cv::Point2f(x3,y3),false)<0)
		return false;
	return true;

}
void Visualizer::make_all_vertices(){
	cout<<"Making all the vertices for visualization"<<endl;
	int jc=0;
	for(int frame=0;frame<listcontours.size();frame++){
		
		vector<vector<cv::Point>> contours = listcontours[frame];
		vector<osg::DrawElementsUInt*> frameindices;
		vector<osg::Vec3Array*> framevertices;
		for(unsigned int i=0;i<contours.size();i++){
			osg::Vec3Array *vdata;
			osg::DrawElementsUInt* idata;
			//Find the contour bounds
			cv::Rect r=cv::boundingRect(contours[i]);
			r.x=r.x<0?0:r.x;
			r.y=r.y<0?0:r.y;
			//if(conditions[frame][i]==JOINED){
			//	int step=joinvec[jc].size();
			//	for(int k=0,rh=0;k<step;k++,rh+=r.height/step){
			//		make_vertices(rh,rh+r.height/step,i,contours,r,vdata,idata);
			//		framevertices.push_back(vdata);
			//		frameindices.push_back(idata);
			//	}
			//	jc++;
			//}//JOIN CONDN
			//else{
			//	make_vertices(0,r.height,i,contours,r,vdata,idata);
			//	framevertices.push_back(vdata);
			//	frameindices.push_back(idata);
			//}
			
			make_vertices(0,r.height,i,contours,r,vdata,idata);
			framevertices.push_back(vdata);
			frameindices.push_back(idata);
			//vertices.push_back(vertice);
		}//i
		vertices_all.push_back(framevertices);
		inds_all.push_back(frameindices);
		show_visual_analysis_time(float(frame)/float(listcontours.size()));
	}//frame
	show_visual_analysis_time(1.0);
}

void Visualizer::make_vertices(int rheightstart,int rheightend,int i,vector<vector<cv::Point>> contours,cv::Rect r,osg::Vec3Array*& vdata,
								osg::DrawElementsUInt*& idata){
	vdata=new osg::Vec3Array();
	idata=new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);
	for(int y=rheightstart;y<rheightend && y<r.height;y++){
		for(int x=0;x<r.width;x++){
			double zz=(data[i][r.x+x+(r.y+y)*width]-minval)/(maxval-minval)+1;
			vdata->push_back(osg::Vec3d(r.x+x,r.y+y,zz*20));
			if(!check_xy_valid(r.x+x,r.y+y))
				continue;
			float f1= data[i][r.x+x+(r.y+y)*width];
			float f2= data[i][r.x+x+(r.y+y+1)*width];
			float f3= data[i][(r.x+x+1)+(r.y+y+1)*width];
			float f4= data[i][(r.x+x+1)+(r.y+y)*width];
				
			for(int j=0;j<3;j++){
				switch(j){
				case(0):
					if(!check_if_valid(contours[i],r.x+x,r.y+y,r.x+x,r.y+y+1,r.x+x+1,r.y+y+1,f1,f2,f3))
						break;
					//vertice->push_back(osg::Vec3(x,y,f1));
					//vertice->push_back(osg::Vec3(x,y+1,f2));
					//vertice->push_back(osg::Vec3(x+1,y+1,f3));
					idata->push_back(x+(y-rheightstart)*r.width);
					idata->push_back(x+(y-rheightstart+1)*r.width);
					idata->push_back(x+1+(y-rheightstart+1)*r.width);
					break;
				case(1):
					if(!check_if_valid(contours[i],r.x+x,r.y+y,r.x+x+1,r.y+y+1,r.x+x+1,r.y+y,f1,f3,f4))
						break;
					//vertice->push_back(osg::Vec3(x,y,f1));
					//vertice->push_back(osg::Vec3(x+1,y+1,f3));
					//vertice->push_back(osg::Vec3(x+1,y,f4));
					idata->push_back(x+(y-rheightstart)*r.width);
					idata->push_back(x+1+(y-rheightstart+1)*r.width);
					idata->push_back(x+1+(y-rheightstart)*r.width);
					break;
				case(2):
					if(!check_if_valid(contours[i],r.x+x,r.y+y+1,r.x+x+1,r.y+y+1,r.x+x+1,r.y+y,f2,f3,f4))
						break;
					//vertice->push_back(osg::Vec3(x,y+1,f2));
					//vertice->push_back(osg::Vec3(x+1,y+1,f3));
					//vertice->push_back(osg::Vec3(x+1,y,f4));
					idata->push_back(x+(y-rheightstart+1)*r.width);
					idata->push_back(x+1+(y-rheightstart+1)*r.width);
					idata->push_back(x+1+(y-rheightstart)*r.width);
					break;
				}//switch
			}//j
		}//width
	}//height
}

void Visualizer::make_vertices_2(int frameno,int cont,osg::Vec3Array*& vdata,osg::DrawElementsUInt*& idata){
	vdata=new osg::Vec3Array();
	idata=new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLES, 0);

	for(int yy=0;yy<height;yy++){
		for(int xx=0;xx<width;xx++)
			vdata->push_back(osg::Vec3d(xx,yy,0));
	}
	for(vector<HPoint>::iterator it=all_points[frameno][cont].begin();it!=all_points[frameno][cont].end();it++){
		double zz=((*it).z - minval)/(maxval - minval) + 1;
		vdata->at((*it).p.x+(*it).p.y*width).set(osg::Vec3d((*it).p.x,(*it).p.y,zz*20));
		//if(!check_xy_valid(r.x+x,r.y+y))
			//continue;
		int x=(*it).p.x;
		int y=(*it).p.y;
		/*float f1= data[frameno][x+(y)*width];
		float f2= data[frameno][x+(y+1)*width];
		float f3= data[frameno][(x+1)+(y+1)*width];
		float f4= data[frameno][(x+1)+(y)*width];*/
				
		for(int j=0;j<3;j++){
			switch(j){
			case(0):
				if(!valid[frameno][x][y] || !valid[frameno][x][y+1] || !valid[frameno][x+1][y+1])
					break;
				//vertice->push_back(osg::Vec3(x,y,f1));
				//vertice->push_back(osg::Vec3(x,y+1,f2));
				//vertice->push_back(osg::Vec3(x+1,y+1,f3));
				idata->push_back(x+(y)*width);
				idata->push_back(x+(y+1)*width);
				idata->push_back(x+1+(y+1)*width);
				break;
			case(1):
				if(!valid[frameno][x][y] || !valid[frameno][x+1][y+1] || !valid[frameno][x+1][y])
					break;
				//vertice->push_back(osg::Vec3(x,y,f1));
				//vertice->push_back(osg::Vec3(x+1,y+1,f3));
				//vertice->push_back(osg::Vec3(x+1,y,f4));
				idata->push_back(x+(y)*width);
				idata->push_back(x+1+(y+1)*width);
				idata->push_back(x+1+(y)*width);
				break;
			case(2):
				if(!valid[frameno][x][y+1] || !valid[frameno][x+1][y+1] || !valid[frameno][x+1][y])
					break;
				//vertice->push_back(osg::Vec3(x,y+1,f2));
				//vertice->push_back(osg::Vec3(x+1,y+1,f3));
				//vertice->push_back(osg::Vec3(x+1,y,f4));
				idata->push_back(x+(y)*width);
				idata->push_back(x+1+(y+1)*width);
				idata->push_back(x+1+(y)*width);
				break;
			}//switch
		}//j
	}//it
}

void Visualizer::show_visual_analysis_time(float t){
	static char bar[]= "<.................................................>";
	int ptage=t*100;
	if(ptage%2!=0)
		ptage++;
	printf("%s %d%%\r",&bar[50-ptage/2],ptage);
	fflush(stdout);
}

osg::ref_ptr<osg::Switch> Visualizer::get_modelgroup(){
	osg::ref_ptr<osg::Switch> modelswitch=new osg::Switch;
	osg::PositionAttitudeTransform *pat1=new osg::PositionAttitudeTransform;
	osg::PositionAttitudeTransform *pat2=new osg::PositionAttitudeTransform;
	osg::PositionAttitudeTransform *pat3=new osg::PositionAttitudeTransform;

	modelswitch->addChild(pat1);
	modelswitch->addChild(pat2);
	modelswitch->addChild(pat3);

	//Add first model to pat1

	//Add second model to pat2

	//Add third model to pat3


	return modelswitch;
}

void Visualizer::make_particle_models(){
	osg::Sphere* unitSphere = new osg::Sphere( osg::Vec3(0,0,0), 10.0);
	osg::ShapeDrawable* unitSphereDrawable = new osg::ShapeDrawable(unitSphere);
	osg::Geode* unitSphereGeode = new osg::Geode();
	unitSphereGeode->addDrawable(unitSphereDrawable);
	particlemodels.push_back(unitSphereGeode);
}

VisualizerCallback::VisualizerCallback(int nf,Visualizer* _parent){
	frame=0;
	nframes=nf;
	parent=_parent;
	slow=SLOW_DOWN;
}

VisualizerCallback::~VisualizerCallback(){
	
}

void VisualizerCallback::operator()(osg::Node* node, osg::NodeVisitor* nv){
	
	parent->update_vertices(frame);
	if(slow--==0){
		frame=frame+1>=nframes?0:frame+1;
		slow=SLOW_DOWN;
	}
	traverse(node, nv);
}