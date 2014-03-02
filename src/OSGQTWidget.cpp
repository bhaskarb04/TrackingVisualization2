#include "OSGQTWidget.h"

ViewerWidget::ViewerWidget(QWidget* parent) : QWidget(parent)
{
	setThreadingModel(osgViewer::CompositeViewer::SingleThreaded);
	grid = new QGridLayout;
	root = new osg::Group;
	datapat= new osg::PositionAttitudeTransform;
	modelpat= new osg::PositionAttitudeTransform;
	osg::PositionAttitudeTransform *pat = new osg::PositionAttitudeTransform;
	//pat->setAttitude(osg::Quat(osg::DegreesToRadians(-90.0),osg::Vec3(0,0,1))*osg::Quat(osg::DegreesToRadians(-90.0),osg::Vec3(0,1,0)));
	//pat->addChild(osgDB::readNodeFile("./Shubang/3ds/All.3ds"));
	modelpat->addChild(pat);
	root->addChild(datapat);
	root->addChild(modelpat);
	QWidget* widget = addViewWidget( createGraphicsWindow(0,0,100,100), root);
	grid->addWidget( widget, 0, 0 );
	setLayout( grid );
	home=true;
}

void ViewerWidget::draw_data(osg::Node* data)
{
	if(home){
		modelpat->removeChildren(0,modelpat->getNumChildren());
		home=false;
		connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
		_timer.start( TIMER_ );
	}
	datapat->addChild(data);
}
void ViewerWidget::delete_data()
{
	//_timer.stop();
	//cv::waitKey(50);
	datapat->removeChildren(0,datapat->getNumChildren());
	//datapat->setPosition(osg::Vec3d(0,0,0));
	//datapat->setAttitude(osg::Quat(0,0,0,1));
}
void ViewerWidget::draw_model(string modelfileName)
{
	if(home){
		modelpat->removeChildren(0,modelpat->getNumChildren());
		home=false;
		connect( &_timer, SIGNAL(timeout()), this, SLOT(update()) );
		_timer.start( TIMER_ );
	}
	osg::ref_ptr<osg::Node> tractorface = osgDB::readNodeFile(modelfileName);
	modelpat->setPosition(osg::Vec3(3950,-280,0));
	modelpat->setAttitude(osg::Quat(osg::DegreesToRadians(-90.0),osg::Vec3(0,1,0)));
	osg::ref_ptr<osg::Switch> myswitch = new osg::Switch;
	myswitch->addChild(tractorface);
	modelpat->addChild(myswitch);
}
void ViewerWidget::delete_model(){
	modelpat->removeChildren(0,modelpat->getNumChildren());
}

void ViewerWidget::switch_model(bool on){
	if(on)
		modelpat->getChild(0)->asSwitch()->setAllChildrenOn();
	else
		modelpat->getChild(0)->asSwitch()->setAllChildrenOff();
}

//QWidget* ViewerWidget::addViewWidget( osg::Camera* camera, osg::Node* scene )
//{
//	view = new osgViewer::View;
//	view->setCamera( camera );
//	addView( view );
//        
//	view->setSceneData( scene );
//	view->addEventHandler( new osgViewer::StatsHandler );
//	view->setCameraManipulator( new osgGA::TrackballManipulator );
//        
//	osgQt::GraphicsWindowQt* gw = dynamic_cast<osgQt::GraphicsWindowQt*>( camera->getGraphicsContext() );
//	return gw ? gw->getGLWidget() : NULL;
//}
//osg::Camera* ViewerWidget::createCamera( int x, int y, int w, int h, const std::string& name, bool windowDecoration )
//{
//	osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
//	osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
//	traits->windowName = name;
//	traits->windowDecoration = windowDecoration;
//	traits->x = x;
//	traits->y = y;
//	traits->width = w;
//	traits->height = h;
//	traits->doubleBuffer = true;
//	traits->alpha = ds->getMinimumNumAlphaBits();
//	traits->stencil = ds->getMinimumNumStencilBits();
//	traits->sampleBuffers = ds->getMultiSamples();
//	traits->samples = ds->getNumMultiSamples();
//        
//	osg::ref_ptr<osg::Camera> camera = new osg::Camera;
//	camera->setGraphicsContext( new osgQt::GraphicsWindowQt(traits.get()) );
//        
//	camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
//	camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
//	//camera->setProjectionMatrixAsPerspective(
//	//	30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );
//	//camera->setViewMatrixAsLookAt(osg::Vec3d(-10000,5000,100000),osg::Vec3d(0,0,0),osg::Vec3d(0,-1,0));
//	return camera.release();
//}

osgQt::GraphicsWindowQt* ViewerWidget::createGraphicsWindow( int x, int y, int w, int h){
    osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = x;
    traits->y = y;
    traits->width = w;
    traits->height = h;
    traits->doubleBuffer = true;
    traits->alpha = ds->getMinimumNumAlphaBits();
    traits->stencil = ds->getMinimumNumStencilBits();
    traits->sampleBuffers = ds->getMultiSamples();
    traits->samples = ds->getNumMultiSamples();

    return new osgQt::GraphicsWindowQt(traits.get());
}

QWidget* ViewerWidget::addViewWidget( osgQt::GraphicsWindowQt* gw, osg::Node* scene ){
    osgViewer::View* view = new osgViewer::View;
    addView( view );

    osg::Camera* camera = view->getCamera();
    camera->setGraphicsContext( gw );

    const osg::GraphicsContext::Traits* traits = gw->getTraits();

    camera->setClearColor( osg::Vec4(0.2, 0.2, 0.6, 1.0) );
    camera->setViewport( new osg::Viewport(0, 0, traits->width, traits->height) );
    camera->setProjectionMatrixAsPerspective(30.0f, static_cast<double>(traits->width)/static_cast<double>(traits->height), 1.0f, 10000.0f );

    view->setSceneData( scene );
    view->addEventHandler( new osgViewer::StatsHandler );
    view->setCameraManipulator( new osgGA::TrackballManipulator );

    return gw->getGLWidget();
}

void ViewerWidget::translate_model_x(float val){
	osg::Vec3d prev=modelpat->getPosition();
	prev.set(val,prev.y(),prev.z());
	modelpat->setPosition(prev);
}

void ViewerWidget::translate_model_y(float val){
	osg::Vec3d prev=modelpat->getPosition();
	prev.set(prev.x(),val,prev.z());
	modelpat->setPosition(prev);
}

void ViewerWidget::translate_model_z(float val){
	osg::Vec3d prev=modelpat->getPosition();
	prev.set(prev.x(),prev.y(),val);
	modelpat->setPosition(prev);
}

void ViewerWidget::rotate_model_x(float val){
	modelpat->setAttitude(modelpat->getAttitude()*osg::Quat(osg::DegreesToRadians(val),osg::Vec3(1,0,0)));
}

void ViewerWidget::rotate_model_y(float val){
	modelpat->setAttitude(modelpat->getAttitude()*osg::Quat(osg::DegreesToRadians(val),osg::Vec3(0,1,0)));
}

void ViewerWidget::rotate_model_z(float val){
	modelpat->setAttitude(modelpat->getAttitude()*osg::Quat(osg::DegreesToRadians(val),osg::Vec3(0,0,1)));
}

void ViewerWidget::translate_data_x(float val){
	osg::Vec3d prev=datapat->getPosition();
	prev.set(val,prev.y(),prev.z());
	datapat->setPosition(prev);
}

void ViewerWidget::translate_data_y(float val){
	osg::Vec3d prev=datapat->getPosition();
	prev.set(prev.x(),val,prev.z());
	datapat->setPosition(prev);
}

void ViewerWidget::translate_data_z(float val){
	osg::Vec3d prev=datapat->getPosition();
	prev.set(prev.x(),prev.y(),val);
	datapat->setPosition(prev);
}