#include "Visualizer.h"

#include <opencv\cv.h>
#include <opencv\highgui.h>
Visualizer::Visualizer(float **d,int num_of_frames,int w,int h){
	data=d;
	nframes=num_of_frames;
	width=w;
	height=h;
	vc = new VisualizerCallback(num_of_frames,this);
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
	make_color_array();
}


osg::Vec4 Visualizer::make_new_color(){
	float r= (float)rand()/(float)RAND_MAX;
	float g= (float)rand()/(float)RAND_MAX;
	float b= (float)rand()/(float)RAND_MAX;
	return osg::Vec4(r,g,b,0.0);
}

void Visualizer::make_color_array(){
	int jc=0;
	bool getoutofjoined;
	for(int frame=0;frame<framelinks.size();frame++){
		vector<osg::Vec4> framecolor;
		vector<int> alreadyseen;
		for(int bc=0;bc<framelinks[frame].size();bc++){
			switch(conditions[frame][framelinks[frame][bc].first]){
			case(UNCHANGED):
				framecolor.push_back(colors[frame-1][framelinks[frame][bc].second]);
				break;
			case(BROKEN):
				framecolor.push_back(colors[frame-1][framelinks[frame][bc].second]);
				break;
			case(JOINED):
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
			case(FRESH):
				framecolor.push_back(make_new_color());
				break;
			}
		}
		colors.push_back(framecolor);
	}
}

void Visualizer::view(){
	//create_vertices();
	make_all_vertices();
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
	root->addChild(bgdraw());
	//root->addChild(tractordraw());
	root->addChild(drawAxes());
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

void Visualizer::update_vertices(int framenum){
	if(inds_all[framenum].size()==0)
		return;
	if(particletransform->getNumChildren()>0)
		particletransform->removeChildren(0,particletransform->getNumChildren());
	for(int i=0;i<vertices_all[framenum].size();i++){
		osg::ref_ptr<osg::Geode> geode = new osg::Geode; 
		osg::ref_ptr<osg::Geometry> geometry= new osg::Geometry;
		geometry->setVertexArray((osg::Vec3Array*)vertices_all[framenum][i]->clone(osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
		osg::Vec4Array *colorarray = new osg::Vec4Array;
		colorarray->push_back(colors[framenum][i]);
		geometry->setColorArray(colorarray);
		geometry->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE_SET);
		//geometry->setVertexArray(vertices[framenum]);
		//cout<<"Blah Blah "<<framenum<<endl;
		geometry->addPrimitiveSet((osg::DrawElementsUInt*)inds_all[framenum][i]->clone(osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
		geode->addDrawable(geometry);
		/*osg::StateSet* stateset = new osg::StateSet;
		stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);
		stateset->setAttribute(new osg::Point(3.0f),osg::StateAttribute::ON);
		geometry->setStateSet(stateset);*/
		particletransform->addChild(geode);
	}
	/*fstream fs;
	fs.open("check.txt",fstream::out);
	for(int i=0;i<inds[framenum]->size();i++)
		if(_isnan(data[framenum][inds[framenum]->at(i)])) cout<<"Yipee";
	fs.close();*/
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
			cv::Rect r=cv::minAreaRect(contours[i]).boundingRect();
			r.x=r.x<0?0:r.x;
			r.y=r.y<0?0:r.y;
			if(conditions[frame][i]==JOINED){
				int step=joinvec[jc].size();
				for(int k=0,rh=0;k<step;k++,rh+=r.height/step){
					make_vertices(rh,rh+r.height/step,i,contours,r,vdata,idata);
					framevertices.push_back(vdata);
					frameindices.push_back(idata);
				}
				jc++;
			}//JOIN CONDN
			else{
				make_vertices(0,r.height,i,contours,r,vdata,idata);
				framevertices.push_back(vdata);
				frameindices.push_back(idata);
			}
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
			vdata->push_back(osg::Vec3d(r.x+x,r.y+y,(data[i][r.x+x+(r.y+y)*width]+2)*40));
			if(!check_xy_valid(r.x+x,r.y+y))
				continue;
			float f1= data[i][r.x+x+(r.y+y)*width];
			float f2= data[i][r.x+x+(r.y+y+1)*width];
			float f3= data[i][(r.x+x+1)+(r.y+y+1)*width];
			float f4= data[i][(r.x+x+1)+y*width];
				
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
		}//height
	}//width
}

void Visualizer::show_visual_analysis_time(float t){
	static char bar[]= "<.................................................>";
	int ptage=t*100;
	if(ptage%2!=0)
		ptage++;
	printf("%s %d%%\r",&bar[50-ptage/2],ptage);
	fflush(stdout);
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
